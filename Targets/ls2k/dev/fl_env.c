#include <sys/types.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include <pmon.h>
#include <stdlib.h>
#include "target/bonito.h"
#include "pflash.h"
#include "flash.h"
#include "dev/pflash_tgt.h"
#include "include/ls2k1000.h"

#if (NMOD_FLASH_AMD + NMOD_FLASH_INTEL + NMOD_FLASH_SST) == 0
#ifdef HAVE_FLASH
#undef HAVE_FLASH
#endif
#else
#define HAVE_FLASH
#endif

extern unsigned long long  memorysize;
extern unsigned long long  memorysize_high;
extern unsigned char hwethadr[6];

static int nvram_invalid = 0;

static int cksum(void *p, size_t s, int set);
#ifdef HAVE_FLASH
/*
 *  Flash programming support code.
 */

/*
 *  Table of flash devices on target. See pflash_tgt.h.
 */

struct fl_map tgt_fl_map_boot16[]={
	TARGET_FLASH_DEVICES_16
};


struct fl_map *tgt_flashmap()
{
	return tgt_fl_map_boot16;
}

void tgt_flashwrite_disable()
{
}

int tgt_flashwrite_enable()
{
	return(1);
}

void tgt_flashinfo(void *p, size_t *t)
{
	struct fl_map *map;

	map = fl_find_map(p);
	if (map) {
		*t = map->fl_map_size;
	} else {
		*t = 0;
	}
}

void _core1_idle()
{
	while(1);
}

void tgt_flashprogram(void *p, int size, void *s, int endian)
{
	if ((unsigned long long)p == BONITO_FLASH_BASE) {
		unsigned long long idle_addr = &_core1_idle;
		readq(NODE0_CORE1_BUF0 + FN_OFF) = idle_addr;
	}

	printf("Programming flash %llx:%x into %llx\n", s, size, p);
	if (fl_erase_device(p, size, TRUE)) {
		printf("Erase failed!\n");
		return;
	}
	printf("Erase end!\n");
	if (fl_program_device(p, s, size, TRUE)) {
		printf("Programming failed!\n");
	}
	printf("Programming end!\n");
	fl_verify_device(p, s, size, TRUE);
}
#endif /* PFLASH */

/*
 *  Try to figure out what kind of flash there is on given address.
 */
struct fl_device *fl_devident(void *base, struct fl_map **m)
{
//	if (selected_lpc_spi()) {
	if (1) {
		return spi_fl_devident(base,m);
	} else {
		struct fl_device *dev;
		struct fl_map *map;
		char mfgid, chipid;
		int retry = 0;

		/* If we can't write to flash, we can't identify */
		if (!tgt_flashwrite_enable()) {
			return((struct fl_device *)NULL);
		}

		map = fl_find_map(base);

		if (map != NULL) {
			outb(LS2K1000_GENERAL_CFG2, 0x20);
			outl(LS2K1000_LPC_CFG0_REG, 0xffffff);
			outl(LS2K1000_LPC_CFG1_REG, 0);
again:
			fl_autoselect(map);

			switch(map->fl_map_bus) {
				case FL_BUS_8:

#ifndef LS3B_SPI_BOOT
					mfgid = inb(map->fl_map_base);
					chipid = inb(map->fl_map_base + 1);
#else
					mfgid = loongson_spi_readid();
					chipid = loongson_spi_readchipid();
#endif
					if (chipid == mfgid) { /* intel 16 bit flash mem */
						chipid = inb(map->fl_map_base+3);
					}
					break;

				case FL_BUS_8_ON_64:
					mfgid = inb(map->fl_map_base);
					chipid = inb(map->fl_map_base + 8);
					break;

				case FL_BUS_16:
					mfgid = inw(map->fl_map_base);
					chipid = inw(map->fl_map_base + 2);
					break;

				case FL_BUS_32:
					mfgid = inl(map->fl_map_base);
					chipid = inl(map->fl_map_base + 4);
					break;

				case FL_BUS_64:
					mfgid = inw(map->fl_map_base);
					chipid = inw(map->fl_map_base + 8);
					break;
			}
			fl_reset(map);

			if (retry == 0 && mfgid == 0 && chipid == 0) {
				outb(LS2K1000_GENERAL_CFG2, 0x60);
				outl(LS2K1000_LPC_CFG0_REG, 0x80ffff);
				outl(LS2K1000_LPC_CFG1_REG, 0x80000000);
				retry = 1;
				goto again;
			}
			/* Lookup device type using manufacturer and device id */
			for(dev = &fl_known_dev[0]; dev->fl_name != 0; dev++) {
				if (dev->fl_mfg == mfgid && dev->fl_id == chipid) {
					tgt_flashwrite_disable();
					if (m) {
						*m = map;
					}
					return(dev);	/* GOT IT! */
				}
			}
			printf("Mfg %2x, Id %2x\n", mfgid, chipid);
		}

		tgt_flashwrite_disable();
		outb((map->fl_map_base), 0xf0);
		outb((map->fl_map_base), 0x90);
		outb((map->fl_map_base), 0x00);

		return((struct fl_device *)NULL);
	}
}

