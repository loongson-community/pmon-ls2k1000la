/*	$Id: tgt_machdep.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <include/stdarg.h>
#include <include/stdio.h>
#include <include/file.h>
#include <linux/io.h>
#include <sys/ioccom.h>
#include <sys/types.h>
#include <frame.h>
#include <termio.h>
#include <string.h>
#include <stdlib.h>
#include <dev/pci/pcivar.h>

#include <autoconf.h>
#include <pmon.h>
#include <machine/cpu.h>
#include <machine/pio.h>

#include <pmon/dev/ns16550.h>
#include "loongson3_def.h"

#include "target/ls7a.h"

#include "mod_x86emu_int10.h"
#include "mod_x86emu.h"
#include "mod_vgacon.h"
#include "mod_framebuffer.h"
#include "mod_sisfb.h"

#ifdef	ACPI_SUPPORT
#include "../acpi/acpi_tables/dsdt/dsdt.c"
#endif

/* Generic file-descriptor ioctl's. */
#define	FIOCLEX		 _IO('f', 1)		/* set close on exec on fd */
#define	FIONCLEX	 _IO('f', 2)		/* remove close on exec */
#define	FIONREAD	_IOR('f', 127, int)	/* get # bytes to read */
#define	FIONBIO		_IOW('f', 126, int)	/* set/clear non-blocking i/o */
#define	FIOASYNC	_IOW('f', 125, int)	/* set/clear async i/o */
#define	FIOSETOWN	_IOW('f', 124, int)	/* set owner */
#define	FIOGETOWN	_IOR('f', 123, int)	/* get owner */

#define STDIN		((kbd_available|usb_kbd_available) ? 3 : 0)

extern char * nvram_offs;

void tgt_putchar (int);
int tgt_printf (const char *fmt, ...)
{
	int  n;
	char buf[1024];
	char *p = buf;
	char c;
	va_list ap;
	va_start(ap, fmt);
	n = vsprintf(buf, fmt, ap);
	va_end(ap);
	while ((c = *p++)) {
		if (c == '\n')
			tgt_putchar('\r');
		tgt_putchar(c);
	}
	return (n);
}

extern int kbd_initialize(void);
#if NMOD_VGACON
extern const char *kbd_error_msgs[];
#endif

#if (NMOD_X86EMU_INT10 == 0) && (NMOD_X86EMU == 0)
int vga_available = 0;
#elif defined(VGAROM_IN_BIOS)
#include "target/vgarom.h"
#endif

extern struct trapframe DBGREG;
extern void gmac_mac_init();
int kbd_available;
int bios_available;
int usb_kbd_available;
int vga_available;
int cmd_main_mutex = 0;
int bios_mutex = 0;

static uint64_t md_pipefreq = 0;
static void ls7a_pwm(int x, int y);
extern int w83795_config(void);

extern int vgaterm(int op, struct DevEntry * dev, unsigned long param, int data);
extern int fbterm(int op, struct DevEntry * dev, unsigned long param, int data);

extern void init_ls_rtc(unsigned long long base);
extern void ls7a_pcie_irq_fixup(void);
extern int ls7a_version(void);
extern void hda_codec_set(void);


#ifdef INTERFACE_3A780E
#define REV_ROW_LINE 560
#define INF_ROW_LINE 576

int afxIsReturnToPmon = 0;

struct FackTermDev
{
	int dev;
};

#endif

ConfigEntry	ConfigTable[] =
{
	{ (char *)GS3_UART_BASE, 0, ns16550, 256, CONS_BAUD, NS16550HZ },
#if NMOD_VGACON >0
#if NMOD_FRAMEBUFFER >0
	{ (char *)1, 0, fbterm, 256, CONS_BAUD, NS16550HZ },
#else
	{ (char *)1, 0, vgaterm, 256, CONS_BAUD, NS16550HZ },
#endif
#endif
	{ 0 }
};

unsigned long _filebase;

extern unsigned long long memorysize_high_n[];
extern unsigned long long memorysize;
extern unsigned long long memorysize_total;

extern char LoongArchException[], LoongArchExceptionEnd[];

