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
#include <sys/linux/types.h>
#include <include/stdarg.h>
unsigned long long mem_size = 0;

#include "../../../pmon/common/smbios/smbios.h"
unsigned char *cpu_name = "LS2K";

void tgt_putchar(int);
int tgt_printf(const char *fmt, ...)
{
	int n;
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

#include <sys/param.h>
#include <sys/syslog.h>
#include <machine/endian.h>
#include <sys/device.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include <machine/intr.h>
#include <dev/pci/pcivar.h>
#include <sys/types.h>
#include <termio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <dev/ic/mc146818reg.h>
#include <linux/io.h>

#include <autoconf.h>

#include "pflash.h"
#include "dev/pflash_tgt.h"

#include "target/bonito.h"
#include "target/ls2k1000.h"
#include "target/board.h"
#include <pmon/dev/gt64240reg.h>
#include <pmon/dev/ns16550.h>

#include <pmon.h>

#include "mod_x86emu_int10.h"
#include "mod_x86emu.h"
#include "mod_vgacon.h"
#include "mod_framebuffer.h"
#include "mod_sisfb.h"
#include "sii9022a.h"
#include "dc_bridge.h"
#include "lt9211.h"
#if (NMOD_X86EMU_INT10 > 0)||(NMOD_X86EMU >0)
extern int vga_bios_init(void);
#endif
extern int radeon_init(void);
extern int kbd_initialize(void);
extern int write_at_cursor(char val);
extern const char *kbd_error_msgs[];
#include "flash.h"
#if (NMOD_FLASH_AMD + NMOD_FLASH_INTEL + NMOD_FLASH_SST) == 0
#ifdef HAVE_FLASH
#undef HAVE_FLASH
#endif
#else
#define HAVE_FLASH
#endif

#if (NMOD_X86EMU_INT10 == 0)&&(NMOD_X86EMU == 0)
int vga_available = 0;
#elif defined(VGAROM_IN_BIOS)
#include "target/vgarom.h"
#endif
#include "nand.h"
#include "spinand_lld.h"
#include "m25p80.h"

int tgt_i2cread(int type, unsigned char *addr, int addrlen, unsigned char reg,
		unsigned char *buf, int count);
int tgt_i2cwrite(int type, unsigned char *addr, int addrlen, unsigned char reg,
		 unsigned char *buf, int count);
extern struct trapframe DBGREG;
extern void *memset(void *, int, size_t);

int kbd_available;
int bios_available;
int usb_kbd_available;;
int vga_available;
int cmd_main_mutex = 0;
int bios_mutex = 0;
/* Board Version Number */
unsigned int board_ver_num;

static int md_pipefreq = 0;
static int md_cpufreq = 0;
static int clk_invalid = 0;
static void _probe_frequencies(void);

#ifndef NVRAM_IN_FLASH
void nvram_get(char *);
void nvram_put(char *);
#endif

extern int vgaterm(int op, struct DevEntry *dev, unsigned long param, int data);
extern int fbterm(int op, struct DevEntry *dev, unsigned long param, int data);
void error(unsigned long *adr, unsigned long good, unsigned long bad);
void modtst(int offset, int iter, unsigned long p1, unsigned long p2);
void do_tick(void);
void print_hdr(void);
void ad_err2(unsigned long *adr, unsigned long bad);
void ad_err1(unsigned long *adr1, unsigned long *adr2, unsigned long good,
	     unsigned long bad);
void mv_error(unsigned long *adr, unsigned long good, unsigned long bad);

void print_err(unsigned long *adr, unsigned long good, unsigned long bad,
	       unsigned long xor);
static void init_legacy_rtc(void);

ConfigEntry ConfigTable[] = {
	{(char *)COM1_BASE_ADDR, 0, ns16550, 256, CONS_BAUD, NS16550HZ},
#if NMOD_VGACON >0
#if NMOD_FRAMEBUFFER >0
	{(char *)1, 0, fbterm, 256, CONS_BAUD, NS16550HZ},
#else
	{(char *)1, 0, vgaterm, 256, CONS_BAUD, NS16550HZ},
#endif
#endif
	{0}
};

int afxIsReturnToPmon = 0;
unsigned char activecom = 0x3;
unsigned char em_enable = 0x3;
unsigned long _filebase;

unsigned char hwethadr[6];

void addr_tst1(void);
void addr_tst2(void);
void movinv1(int iter, ulong p1, ulong p2);

pcireg_t _pci_allocate_io(struct pci_device *dev, vm_size_t size);
extern char LoongArchException[], LoongArchExceptionEnd[];
static void superio_reinit();
extern char slave_main[];
#ifdef SLT
extern void slt_test();
#endif

void clear_pcie_inter_irq(void)
{
	unsigned int dev;
	unsigned int val;
	uint64_t addr;
	int i;
	for (i = 9; i < 15; i++) {
		dev = _pci_make_tag(0, i, 0);
		val = _pci_conf_read(dev, 0x00);
		if (val != 0xffffffff) {
			addr = PHYS_TO_UNCACHED(_pci_conf_read(dev, 0x10) & (~0xf));
			val = inl(addr + 0x18);
			if (val) {
				outl(addr + 0x1c, val);
			}
		}
	}
}

void mul_pin_def_cfg(void)
{
#if defined(CONFIG_REG1) && defined(CONFIG_REG2)
    readq(LS2K1000_GENERAL_CFG1) |= CONFIG_REG1;
    readq(LS2K1000_GENERAL_CFG2) |= CONFIG_REG2;
#else
	/* HDA CAN I2C1 */
	readq(LS2K1000_GENERAL_CFG0) &= ~(1 << 6);
	readq(LS2K1000_GENERAL_CFG0) |= (1 << 4) | (3 << 10);
#ifdef SEL_CAN
    readq(LS2K1000_GENERAL_CFG0) |= (3 << 16);
#endif

	/* for 2K1000_PC_EVB_V1.2 sdio's gpio */
	//readq(LS2K1000_GENERAL_CFG0) &= ~(0xf << 12);
#endif
	/* DMA nand 0 sdio 1*/
	readq(LS2K1000_APB_DMA_CFG) &= ~(0x7 | (0x7 << 15));
	readq(LS2K1000_APB_DMA_CFG) |= 1 << 15;

	/* Disable USB prefetch */
	readl(LS2K1000_GENERAL_CFG1) &= ~(1 << 19);

}

void mem_win_cfg(unsigned long long mem_size)
{
	/* CPU_WIN0 */
	readq(LS2K1000_XBAR_WIN0_BASE) = 0x1c000000;		/* base */
	readq(LS2K1000_XBAR_WIN0_MASK) = 0xfffffffffff00000;	/* mask */
	readq(LS2K1000_XBAR_WIN0_MMAP) = 0x1fc000d2;		/* mmap */

	/* CPU_WIN1 */
	readq(LS2K1000_XBAR_WIN1_BASE) = 0x10000000;
	readq(LS2K1000_XBAR_WIN1_MASK) = 0xfffffffff0000000;
	readq(LS2K1000_XBAR_WIN1_MMAP) = 0x10000082;

	/* CPU_WIN2 */
	readq(LS2K1000_XBAR_WIN2_BASE) = 0x0;
	readq(LS2K1000_XBAR_WIN2_MASK) = 0xfffffffff0000000;
	readq(LS2K1000_XBAR_WIN2_MMAP) = 0xf0;

	/* CPU_WIN3 */
	readq(LS2K1000_XBAR_WIN3_BASE) = 0x80000000;
	readq(LS2K1000_XBAR_WIN3_MASK) = 0xffffffff80000000;
	readq(LS2K1000_XBAR_WIN3_MMAP) = 0xf0;
    if(mem_size == 0x8){            //4GB 内存
	/* CPU_WIN4 */
	readq(LS2K1000_XBAR_WIN4_BASE) = 0x100000000;
	readq(LS2K1000_XBAR_WIN4_MASK) = 0xffffffff80000000;
	readq(LS2K1000_XBAR_WIN4_MMAP) = 0x800000f0;

	/* CPU_WIN5 */
	readq(LS2K1000_XBAR_WIN5_BASE) = 0x0;
	readq(LS2K1000_XBAR_WIN5_MASK) = 0x0;
	readq(LS2K1000_XBAR_WIN5_MMAP) = 0x0;
    }else if(mem_size == 0x10){     //8GB 内存
	/* CPU_WIN4 */
	readq(LS2K1000_XBAR_WIN4_BASE) = 0x100000000;
	readq(LS2K1000_XBAR_WIN4_MASK) = 0xffffffff00000000;
	readq(LS2K1000_XBAR_WIN4_MMAP) = 0x1000000f0;

	/* CPU_WIN5 */
	readq(LS2K1000_XBAR_WIN5_BASE) = 0x200000000;
	readq(LS2K1000_XBAR_WIN5_MASK) = 0xffffffff80000000;
	readq(LS2K1000_XBAR_WIN5_MMAP) = 0x800000f0;
    }else{                          //2GB内存
	/* CPU_WIN4 */
	readq(LS2K1000_XBAR_WIN4_BASE) = 0x0;
	readq(LS2K1000_XBAR_WIN4_MASK) = 0x0;
	readq(LS2K1000_XBAR_WIN4_MMAP) = 0x0;

	/* CPU_WIN5 */
	readq(LS2K1000_XBAR_WIN5_BASE) = 0x0;
	readq(LS2K1000_XBAR_WIN5_MASK) = 0x0;
	readq(LS2K1000_XBAR_WIN5_MMAP) = 0x0;
    }


	/* CPU_WIN6 */
	readq(LS2K1000_XBAR_WIN6_BASE) = 0x0;
	readq(LS2K1000_XBAR_WIN6_MASK) = 0x0;
	readq(LS2K1000_XBAR_WIN6_MMAP) = 0x0;

	/* CPU_WIN7 */
	readq(LS2K1000_XBAR_WIN7_BASE) = 0x0;
	readq(LS2K1000_XBAR_WIN7_MASK) = 0x0;
	readq(LS2K1000_XBAR_WIN7_MMAP) = 0x0;
}

void unlock_scache (unsigned long long unlock_base, unsigned int size)
{
	unsigned int i;
	unsigned long long reg_base = PHYS_TO_UNCACHED(0x1fe00200);

	/* unlock scache windows first */
	tgt_printf("unlock scache windows first\r\n");
	readq(reg_base + 0x40) = 0;
	readq(reg_base + 0x0) = 0;
	/* flush scache using hit-invalidate */
	tgt_printf("flush scache to unlock scache\r\n");
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

unsigned long long memorysize_high;
unsigned long long raw_memsz;
unsigned long long memorysize_total;

void get_memorysize(unsigned long long tgt_memsz)
{
	raw_memsz = tgt_memsz;
	tgt_printf("raw_memsz: 0x%llx\n", raw_memsz/2);
	mem_size = raw_memsz & 0xff;
	mem_size = mem_size << 29;
	mem_size = mem_size - 0x1000000;
	mem_size = mem_size >> 20;
	/*
	 * Set up memory address decoders to map entire memory.
	 * But first move away bootrom map to high memory.
	 */
	memorysize = mem_size > 240 ? 240 << 20 : mem_size << 20;
	memorysize_high = mem_size > 240 ? (((unsigned long long)mem_size) - 240) << 20 : 0;

	memorysize_total =  ((memorysize  +  memorysize_high)  >> 20) + 16;
	tgt_printf("memorysize_high: 0x%llx\n", memorysize_high);
}

void init_loongarch(unsigned long long tgt_memsz)
{
	unsigned int hi;
	unsigned short i;

	unlock_scache(LOCK_CACHE_BASE, LOCK_CACHE_SIZE);
	//core1 run wait_for_smp_call function in ram
	asm volatile("st.d %1,%0,0x20;"::"r"(0x800000001fe01100),"r"(&slave_main));

	mem_win_cfg(tgt_memsz);

#ifdef CONFIG_UART0_SPLIT
	readq(LS2K1000_GENERAL_CFG1) |= 0xe;
#endif
	/*enable float */
	tgt_fpuenable();

	get_memorysize(tgt_memsz);

	tgt_fpuenable();

	/*
	 *  Probe clock frequencys so delays will work properly.
	 */

    ls2k_i2c_init(0, LS2K1000_I2C0_REG_BASE);
    ls2k_i2c_init(0, LS2K1000_I2C1_REG_BASE);
	tgt_cpufreq();
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
	printf("BEV in SR set to zero.\n");

#if NNAND
#ifdef CONFIG_LS2K_NAND
	readq(LS2K1000_GENERAL_CFG0) |= (1 << 9);
	ls2k_nand_init();
#else
	/*nand pin as gpio*/
	readq(LS2K1000_GENERAL_CFG0) &= ~(1 << 9);
#endif
#if NSPINAND_MT29F || NSPINAND_LLD
	ls2k_spi_nand_probe();
#endif
#if NM25P80
	ls2k_m25p_probe();
#endif
#else
	/*nand pin as gpio*/
	readq(LS2K1000_GENERAL_CFG0) &= ~(1 << 9);
#endif

#ifdef DTB
	verify_dtb();
#endif
	clear_pcie_inter_irq();

	mul_pin_def_cfg();
	/*
	 * Launch!
	 */
	main();
}

#define STR_STORE_BASE	PHYS_TO_UNCACHED(0xfaaa000)
#define	PM_REG_BASE		PHYS_TO_UNCACHED(0x1fe27000)
/*
 *  Put all machine dependent initialization here. This call
 *  is done after console has been initialized so it's safe
 *  to output configuration and debug information with printf.
 */
extern void vt82c686_init(void);
int psaux_init(void);
extern int video_hw_init(void);

extern int fb_init(unsigned long, unsigned long);
extern unsigned long dc_init();

extern unsigned short ScreenLineLength;
extern unsigned short ScreenDepth;
extern unsigned short ScreenHeight;


static void init_pcidev(void)
{
	unsigned int val;
#if NMOD_VGACON > 0
	int rc = 1;
#if NMOD_FRAMEBUFFER > 0
	unsigned long fbaddress, ioaddress;
	extern struct pci_device *pcie_dev;
#endif
#endif
#ifdef UART0_ENABLE
	readq(LS2K1000_GENERAL_CFG1) |= 0xf;
#endif

	_pci_devinit(1);	/* PCI device initialization */

#ifdef PAI2
    pai2_init();
#endif

#if NMOD_FRAMEBUFFER > 0
#if (NMOD_X86EMU_INT10 > 0)||(NMOD_X86EMU >0)
	if(pcie_dev != NULL){
		SBD_DISPLAY("VGAI", 0);
		rc = vga_bios_init();
	}
#endif
	if (rc > 0) {
		if(pcie_dev == NULL){
			printf("begin dc_init\n");
			fbaddress = dc_init();
		} else {
			fbaddress  = _pci_conf_read(pcie_dev->pa.pa_tag, 0x10);
			fbaddress = fbaddress &0xffffff00; //laster 8 bit
			fbaddress |= UNCACHED_MEMORY_ADDR;
		}
		printf("fbaddress = %llx\n", fbaddress);

		fb_init(fbaddress, ioaddress);//ioaddress is unuseful
		printf("fb_init done\n");
	} else {
		printf("vga bios init failed, rc=%d\n",rc);
	}
#if NSII9022A
	config_sii9022a();
#endif
#if NLT9211
	LT9211_Config();
#endif
#if NDC_BRIDGE
	config_dc_bridge();
#endif
#endif

#if (NMOD_FRAMEBUFFER > 0)
	if (rc > 0)
		if (!getenv("novga"))
			vga_available = 1;
		else
			vga_available = 0;
#endif

	return;
}

void tgt_devconfig()
{
	/* Enable pci device and init VGA device */
	init_pcidev();

	config_init();
	configure();

#ifdef SLT
	slt_test();
#endif
	printf("devconfig done.\n");

}

uint64_t cmos_read64(unsigned long addr)
{
	unsigned char bytes[8];
	int i;

	for (i = 0; i < 8; i++)
		bytes[i] = *((unsigned char *)(STR_STORE_BASE + addr + i));
	return *(uint64_t *) bytes;
}

void cmos_write64(uint64_t data, unsigned long addr)
{
	int i;
	unsigned char *bytes = (unsigned char *) &data;

	for (i = 0; i < 8; i++)
		*((unsigned char *)(STR_STORE_BASE + addr + i)) = bytes[i];
}

void check_str()
{
	uint64_t s3_ra,s3_flag, s3_sp;
	unsigned int sp_h,sp_l;
	unsigned int gpe0_stat;

	s3_ra = cmos_read64(0x40);
	s3_sp = cmos_read64(0x48);
	s3_flag = cmos_read64(0x50);

	sp_h = s3_sp >> 32;
	sp_l = s3_sp;

	if ((s3_sp < CACHED_MEMORY_ADDR) || (s3_flag != 0x5a5a5a5a5a5a5a5a)) {
		*(uint64_t *)(STR_STORE_BASE + 0x50) = 0x0; //clean str flag
		printf("S3 status no exit %llx\n", s3_flag);
		return;
	}
	/* clean s3 wake flag */
	cmos_write64(0x0, 0x40);
	cmos_write64(0x0, 0x48);
	cmos_write64(0x0, 0x50);

	/*clean s3 wake status*/
	gpe0_stat = *(volatile unsigned int *)(PM_REG_BASE + 0x28);
	gpe0_stat |= 0xfff0;
	*(volatile unsigned int *)(PM_REG_BASE + 0x28) = gpe0_stat;

	/* Enable pci device and init VGA device */
	init_pcidev();

	/* fixup pcie config */
	ls_pcie_config_set();

	printf("jump to kernel....\n");
	//used sp
	__asm__ __volatile__(
			"or	$r3, %0, $r0		\n"
			"or	$r1, %1, $r0		\n"
			"jirl	$r0, $r1, 0		\n"
			: /* No outputs */
			:"r"(s3_sp), "r"(s3_ra)
			);
}

extern int test_icache_1(short *addr);
extern int test_icache_2(int addr);
extern int test_icache_3(int addr);
extern void godson1_cache_flush(void);
#define tgt_putchar_uc(x) (*(void (*)(char)) (((long)tgt_putchar)|0x20000000)) (x)

void tgt_devinit()
{
	/*
	 *  Gather info about and configure caches.
	 */
	CPU_ConfigCache();


	_pci_businit(1);        /* PCI bus initialization */
}

static int ls2k_rtc_wakeup_reboot(int delay);
void tgt_reboot()
{
#ifdef LS2K_RTC_WAKEUP_REBOOT
	init_legacy_rtc();
	ls2k_rtc_wakeup_reboot(LS2K_RTC_WAKEUP_REBOOT);
#elif LS2K_REBOOT_WDT
	readl(LS2K1000_WD_TIMER_REG) = 0x10;
	readl(LS2K1000_RST_CNT_REG) = 2;
	readl(LS2K1000_WD_SET_REG) = 1;
#else
	readl(LS2K1000_RST_CNT_REG) = 1;
#endif
}

void tgt_poweroff()
{
	readl(LS2K1000_PM1_STS_REG) &= 0xffffffff;
	readl(LS2K1000_PM1_CNT_REG) = 0x3c00;
}

#ifdef LS2K_RTC_WAKEUP_REBOOT
static time_t ls2k_rtc_gettime();
static int acpi_suspend()
{

	readl(LS2K1000_GPE0_EN_REG) = 0xfd70;
	readl(LS2K1000_GPE0_STS_REG) = 0x0000ffff;
	readl(LS2K1000_PM1_STS_REG) = 0x0000ffff;
	readl(LS2K1000_PM_RESUME_REG) |= 0x2880;
	readl(LS2K1000_PM1_EN_REG) |= 0x400;
	readl(LS2K1000_PM1_CNT_REG) = 0x00003400;
}

static int ls2k_rtc_alarmset(struct tm *tm)
{
	long rtc_reg = LS2K1000_RTC_REG_BASE;
	int c = readl(rtc_reg+0x40);
	if ((c&0x2900)!=0x2900) readl(rtc_reg+0x40) = 0x2900;

	readl(rtc_reg+0x34) = ((tm->tm_year&0x3f)<<26)|((tm->tm_mon + 1)<<22)|(tm->tm_mday<<17) \
			      |(tm->tm_hour<<12)|(tm->tm_min<<6)|(tm->tm_sec<<0);
	return 0;
}

static int ls2k_rtc_wakeup_reboot(int delay)
{
	struct tm *tm;
	time_t t = ls2k_rtc_gettime();
	t += delay;
	tm = gmtime(&t);
	ls2k_rtc_alarmset(tm);
	acpi_suspend();
	return 0;
}
#endif

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

static void init_legacy_rtc(void)
{
	int year, month, date, hour, min, sec, val;
	val = (1 << 13) | (1 << 11) | (1 << 8);

	outl(LS2K1000_RTC_CTRL_REG, val);
	outl(LS2K1000_TOY_TRIM_REG, 0);
	outl(LS2K1000_RTC_TRIM_REG, 0);

	val = inl(LS2K1000_TOY_READ0_REG);

	year = inl(LS2K1000_TOY_READ1_REG);
	month = (val >> 26) & 0x3f;
	date = (val >> 21) & 0x1f;
	hour = (val >> 16) & 0x1f;
	min = (val >> 10) & 0x3f;
	sec = (val >> 4) & 0x3f;
	if ((year < 0 || year > 138)
		|| (month < 1 || month > 12)
		|| (date < 1 || date > 31)
		|| (hour > 23) || (min > 59)
		|| (sec > 59)) {

		tgt_printf("RTC time invalid, reset to epoch.\n");
		/* 2000-01-01 00:00:00 */
		val = (1 << 26) | (1 << 21);
		outl(LS2K1000_TOY_WRITE1_REG, 0x64);
		outl(LS2K1000_TOY_WRITE0_REG, val);
	}
}

#ifdef EXTERNAL_RTC
extern int rtc_get_time(unsigned char *);
extern int rtc_set_time(unsigned char *);
extern int rtc_get_sec(void);
#endif

static void _probe_frequencies()
{
#ifdef HAVE_TOD
	int i, timeout, cur, sec, cnt;
#endif

	SBD_DISPLAY("FREQ", CHKPNT_FREQ);

	md_pipefreq = 660000000;
	md_cpufreq = 60000000;

	clk_invalid = 1;
#ifdef HAVE_TOD
#ifdef HPET_RTC
/* whd : USE HPET to calculate the frequency,
 *       reduce the booting delay and improve the frequency accuracy.
 *       when use the RTC counter of 7A, it cost 160us+ for one read,
 *       but if we use the HPET counter, it only cost ~300ns for one read,
 *       so the HPET has good accuracy even use less time */

	outl(LS2K1000_HPET0_CONF, 0x1);//Enable main clock

	/*
	 * Do the next twice to make sure we run from cache
	 */
	for (i = 2; i != 0; i--) {
		timeout = 10000000;

		sec = inl(LS2K1000_HPET0_MAIN);//get time now
		cnt = CPU_GetCOUNT();
		cur = (inl(LS2K1000_HPET0_PERIOD) / 1000000);
		sec = sec + (100000000 / cur);//go 100 ms
		do {
			timeout--;
			cur = (inl(LS2K1000_HPET0_MAIN));
		} while (timeout != 0 && (cur < sec));

		cnt = CPU_GetCOUNT() - cnt;
		if (timeout == 0) {
			tgt_printf("time out!\n");
			break;	/* Get out if clock is not running */
		}
	}

	/*
	 *  Calculate the external bus clock frequency.
	 */
	if (timeout != 0) {
		clk_invalid = 0;
		md_pipefreq = cnt / 1000;

		if((cnt % 1000) >= 500)//to make rounding
			md_pipefreq = md_pipefreq + 1;

		md_pipefreq *= 20000;
		/* we have no simple way to read multiplier value
		 */
		md_cpufreq = 66000000;
	}
		cur = (inl(LS2K1000_HPET0_PERIOD) / 1000000);
	tgt_printf("cpu freq %u, cnt %u\n", md_pipefreq, cnt);

	outl(LS2K1000_HPET0_CONF, 0x0);//Disable main clock
#else
#ifdef EXTERNAL_RTC
	for (i = 2; i != 0; i--) {
		timeout = 10000000;
		sec = rtc_get_sec();
		do {
			//wait 1 sec.
			cur = rtc_get_sec();
		} while (cur == sec);

		cnt = CPU_GetCOUNT();
		do {
			timeout--;
			sec = rtc_get_sec();
		} while (timeout != 0 && (cur == sec));
		cnt = CPU_GetCOUNT() - cnt;
		if (timeout == 0) {
			tgt_printf("time out!\n");
			break;	/* Get out if clock is not running */
		}
	}
#elif defined(INTERNAL_RTC)
	init_legacy_rtc();

	SBD_DISPLAY("FREI", CHKPNT_FREQ);

	/*
	 * Do the next twice for two reasons. First make sure we run from
	 * cache. Second make sure synched on second update. (Pun intended!)
	 */
	for (i = 2; i != 0; i--) {
		timeout = 10000000;
		sec = (inl(LS2K1000_TOY_READ0_REG) >> 4) & 0x3f;
		do {
			cur = ((inl(LS2K1000_TOY_READ0_REG) >> 4) & 0x3f);
		} while (cur == sec);

		cnt = CPU_GetCOUNT();
		do {
			timeout--;
			sec = (inl(LS2K1000_TOY_READ0_REG) >> 4) & 0x3f;
		} while (timeout != 0 && (cur == sec));
		cnt = CPU_GetCOUNT() - cnt;
		if (timeout == 0) {
			tgt_printf("time out!\n");
			break;	/* Get out if clock is not running */
		}
	}
#endif

	/*
	 *  Calculate the external bus clock frequency.
	 */
	if (timeout != 0) {
		clk_invalid = 0;
		md_pipefreq = cnt / 10000;
		md_pipefreq *= 20000;
		/* we have no simple way to read multiplier value
		 */
		md_cpufreq = 66000000;
	}
#endif
	tgt_printf("cpu freq %u\n", md_pipefreq);
#endif /* HAVE_TOD */
}

/*
 *   Returns the CPU pipelie clock frequency
 */
uint64_t tgt_pipefreq()
{
	if (md_pipefreq == 0)
		_probe_frequencies();

	return (md_pipefreq);
}

/*
 *   Returns the external clock frequency, usually the bus clock
 */
int tgt_cpufreq()
{
	if (md_cpufreq == 0)
		_probe_frequencies();

	return (md_cpufreq);
}

/*
ls2k RTC_TOY_READ1/0x1fe07830 read result will or RST_CNT/0x1fe07030 read result.
ls7a RTC_TOY_READ1/0x100d0130 read result will or RST_CNT/0x100d0030 read result.
so we need write RST_CNT/0x1fe07030 to 0 before read RTC_TOY_READ1/0x1fe07830.
we feed dog if wdt_en, because another core may feed dog when we set RST_CNT/0x1fe07030 to 0.
*/
#define RST_WDTEN 2
#define RTS_CNT_OFFSET 0x800
static time_t ls2k_rtc_gettime()
{
	struct tm tm;
	time_t t;
	unsigned int val;
	unsigned int  rst_ctrl = inl(LS2K1000_TOY_READ1_REG - RTS_CNT_OFFSET);
	if (rst_ctrl & RST_WDTEN)
		outl(LS2K1000_TOY_READ1_REG - RTS_CNT_OFFSET, 0);
	tm.tm_year = inl(LS2K1000_TOY_READ1_REG);
	if (rst_ctrl & RST_WDTEN) {
		outl(LS2K1000_TOY_READ1_REG - RTS_CNT_OFFSET, rst_ctrl);
		outl(LS2K1000_TOY_READ1_REG - RTS_CNT_OFFSET + 4, 1);
	}

	val = inl(LS2K1000_TOY_READ0_REG);
	tm.tm_sec = (val >> 4) & 0x3f;
	tm.tm_min = (val >> 10) & 0x3f;
	tm.tm_hour = (val >> 16) & 0x1f;
	tm.tm_mday = (val >> 21) & 0x1f;
	tm.tm_mon = ((val >> 26) & 0x3f) - 1;
	tm.tm_isdst = tm.tm_gmtoff = 0;
	t = gmmktime(&tm);
	return (t);
}


time_t tgt_gettime()
{
	struct tm tm;
	time_t t;

#ifdef HAVE_TOD
#ifdef EXTERNAL_RTC
	unsigned char buf[7] = {0};

	t = rtc_get_time(buf);

	if (t) {
		tm.tm_sec = buf[0];
		tm.tm_min = buf[1];
		tm.tm_hour = buf[2];
		tm.tm_mday = buf[4];
		tm.tm_mon = buf[5]-1;
		tm.tm_year = buf[6];
		if (tm.tm_year < 50)
			tm.tm_year += 100;
		tm.tm_isdst = tm.tm_gmtoff = 0;
		t = gmmktime(&tm);
	} else
#elif defined(INTERNAL_RTC)
	unsigned int val;

	if (!clk_invalid) {
		t = ls2k_rtc_gettime();
	} else
#endif
#endif
	{
		t = 957960000;	/* Wed May 10 14:00:00 2000 :-) */
	}
	return (t);
}

/*
 *  Set the current time if a TOD clock is present
 */
void tgt_settime(time_t t)
{
	struct tm *tm;

#ifdef HAVE_TOD
#ifdef EXTERNAL_RTC
	unsigned char buf[7] = {0};
	tm = gmtime(&t);
	buf[0] = tm->tm_sec;
	buf[1] = tm->tm_min;
	buf[2] = tm->tm_hour;
	buf[4] = tm->tm_mday;
	buf[5] = (tm->tm_mon + 1);
	 if(tm->tm_year > 100)
		 tm->tm_year -=100;
	buf[6] = tm->tm_year;

	rtc_set_time(buf);
#elif defined(INTERNAL_RTC)
	unsigned int val;

	if (!clk_invalid) {
		tm = gmtime(&t);
		val = ((tm->tm_mon + 1) << 26) | (tm->tm_mday << 21) |
			(tm->tm_hour << 16) | (tm->tm_min << 10) |
			(tm->tm_sec << 4);
		outl(LS2K1000_TOY_WRITE0_REG, val);
		outl(LS2K1000_TOY_WRITE1_REG, tm->tm_year);
	}
#endif
#endif
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
	printf("Copyright 2005, ICT CAS.\n");
	printf("CPU %s @", cpu_name);
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

int tgt_ethaddr(char *p)
{
	bcopy((void *)&hwethadr, p, 6);
	return (0);
}

void tgt_netreset()
{
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

#ifdef   BOOT_FROM_NAND

int cmp_test_mem(char *dist, char *src, int size)
{
	int i, j, count;
	j = 0;
	count = 0;
	for (i = 0; i < size; i++) {

		if (*dist != *src) {
			count++;
			if (j <= 10) {
				printf("dist != num.addr is :0x%x\n\n", src);

			}
			j++;
		}
		dist++;
		src++;
	}
	if (count == 0) {
		printf("dist == num.\n");
	} else {

		printf("dist != num. count:%d\n", count);
	}
	return 0;
}

#define NAND_TEMP_ADDR ((volatile unsigned char *)0x86200000)

int update_env_to_nand(char *nvram_addr, char *update_buff, int size)
{
	int block_num;
	char *block_addr;
	int block_offset;
	char *nand_update_ramaddr;
	char *test_addr;

	block_addr =
	    (char *)(((unsigned int)NVRAM_OFFS + 0x400) & (~(0x20000 - 1)));
	// block_addr = ((unsigned int )(nvram_addr) &(~(0x20000 - 1)));
	nandread(NAND_TEMP_ADDR, block_addr, 0x20000);	//read 1 block to ram

	// block_offset = nvram_addr - block_addr;
	block_offset = NVRAM_OFFS + 0x400 - (unsigned int)block_addr;
	nand_update_ramaddr = NAND_TEMP_ADDR + block_offset;

	memcpy(nand_update_ramaddr, update_buff, size);	//update env in ram

	block_num = (unsigned int)block_addr / (2048 * 64);
	block_erase(block_num, 1);

	nandwrite(NAND_TEMP_ADDR, block_addr, 0x20000);	//write ram to nand for 1 block

	return 0;
}

#define REMAP_DDR_DMA 0x00e00000
#define ROM_OFFSET    0x400
#define ROM_OFFSET_ECC    0xeb0
#define NANDBLOCK_SIZE 0x20000
#define NANDBLOCK_SIZE_ECC (188*10*64)
#define NAND_PAGE_SIZE 2048
#define NAND_PAGE_SIZE_ECC 1880
char *romaddr_to_ramaddr(char *romaddr)
{
	char *ramaddr;

	ramaddr = (((unsigned int)romaddr) & (~0xfff00000));
	ramaddr = ((unsigned int)ramaddr) | REMAP_DDR_DMA;
	return ramaddr;
}

#ifndef NAND_ECC_MODE

int update_rom_to_nand_1block(char *nvram_addr, char *nvram_start_addr)	//normal mode
{
	int block_num;
	char *nand_block_addr, *rom_block_addr;
	rom_block_addr =
	    (char
	     *)((((unsigned int)nvram_addr +
		  ROM_OFFSET) & (~(NANDBLOCK_SIZE - 1))) - ROM_OFFSET);
	rom_block_addr = romaddr_to_ramaddr(rom_block_addr);
	nand_block_addr =
	    (nvram_addr - nvram_start_addr +
	     ROM_OFFSET) & (~(NANDBLOCK_SIZE - 1));
	block_num = (unsigned int)nand_block_addr / NANDBLOCK_SIZE;

	nandwrite_set_badblock(rom_block_addr, nand_block_addr, NANDBLOCK_SIZE);	//write ram to nand for 1 block

	return 0;
}
#else

int update_rom_to_nand_1block(char *nvram_addr, char *nvram_start_addr)	//ecc mode
{
	int block_num, nand_count;
	char *nand_block_addr, *rom_block_addr;
	rom_block_addr =
	    ((unsigned int)nvram_addr) - ((unsigned int)nvram_start_addr) +
	    ROM_OFFSET_ECC;
	rom_block_addr =
	    ((unsigned int)rom_block_addr) -
	    (((unsigned int)rom_block_addr) % NANDBLOCK_SIZE_ECC) -
	    ROM_OFFSET_ECC;
	rom_block_addr = romaddr_to_ramaddr(rom_block_addr);
	nand_count = nvram_addr - nvram_start_addr + ROM_OFFSET_ECC;
	nand_block_addr =
	    (nand_count / NAND_PAGE_SIZE_ECC) * NAND_PAGE_SIZE +
	    ((nand_count % NAND_PAGE_SIZE_ECC) / 188) * 204;
	nand_block_addr =
	    ((unsigned int)nand_block_addr) & (~(NANDBLOCK_SIZE - 1));

	nandwrite_set_badblock_ecc(rom_block_addr, nand_block_addr, NANDBLOCK_SIZE_ECC);	//write ram to nand for 1 block

	return 0;
}

#endif

int check_bad_block_pt(void)
{
	int i;
	for (i = 0; i < 16; i++) {
		NAND_TEMP_ADDR[i] = 0;
	}
	nandwrite_spare(NAND_TEMP_ADDR, 333 * 2048 * 64, 16);
	nandwrite_spare(NAND_TEMP_ADDR, 444 * 2048 * 64, 16);
	nandwrite_spare(NAND_TEMP_ADDR, 555 * 2048 * 64, 16);
	for (i = 0; i < 1024; i++) {
		nandread_spare(NAND_TEMP_ADDR, i * 2048 * 64, 16);
		if (NAND_TEMP_ADDR[0] != 0xff) {
			printf("bad block addr: 0x%x,num: %d\n", i * 2048 * 64,
			       i);
		}
	}
}

void tgt_nand_test_print(void)
{
	int i, j, k;
	int block_num;
	//nandread_check_badblock_ecc(0x86000000,0 ,188*10*64*4 );//write ram to nand for 1 block

	//cmp_test_mem(0x85000000  ,0x86000000 + 1880*2 ,188*10*64*4  - 1880*2 );
}
#endif


#ifndef NVRAM_IN_FLASH

/*
 *  Read and write data into non volatile memory in clock chip.
 */
void nvram_get(char *buffer)
{
	int i;
	for (i = 0; i < 114; i++) {
		linux_outb(i + RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
		buffer[i] = linux_inb(RTC_DATA_REG);
	}
}

void nvram_put(char *buffer)
{
	int i;
	for (i = 0; i < 114; i++) {
		linux_outb(i + RTC_NVRAM_BASE, RTC_INDEX_REG);	/* Address */
		linux_outb(buffer[i], RTC_DATA_REG);
	}
}

#endif

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

#else
extern void longjmp(label_t *, int);
void gsignal(label_t * jb, int sig);
void gsignal(label_t * jb, int sig)
{
	if (jb != NULL) {
		longjmp(jb, 1);
	}
};

int netopen(const char *, int);
int netread(int, void *, int);
int netwrite(int, const void *, int);
long netlseek(int, long, int);
int netioctl(int, int, void *);
int netclose(int);
int netopen(const char *p, int i)
{
	return -1;
}

int netread(int i, void *p, int j)
{
	return -1;
}

int netwrite(int i, const void *p, int j)
{
	return -1;
}

int netclose(int i)
{
	return -1;
}

long int netlseek(int i, long j, int k)
{
	return -1;
}

int netioctl(int j, int i, void *p)
{
	return -1;
}

void tgt_netpoll()
{
};

#endif /*INET*/

char *tran_month(char *c, char *i)
 {
     switch (*c++){
     case  'J':
         if(*c++ == 'a')     /* Jan */
             i = "01";
         else if(*c++ == 'n')    /* June */
             i = "06";
         else                /* July */
             i = "07";
         break;
     case  'F':              /* Feb */
         i = "02";
         break;
     case  'M':
         c++;
         if(*c++ == 'r')     /* Mar */
             i = "03";
         else                /* May */
             i = "05";
         break;
     case  'A':
         if(*c++ == 'p')     /* Apr */
             i = "04";
         else                /* Aug */
             i = "08";
         break;
         case  'S':              /* Sept */
         i = "09";
         break;
     case  'O':              /* Oct */
         i = "10";
         break;
     case  'N':              /* Nov */
         i = "11";
         break;
     case  'D':              /* Dec */
         i = "12";
         break;
     default :
         i = NULL;
     }

     return i;
 }


int get_update(char *p)
 {
     int i=0;
     char *t,*mp,m[3];

     t  = strstr(vers, ":");
     strncpy(p, t+26, 4);     /* year */
     p[4] = '-';
     mp = tran_month(t+6, m);    /* month */
     strncpy(p+5,mp,2);
     p[7]='-';
     strncpy(p+8, t+10, 2);   /* day */
     p[10] = '\0';

     return 0;
 }

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct pci_config_data pci_config_array[] = {
			/*		APB		*/
[0] = {
.bus = 0, .dev = 0x2, .func = 0, .interrupt = 0, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x1fe00000, .mem_end = 0x1fe0ffff, .type = PCI_DEV,
},
			/*		GMAC0	*/
[1] = {
.bus = 0, .dev = 0x3, .func = 0, .interrupt = 20, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40040000, .mem_end = 0x4004ffff, .type = PCI_DEV,
},
			/*		GMAC1	*/
[2] = {
.bus = 0, .dev = 0x3, .func = 1, .interrupt = 22, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40050000, .mem_end = 0x4005ffff, .type = PCI_DEV,
},
			/*		OTG		*/
[3] = {
.bus = 0, .dev = 0x4, .func = 0, .interrupt = 57, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40000000, .mem_end = 0x4003ffff, .type = PCI_DEV,
},
			/*		EHCI	*/
[4] = {
.bus = 0, .dev = 0x4, .func = 1, .interrupt = 58, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40060000, .mem_end = 0x4006ffff, .type = PCI_DEV,
},
			/*		OHCI	*/
[5] = {
.bus = 0, .dev = 0x4, .func = 2, .interrupt = 59, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40070000, .mem_end = 0x4007ffff, .type = PCI_DEV,
},
			/*		GPU		*/
[6] = {
.bus = 0, .dev = 0x5, .func = 0, .interrupt = 37, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x40080000, .mem_end = 0x400bffff, .type = PCI_DEV,
},
			/*		DC		*/
[7] = {
.bus = 0, .dev = 0x6, .func = 0, .interrupt = 36, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x400c0000, .mem_end = 0x400cffff, .type = PCI_DEV,
},
			/*		HDA		*/
[8] = {
.bus = 0, .dev = 0x7, .func = 0, .interrupt = 12, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x400d0000, .mem_end = 0x400dffff, .type = PCI_DEV,
},
			/*		SATA	*/
[9] = {
.bus = 0, .dev = 0x8, .func = 0, .interrupt = 27, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x400e0000, .mem_end = 0x400effff, .type = PCI_DEV,
},
#if 0		/*	PCIE0-PORT0	*/
[10] = {
.bus = 0, .dev = 0x9, .func = 0, .interrupt = 40, .primary = 0, .secondary = 1,
.subordinate = 1, .mem_start = 0x40100000, .mem_end = 0x4fffffff, .type = PCI_BRIDGE,
.io_start = 0x18000000, .io_end = 0x180fffff,
},
			/*	PCIE0-PORT1	*/
[11] = {
.bus = 0, .dev = 0xa, .func = 0, .interrupt = 41, .primary = 0, .secondary = 4,
.subordinate = 4, .mem_start = 0x50000000, .mem_end = 0x53ffffff, .type = PCI_BRIDGE,
.io_start = 0x18100000, .io_end = 0x181fffff,
},
			/*	PCIE0-PORT2	*/
[12] = {
.bus = 0, .dev = 0xb, .func = 0, .interrupt = 42, .primary = 0, .secondary = 8,
.subordinate = 8, .mem_start = 0x54000000, .mem_end = 0x57ffffff, .type = PCI_BRIDGE,
.io_start = 0x18200000, .io_end = 0x182fffff,
},
			/*	PCIE0-PORT3	*/
[13] = {
.bus = 0, .dev = 0xc, .func = 0, .interrupt = 43, .primary = 0, .secondary = 0xc,
.subordinate = 0xc, .mem_start = 0x58000000, .mem_end = 0x5fffffff, .type = PCI_BRIDGE,
.io_start = 0x18300000, .io_end = 0x183fffff,
},
			/*	PCIE1-PORT0	*/
[14] = {
.bus = 0, .dev = 0xd, .func = 0, .interrupt = 44, .primary = 0, .secondary = 0x10,
.subordinate = 0x10, .mem_start = 0x60000000, .mem_end = 0x77ffffff, .type = PCI_BRIDGE,
.io_start = 0x18400000, .io_end = 0x184fffff,
},
			/*	PCIE1-PORT1	*/
[15] = {
.bus = 0, .dev = 0xe, .func = 0, .interrupt = 45, .primary = 0, .secondary = 0x14,
.subordinate = 0x14, .mem_start = 0x78000000, .mem_end = 0x7fffffff, .type = PCI_BRIDGE,
.io_start = 0x18500000, .io_end = 0x185fffff,
},
#endif
			/*		VPU	*/
[16] = {
.bus = 0, .dev = 0x10, .func = 0, .interrupt = 30, .primary = 0, .secondary = 0,
.subordinate = 0, .mem_start = 0x79000000, .mem_end = 0x7900ffff, .type = PCI_DEV,
},
};

int pci_config_array_size = ARRAY_SIZE(pci_config_array);
#define  __raw__readq(addr)			(*(volatile unsigned long long *)(addr))
#define	 __raw__writeq(addr, val)	(*(volatile unsigned long long *)(addr) = (val))
void ls_pcie_config_set(void)
{
	int i;
	for(i = 0;i < ARRAY_SIZE(pci_config_array);i++){
			ls_pcie_interrupt_fixup(pci_config_array + i);
			ls_pcie_payload_fixup(pci_config_array + i);
	}
	ls_pci_msi_window_config();

	map_gpu_addr();
}

extern unsigned long long memorysize_total;
void map_gpu_addr(void)
{
    flushcache();

	if (memorysize_total == 0x800) {
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe00038), 0x20000000);
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe00078), 0xffffffffe0000000);
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe000b8), 0x00000000600000f0);
	} else if (memorysize_total == 0x1000) {
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe00038), 0x20000000);
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe00078), 0xffffffffe0000000);
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe000b8), 0x00000000e00000f0);
	} else if (memorysize_total == 0x2000) {
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe00038), 0x20000000);
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe00078), 0xffffffffe0000000);
		__raw__writeq(PHYS_TO_UNCACHED(0x1fe000b8), 0x00000001e00000f0);
	} else {
		tgt_printf ("Now this Memory size %lld MB is not support mapping GPU address.\n", memorysize_total);
	}
}

