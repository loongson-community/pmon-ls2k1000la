#include <sys/types.h>
#include <sys/malloc.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include <pmon.h>
#include <stdlib.h>
#include "target/bonito.h"
#include "pflash.h"
#include "flash.h"
#include "dev/pflash_tgt.h"

#if (NMOD_FLASH_AMD + NMOD_FLASH_INTEL + NMOD_FLASH_SST) == 0
#ifdef HAVE_FLASH
#undef HAVE_FLASH
#endif
#else
#define HAVE_FLASH
#endif

extern unsigned long long  memorysize;
extern unsigned long long  memorysize_high_n[];

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

void tgt_flashprogram(void *p, int size, void *s, int endian)
{
	unsigned short val;
	unsigned int ret;
	/*close other core*/
	val = inw(PHYS_TO_UNCACHED(0x1fe001d0));
	outw(PHYS_TO_UNCACHED(0x1fe001d0), val & (0x7777 ^ (1 << (BOOTCORE_ID << 2) + 3)));
	ret = inl(PHYS_TO_UNCACHED(0x1fe00420));
	if (ret & 0x100)
		outl(PHYS_TO_UNCACHED(0x1fe00420), (ret & ~0x100));

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
	/*open other core*/
	//if (ret & 0x100)  // ???? open store spd cause reboot
	//	outl(PHYS_TO_UNCACHED(0x1fe00420), ret);
	//outw( PHYS_TO_UNCACHED(0x1fe001d0), val);
}
#endif /* PFLASH */

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
	uint64_t freq;

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

#ifdef no_thank_you
	(*func)("vxWorks", env);
#endif


	sprintf(env, "%d", memorysize / (1024 * 1024));
	(*func)("memsize", env);

	sprintf(env, "%d", (memorysize_high_n[0] - 0x10000000) / (1024 * 1024));
	(*func)("highmemsize", env);
#if	(TOT_NODE_NUM >= 2)
	for (i = 1; i < TOT_NODE_NUM; i++) {
		memset(env, 0, 50);
		sprintf(env, "%d", memorysize_high_n[i] / (1024 * 1024));
		sprintf(&env[20], "memorysize_high_n%d", i);
		(*func)(&env[20], env);
	}
#endif
	freq = tgt_pipefreq(); 
	sprintf(env, "%llu", freq);
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
	{
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