extern void superio_reinit();

void __attribute__((weak)) realinit_loongarch(void)
{

}

void unlock_scache (unsigned long long unlock_base, unsigned int size)
{
	unsigned int i;
	unsigned long long reg_base = PHYS_TO_UNCACHED(0x1fe00000);

	/* unlock scache windows first */
	tgt_printf("unlock scache windows first\r\n");
	readq(reg_base + 0x208) = 0;
	readq(reg_base + 0x248) = 0;
	/* flush scache using hit-invalidate */
	tgt_printf("flush scache to unlock scache for 3a\r\n");
	tgt_printf("unlock_base %11p\r\n", unlock_base);
	for (i = 0; i < size; i += 0x40) {
		asm(
			"cacop 0x13, %0, 0\n\t"
			:
			: "r"(unlock_base)
		);
		unlock_base += 0x40;
	}
	
}

#include "target/mem_ctrl.h"
void get_memorysize(void)
{
	int i, j;
	unsigned long long memsz;
	struct ddr4_smbios *smbios_info_t = SMBIOS_INFO_IN_SDRAM_OFFS;

	memorysize = 240 << 20; //256M-16M
	for (i = 0; i < TOT_NODE_NUM; i++) {
		for (j = 0, memsz = 0; j < 4; j++) {
			//GB unit
			memsz += smbios_info_t[i * 4 + j].memsize;
		}
		memsz = memsz >> 10 << 30;
		//now use Byte unit to transfer memsize
		memorysize_high_n[i] = memsz;
		memorysize_total += memsz;
		tgt_printf("memorysize_high_n[%d]: 0x%llx\n", i, memorysize_high_n[i]);
	}
	memorysize_total >>= 20;
}

void update_cpu_name(void)
{
	unsigned long long *ptr;
	unsigned long long node;

	ptr = (void *)md_cpuname();
	for (node = 0; node < TOT_NODE_NUM; node++)
		readq(PHYS_TO_UNCACHED(0x1fe00020) | (node << NODE_OFFSET)) = *ptr;
}

void i2c_slave(char num)
{
	uint64_t i;
	uint8_t  slave_addr;
	for(i = 1; i < num; i += 4) {
		switch(i) {
		case 1:
			slave_addr = 0x1b;
			break;
		case 5:
			slave_addr = 0x2b;
			break;
		case 9:
			slave_addr = 0x1c;
			break;
		case 13:
			slave_addr = 0x2c;
			break;
		}
		readb(LS3A5000_I2C0_REG_BASE | (i << NODE_OFFSET) + 0x02) &= ~(0x1 << 5);
		readb(LS3A5000_I2C0_REG_BASE | (i << NODE_OFFSET) + 0x07) = ((0x1 << 7) | slave_addr);
	}
}

void init_loongarch()
{
	unlock_scache(LOCK_CACHE_BASE, LOCK_CACHE_SIZE);

	get_memorysize();
#if defined(LS_STR)
	/*if S3, jump to kernel*/
	check_str();
#endif
	update_cpu_name();

	tgt_fpuenable();

	ls7a_pwm(0, 10000);
	SBD_DISPLAY("DONE", 0);

	bcopy(LoongArchException, (char *)GEN_EXC_VEC, LoongArchExceptionEnd - LoongArchException);
	cpu_set_ebase();
	cpu_set_tlb_ebase();
	tgt_printf("set ebase done\n");
	/*
	 *  Init PMON and debug
	 */
	cpuinfotab[0] = &DBGREG;
	dbginit(NULL);

	/*
	 *  Set up exception vectors.
	 */
	SBD_DISPLAY("BEV1", 0);

//	tgt_printf("fush cache start\n");
//	CPU_FlushCache();
//	tgt_printf("fush cache done\n");


	printf("BEV in SR set to zero.\n");
#ifdef DTB
	verify_dtb();
#endif
	set_freq_scale();

	ls132_init();
	main();
}

/*
 *  Put all machine dependent initialization here. This call
 *  is done after console has been initialized so it's safe
 *  to output configuration and debug information with printf.
 */

extern int fb_init(unsigned long,unsigned long);
extern int dc_init(void);