/*************************************************************************/
/*
 *	Target dependent Non volatile memory support code
 *	=================================================
 *
 *
 *  On this target a part of the boot flash memory is used to store
 *  environment. See EV64260.h for mapping details. (offset and size).
 */

/*
 *  Read in environment from NV-ram and set.
 */
void tgt_mapenv(int (*func) __P((char *, char *)))
{
	char *ep;
	char env[512];
	char *nvram;
	int i;

	/*
	 *  Check integrity of the NVRAM env area. If not in order
	 *  initialize it to empty.
	 */
	printf("in envinit\n");
	nvram = (char *)(tgt_flashmap())->fl_map_base;
	printf("nvram=%08x\n",(unsigned int)nvram);
	if (fl_devident(nvram, NULL) == 0 || cksum(nvram + NVRAM_OFFS, NVRAM_SIZE, 0) != 0) {
		printf("NVRAM is invalid!\n");
		nvram_invalid = 1;
	} else {
		nvram += NVRAM_OFFS;
		ep = nvram+2;;

		while (*ep != 0) {
			char *val = 0, *p = env;
			i = 0;
			while ((*p++ = *ep++) && (ep <= nvram + NVRAM_SIZE - 1) && i++ < 255) {
				if ((*(p - 1) == '=') && (val == NULL)) {
					*(p - 1) = '\0';
					val = p;
				}
			}
			if (ep <= nvram + NVRAM_SIZE - 1 && i < 255) {
				(*func)(env, val);
			} else {
				nvram_invalid = 2;
				break;
			}
		}
	}

	printf("NVRAM@%x\n", (u_int32_t)nvram);

	bcopy(&nvram[ETHER_OFFS], hwethadr, 6);
	sprintf(env, "%02x:%02x:%02x:%02x:%02x:%02x", hwethadr[0], hwethadr[1],
		hwethadr[2], hwethadr[3], hwethadr[4], hwethadr[5]);
	(*func) ("ethaddr", env);

#ifdef LOWPOWER
	sprintf(env, "0x%x", shutdev);
	(*func) ("shutdev", env);
#endif

#ifdef no_thank_you
	(*func)("vxWorks", env);
#endif


	sprintf(env, "%d", memorysize / (1024 * 1024));
	(*func)("memsize", env);

	sprintf(env, "%d", memorysize_high / (1024 * 1024));
	(*func)("highmemsize", env);

	uint32_t freq;
	freq = tgt_pipefreq();
	sprintf(env, "%d", freq);
	(*func)("cpuclock", env);

	sprintf(env, "%d",VRAM_SIZE);
	(*func)("vramsize", env);

	sprintf(env, "%d",0);

	(*func)("sharevram", env);

	(*func)("systype", SYSTYPE);
}

int tgt_unsetenv(char *name)
{
	char *ep, *np, *sp;
	char *nvram;
	char *nvrambuf;
	char *nvramsecbuf;
	int status;

	if (nvram_invalid)
		return(0);

	/* Use first defined flash device (we probably have only one) */
	nvram = (char *)(tgt_flashmap())->fl_map_base;

	/* Map. Deal with an entire sector even if we only use part of it */
	nvram += NVRAM_OFFS & ~(NVRAM_SECSIZE - 1);
	nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
	if (nvramsecbuf == 0) {
		printf("Warning! Unable to malloc nvrambuffer!\n");
		return(-1);
	}
	memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
	nvrambuf = nvramsecbuf + (NVRAM_OFFS & (NVRAM_SECSIZE - 1));

	ep = nvrambuf + 2;

	status = 0;
	while ((*ep != '\0') && (ep <= nvrambuf + NVRAM_SIZE)) {
		np = name;
		sp = ep;

		while ((*ep == *np) && (*ep != '=') && (*np != '\0')) {
			ep++;
			np++;
		}
		if ((*np == '\0') && ((*ep == '\0') || (*ep == '='))) {
			while (*ep++);
			while (ep <= nvrambuf + NVRAM_SIZE) {
				*sp++ = *ep++;
			}
			if (nvrambuf[2] == '\0') {
				nvrambuf[3] = '\0';
			}
			cksum(nvrambuf, NVRAM_SIZE, 1);
			if (fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
				status = -1;
				break;
			}

			if (fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
				status = -1;
				break;
			}
			status = 1;
			break;
		}
		else if (*ep != '\0') {
			while (*ep++ != '\0');
		}
	}

	free(nvramsecbuf);
	return(status);
}