void ls_pcie_interrupt_fixup(struct pci_config_data *pdata)
{
	unsigned int dev;
	unsigned int val;

	dev = _pci_make_tag(pdata->bus, pdata->dev, pdata->func);
	val = _pci_conf_read32(dev, 0x00);
	/*	device on the slot	*/
	if ( val != 0xffffffff)
			_pci_conf_write16(dev, 0x3c, pdata->interrupt|0x100);

}

void ls_pcie_payload_fixup(struct pci_config_data *pdata)
{
	unsigned int dev;
	unsigned int val;
	u16 max_payload_spt, control;

	dev = _pci_make_tag(pdata->bus, pdata->dev, pdata->func);
	val = _pci_conf_read32(dev, 0x00);
	/*	device on the slot	*/
	if ( val != 0xffffffff){
			if(pdata->type == PCI_BRIDGE){
					/*set Max_Payload_Size & Max_Read_Request_Size*/
					max_payload_spt = 1;
					control = _pci_conf_read16(dev, 0x78);
					control &= (~PCI_EXP_DEVCTL_PAYLOAD & ~PCI_EXP_DEVCTL_READRQ);
					control |= ((max_payload_spt << 5) | (max_payload_spt << 12));
					_pci_conf_write16(dev, 0x78, control);
			}
	}
}

void ls_pci_msi_window_config(void)
{
	/*config msi window*/
	__raw__writeq(PHYS_TO_UNCACHED(0x1fe02500), 0x000000001fe10000ULL);
	__raw__writeq(PHYS_TO_UNCACHED(0x1fe02540), 0xffffffffffff0000ULL);
	__raw__writeq(PHYS_TO_UNCACHED(0x1fe02580), 0x000000001fe10081ULL);
}