extern unsigned short ScreenLineLength;
extern unsigned short ScreenDepth;
extern unsigned short ScreenHeight;

void set_freq_scale(void)
{
#if	(TOT_NODE_NUM == 1)
	readl(PHYS_TO_UNCACHED(0x1fe00008)) |= (1 << 6); //LOONGSON_CPU_FREQ_SCALE
#ifdef LS132_CORE
	readl(PHYS_TO_UNCACHED(0x1fe00008)) |= (1 << 7); //LOONGSON_CPU_DVFS_V1
	//run ls132 core
	printf("LOONGSON 132 CORE\n");
	readl(PHYS_TO_UNCACHED(0x1fe00420)) |= 0x100;
#endif
#endif
}

void tgt_devconfig()
{
	int ic, len;
	char bootup[] = "Booting...";
	char * s;
	int rc = 1;
#if NMOD_VGACON > 0
#if NMOD_FRAMEBUFFER > 0 
	unsigned long fbaddress, ioaddress;
	extern struct pci_device *pcie_dev;
#endif
#endif
	_pci_devinit(1);	/* PCI device initialization */

#if (NMOD_X86EMU_INT10 > 0)||(NMOD_X86EMU >0)
	if (pcie_dev != NULL) {
		SBD_DISPLAY("VGAI", 0);
		rc = vga_bios_init();
		printf("rc=%d\n", rc);
	}
#endif
#if defined(VESAFB)
	SBD_DISPLAY("VESA", 0);
	if (rc > 0)
		vesafb_init();
#endif
#if NMOD_FRAMEBUFFER > 0
	vga_available = 0;
	if (rc > 0) {
		if (pcie_dev == NULL) {
			printf("begin dc_init\n");
			fbaddress = dc_init();
			//this parameters for 800*600 VGA
			ScreenLineLength = LS_FB_XSIZE * 2;
			ScreenDepth = 16;
			ScreenHeight = LS_FB_YSIZE;
		} else {
			fbaddress  = _pci_conf_read(pcie_dev->pa.pa_tag, 0x10);
			fbaddress = fbaddress &0xffffff00; //laster 8 bit
			fbaddress |= UNCACHED_MEMORY_ADDR;
		}
		printf("fbaddress = %llx\n", fbaddress);

		fb_init(fbaddress, ioaddress);//ioaddress is unuseful
		printf("fb_init done\n");
	} else {
		printf("vga bios init failed, rc=%d\n", rc);
	}
#endif

#if (NMOD_FRAMEBUFFER > 0) || (NMOD_VGACON > 0 )
	if (rc > 0)
		if (!getenv("novga"))
			vga_available = 1;
		else
			vga_available = 0;
	printf("vga available : %d\n", vga_available);
#endif
	config_init();
	configure();
	gmac_mac_init();

	if(getenv("nokbd"))
		rc = 1;
	else {
		superio_reinit();
		rc=kbd_initialize();
	}
#if NMOD_VGACON
	printf("%s\n", kbd_error_msgs[rc]);
#endif
	if (!rc) {
		kbd_available = 1;
	}

	printf("devconfig done.\n");
	ls7a_pcie_irq_fixup();
	hda_codec_set();
#if	(TOT_NODE_NUM >= 4) && !defined(TY_MULTI_BOARD)
	w83795_config();
	printf("w83795 init done.\n");
#endif
	/*i2c slave mode for BMC*/
#if	(LS3A_NODE_NUM >= 8)
		i2c_slave(LS3A_NODE_NUM);
#endif
}


void tgt_devinit()
{
	/*
	 *  Gather info about and configure caches.
	 */
	CPU_ConfigCache();

	_pci_businit(1);	/* PCI bus initialization */
}

void tgt_poweroff()
{
	readl(LS7A_ACPI_PM1_STS_REG) &= 0xffffffff;
	readl(LS7A_ACPI_PM1_CNT_REG) = 0x3c00;
}

void tgt_reboot(void)
{
	readl(LS7A_ACPI_RST_CNT_REG) = 1;
}

/*
 *  This function makes inital HW setup for debugger and
 *  returns the apropriate setting for the status register.
 */