int tgt_setenv(char *name, char *value)
{
	char *ep;
	int envlen;
	char *nvrambuf;
	char *nvramsecbuf;
	char *nvram;

	/* Non permanent vars. */
	if (strcmp(EXPERT, name) == 0) {
		return(1);
	}

	/* Calculate total env mem size requiered */
	envlen = strlen(name);
	if (envlen == 0) {
		return(0);
	}
	if (value != NULL) {
		envlen += strlen(value);
	}
	envlen += 2;	/* '=' + null byte */
	if (envlen > 255) {
		return(0);	/* Are you crazy!? */
	}

	/* Use first defined flash device (we probably have only one) */
	nvram = (char *)(tgt_flashmap())->fl_map_base;

	/* Deal with an entire sector even if we only use part of it */
	nvram += NVRAM_OFFS & ~(NVRAM_SECSIZE - 1);

	/* If NVRAM is found to be uninitialized, reinit it. */
	if (nvram_invalid) {
		nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
		if (nvramsecbuf == 0) {
			printf("Warning! Unable to malloc nvrambuffer!\n");
			return(-1);
		}
		memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
		nvrambuf = nvramsecbuf + (NVRAM_OFFS & (NVRAM_SECSIZE - 1));
		memset(nvrambuf, -1, NVRAM_SIZE);
		nvrambuf[2] = '\0';
		nvrambuf[3] = '\0';
		cksum((void *)nvrambuf, NVRAM_SIZE, 1);
		printf("Warning! NVRAM checksum fail. Reset!\n");
		if (fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
			printf("Error! Nvram erase failed!\n");
			free(nvramsecbuf);
			return(-1);
		}
		if (fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
			printf("Error! Nvram init failed!\n");
			free(nvramsecbuf);
			return(-1);
		}
		nvram_invalid = 0;
		free(nvramsecbuf);
	}

	/* Remove any current setting */
	tgt_unsetenv(name);

	/* Find end of evironment strings */
	nvramsecbuf = (char *)malloc(NVRAM_SECSIZE);
	if (nvramsecbuf == 0) {
		printf("Warning! Unable to malloc nvrambuffer!\n");
		return(-1);
	}
	memcpy(nvramsecbuf, nvram, NVRAM_SECSIZE);
	nvrambuf = nvramsecbuf + (NVRAM_OFFS & (NVRAM_SECSIZE - 1));
	if (strcmp("ethaddr", name) == 0) {
		char *s = value;
		int i;
		int32_t v;
		for (i = 0; i < 6; i++) {
			gethex(&v, s, 2);
			hwethadr[i] = v;
			s += 3;	/* Don't get to fancy here :-) */
		}
	} else {
		ep = nvrambuf+2;
		if (*ep != '\0') {
			do {
				while (*ep++ != '\0');
			} while (*ep++ != '\0');
			ep--;
		}
		if ((ep + NVRAM_SIZE - ep) < (envlen + 1)) {
			free(nvramsecbuf);
			return(0);	/* Bummer! */
		}

		/*
		 *  Special case heaptop must always be first since it
		 *  can change how memory allocation works.
		 */
		if (strcmp("heaptop", name) == 0) {

			bcopy(nvrambuf+2, nvrambuf+2 + envlen,
					ep - nvrambuf+1);

			ep = nvrambuf+2;
			while (*name != '\0') {
				*ep++ = *name++;
			}
			if (value != NULL) {
				*ep++ = '=';
				while ((*ep++ = *value++) != '\0');
			}
			else {
				*ep++ = '\0';
			}
		} else {
			while (*name != '\0') {
				*ep++ = *name++;
			}
			if (value != NULL) {
				*ep++ = '=';
				while ((*ep++ = *value++) != '\0');
			} else {
				*ep++ = '\0';
			}
			*ep++ = '\0';   /* End of env strings */
		}
	}
	cksum(nvrambuf, NVRAM_SIZE, 1);

	bcopy(hwethadr, &nvramsecbuf[ETHER_OFFS], 6);
	if (fl_erase_device(nvram, NVRAM_SECSIZE, FALSE)) {
		printf("Error! Nvram erase failed!\n");
		free(nvramsecbuf);
		return(0);
	}
	if (fl_program_device(nvram, nvramsecbuf, NVRAM_SECSIZE, FALSE)) {
		printf("Error! Nvram program failed!\n");
		free(nvramsecbuf);
		return(0);
	}
	free(nvramsecbuf);
	return(1);
}

/*
 *  Calculate checksum. If 'set' checksum is calculated and set.
 */
static int cksum(void *p, size_t s, int set)
{
	u_int16_t sum = 0;
	u_int8_t *sp = p;
	int sz = s / 2;

	if (set) {
		*sp = 0;	/* Clear checksum */
		*(sp+1) = 0;	/* Clear checksum */
	}
	while (sz--) {
		sum += (*sp++) << 8;
		sum += *sp++;
	}
	if (set) {
		sum = -sum;
		*(u_int8_t *)p = sum >> 8;
		*((u_int8_t *)p+1) = sum;
	}
	return(sum);
}