register_t tgt_enable(int machtype)
{
	/* XXX Do any HW specific setup */
	return 0;
}

/*
 *  Target dependent version printout.
 *  Printout available target version information.
 */
void tgt_cmd_vers()
{
}

static void ls7a_pwm(int x, int y)
{
	readl(LS7A_PWM0_CTRL) &= ~1;
	outl(LS7A_PWM0_LOW, x);
	outl(LS7A_PWM0_FULL, y);
	readl(LS7A_PWM0_CTRL) |= 1;

	readl(LS7A_PWM1_CTRL) &= ~1;
	outl(LS7A_PWM1_LOW, x);
	outl(LS7A_PWM1_FULL, y);
	readl(LS7A_PWM1_CTRL) |= 1;

	//as now, the 7A Fan control circuit(PWM2) has problem, keep it constant to avoid wearing fan.
	//readl(LS7A_PWM2_CTRL) &= ~1;
	//outl(LS7A_PWM2_LOW,x);
	//outl(LS7A_PWM2_FULL,y);
	//readl(LS7A_PWM2_CTRL) |= 1;

	readl(LS7A_PWM3_CTRL) &= ~1;
	outl(LS7A_PWM3_LOW, x);
	outl(LS7A_PWM3_FULL, y);
	readl(LS7A_PWM3_CTRL) |= 1;
}

static void _probe_frequencies()
{
#ifdef HAVE_TOD
	int i, timeout, cur, sec;
	unsigned long long cnt;
#endif

	SBD_DISPLAY ("FREQ", CHKPNT_FREQ);

	md_pipefreq = 6600000;

#ifdef HAVE_TOD
	init_ls_rtc(LS7A_RTC_REG_BASE);

	SBD_DISPLAY ("FREI", CHKPNT_FREQ);

#ifdef USE_RTC_COUNTER
	/*
	 * Do the next twice for two reasons. First make sure we run from
	 * cache. Second make sure synched on second update. (Pun intended!)
	 */
	for (i = 2; i > 0; i--) {
		timeout = 0x10000000;
		sec = get_rtc_sec();
		do {
			cur = get_rtc_sec();
		} while (cur == sec);

		cnt = CPU_GetCOUNT64();
		do {
			timeout--;
			sec = get_rtc_sec;
		} while (timeout != 0 && (cur == sec));
		cnt = CPU_GetCOUNT64() - cnt;
		if (timeout == 0) {
			tgt_printf("time out! You should better check rtc!\n");
			break;	/* Get out if clock is not running */
		}
	}

	/*
	 *  Calculate the external bus clock frequency.
	 */
	if (timeout != 0) {
		md_pipefreq = cnt / 10000;
		md_pipefreq *= 20000;
	}
	tgt_printf("cpu freq %u\n", md_pipefreq);

#else
/*
 //whd: use to check the read delay
		cnt = CPU_GetCOUNT();
	for(i = 0; i != 100; i++) {
		inl(LS7A_TOY_READ0_REG);
	}
		cnt = CPU_GetCOUNT() - cnt;

	tgt_printf("100 read RTC delay %u\n", cnt);

		cnt = CPU_GetCOUNT();
	for(i = 0; i != 100; i++) {
		inl(0xBA000000);
	}
		cnt = CPU_GetCOUNT() - cnt;

	tgt_printf("100 read HT header delay %u\n", cnt);

		cnt = CPU_GetCOUNT();
	for(i = 0; i != 100; i++) {
		inl(0xBA001000);
	}
		cnt = CPU_GetCOUNT() - cnt;

	tgt_printf("100 read RTC header delay %u\n", cnt);

		cnt = CPU_GetCOUNT();
	for(i = 0; i != 100; i++) {
		inl(0xB00A0004);
	}
		cnt = CPU_GetCOUNT() - cnt;

	tgt_printf("100 read PWM header delay %u\n", cnt);

		cnt = CPU_GetCOUNT();
	for(i = 0; i != 100; i++) {
		inl(0xB00010f0);
	}
		cnt = CPU_GetCOUNT() - cnt;

	tgt_printf("100 read HPET header delay %u\n", cnt);
*/

/* whd : USE HPET to calculate the frequency,
 *       reduce the booting delay and improve the frequency accuracy.
 *       when use the RTC counter of 7A, it cost 160us+ for one read,
 *       but if we use the HPET counter, it only cost ~300ns for one read,
 *       so the HPET has good accuracy even use less time */

	outl(LS7A_HPET_CONF, 0x1);//Enable main clock

	/*
	 * Do the next twice to make sure we run from cache
	 */
	for (i = 2; i != 0; i--) {
		timeout = 10000000;

		sec = inl(LS7A_HPET_MAIN);//get time now
		cnt = CPU_GetCOUNT64();
		cur = (inl(LS7A_HPET_PERIOD) / 1000000);
		sec = sec + (100000000 / cur);	//go 100 ms
		do {
			timeout--;
			cur = (inl(LS7A_HPET_MAIN));
		} while (timeout != 0 && (cur < sec));

		cnt = CPU_GetCOUNT64() - cnt;
		if (timeout == 0) {
			tgt_printf("time out!\n");
			break;	/* Get out if clock is not running */
		}
	}

	/*
	 *  Calculate the external bus clock frequency.
	 */
	if (timeout != 0) {
		md_pipefreq = cnt / 1000;

		if ((cnt % 1000) >= 500)//to make rounding
			md_pipefreq = md_pipefreq + 1;

		md_pipefreq *= 20000;
	}
	cur = (inl(LS7A_HPET_PERIOD) / 1000000);
	tgt_printf("cpu freq %u, cnt %u\n", md_pipefreq, cnt);

	outl(LS7A_HPET_CONF, 0x0);//Disable main clock
#endif

#endif /* HAVE_TOD */
}
/*
 *   Returns the CPU pipelie clock frequency
 */
uint64_t tgt_pipefreq()
{
	if (md_pipefreq == 0) {
		_probe_frequencies();
	}
	return(md_pipefreq);
}

/*
 *  Print out any target specific memory information
 */
void tgt_memprint()
{
	printf("L1 Instruction cache size %dKB (%d Line, %d Way)\n",
	                cpu_icache_size / 1024, cpu_icache_sets, cpu_icache_way);
	printf("L1 Data cache size %dKB (%d Line, %d Way)\n",
	                cpu_dcache_size / 1024, cpu_dcache_sets, cpu_dcache_way);
	if (cpu_l2_cache_size != 0) {
	        printf("L2 cache size %dKB\n", cpu_l2_cache_size / 1024);
	}
	if (cpu_l3_cache_size != 0) {
               printf("L3 cache size %dKB\n", cpu_l3_cache_size / 1024);
	}
}

void tgt_machprint()
{
	printf("Copyright 2000-2002, Opsycon AB, Sweden.\n");
	printf("Copyright Loongson Technology.\n");
	printf("CPU %s @", md_cpuname());
}

/*
 *  Return a suitable address for the client stack.
 *  Usually top of RAM memory.
 */

register_t tgt_clienttos()
{
	extern char start[];
	return(register_t)PHYS_TO_CACHED(start - 64);
}

/*
 *  Network stuff.
 */
void tgt_netinit()
{
}

void tgt_netreset()
{
}

/*
 *  Simple display function to display a 4 char string or code.
 *  Called during startup to display progress on any feasible
 *  display before any serial port have been initialized.
 */
void tgt_display(char *msg, int x)
{
	/* Have simple serial port driver */
	tgt_putchar(msg[0]);
	tgt_putchar(msg[1]);
	tgt_putchar(msg[2]);
	tgt_putchar(msg[3]);
	tgt_putchar('\r');
	tgt_putchar('\n');
}

void clrhndlrs()
{
}

int tgt_getmachtype()
{
	return (md_cputype());
}

/*
 *  Create stubs if network is not compiled in
 */
#ifdef INET
void tgt_netpoll()
{
	splx(splhigh());
}
#endif /*INET*/


#define HW_CONFIG PHYS_TO_UNCACHED(0x1fe00180)
#define HW_SAMPLE PHYS_TO_UNCACHED(0x1fe00190)
#define HT_MEM_PLL PHYS_TO_UNCACHED(0x1fe001c0)

uint32_t mem_is_hw_bypassed()
{
	uint32_t hw_sample = readl(HW_SAMPLE + 0x4);
	return ((hw_sample >> 8) & 0x3) == 0x3;
}

uint32_t mem_is_hw_low_freq()
{
	uint32_t hw_sample = readl(HW_SAMPLE + 0x4);
	return ((hw_sample >> 8) & 0x3) == 0x0;
}

uint32_t mem_is_hw_high_freq()
{
	uint32_t hw_sample = readl(HW_SAMPLE + 0x4);
	return ((hw_sample >> 8) & 0x3) == 0x1;
}

uint32_t mem_is_sw_setup()
{
	uint32_t hw_sample = readl(HW_SAMPLE + 0x4);
	return ((hw_sample >> 8) & 0x3) == 0x2;

}

uint32_t mem_is_25_ref()
{
	uint32_t hw_sample = readl(HW_SAMPLE + 0x4);
	return ((hw_sample >> 10) & 0x1) == 0x1;

}

uint32_t mem_sw_freq_loopc()
{
	uint32_t sw_loopc = (readl(HT_MEM_PLL) >> 14) & 0x3ff;

	return sw_loopc;
}

uint32_t mem_sw_freq_div()
{
	uint32_t sw_div = (readl(HT_MEM_PLL) >> 24) & 0x3f;

	return sw_div;
}

uint32_t mem_sw_freq_div_mode()
{
        uint32_t sw_mode = (readl(HT_MEM_PLL) >> 4) & 0x3;

        return sw_mode;
}

uint32_t mem_sw_freq_refc()
{
	uint32_t sw_div = (readl(HT_MEM_PLL) >> 8) & 0x1f;

	return sw_div;
}

void print_mem_freq(void) 
{
	uint32_t mem_ref_clock, mem_freq, memdiv_mode;

#ifdef BONITO_100M
	mem_ref_clock = 100; /* int MHz */
#elif BONITO_25M
	mem_ref_clock = 25;
#endif
	if (mem_is_hw_bypassed()) { 
		printf("hw bypassed! mem @ %dMHz\n", mem_ref_clock);
		return;
	}
	if (mem_is_hw_low_freq())
		printf("hw configured mem @ 466MHz, DDR-1866\n");
	else if (mem_is_hw_high_freq()) 
		printf("hw configured mem @ 600MHz, DDR-2400\n");
	else {
		memdiv_mode = mem_sw_freq_div_mode();
		memdiv_mode = (memdiv_mode == 1 ? 2 : (memdiv_mode == 2 ? 1 : 4));
		mem_freq = (mem_ref_clock/mem_sw_freq_refc()*mem_sw_freq_loopc())/mem_sw_freq_div();
		printf("sw configured DDR-%dMHz\n", mem_freq * memdiv_mode);
	}
}

void  print_cpu_info()
{
	int cycles1, cycles2;
	int bogo;
	int loops = 1 << 18;
	int counts;
	uint64_t freq;
	char fs[10], *fp;

	printf("Copyright 2000-2002, Opsycon AB, Sweden.\n");
	printf("Copyright 2005-2020, Loongson Technology.\n");
	printf("CPU %s @", md_cpuname());

	freq = tgt_pipefreq ();
	sprintf(fs, "%u", freq);
	fp = fs + strlen(fs) - 6;
	fp[3] = '\0';
	fp[2] = fp[1];
	fp[1] = fp[0];
	fp[0] = '.';
	printf (" %s MHz, ", fs);

	freq = freq / 1000000;

	counts = loops;
	cycles1 = read_c0_count();
 	__asm__ __volatile__(
	"	.align 3			\n"
	"1:					\n"
	"	addi.d	%0, %0, -1		\n"
	"	bnez	%0, 1b			\n"
	: "=r"(counts)
	: "0" (counts));
	cycles2 = read_c0_count();

	bogo = freq * loops / (cycles2 - cycles1);

	printf("BogoMIPS: %d\n", bogo);
}

