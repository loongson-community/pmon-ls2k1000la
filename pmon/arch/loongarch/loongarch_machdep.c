/*	$Id: mips_machdep.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/syslog.h>
#include <sys/systm.h>
#include <sys/endian.h>
#include <sys/device.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include <machine/intr.h>
#include <dev/pci/pcivar.h>

#include <stdlib.h>

#include <autoconf.h>

#include <pmon.h>
extern char end[];
extern char edata[];
extern unsigned long long memorysize;

extern char MipsException[], MipsExceptionEnd[];

char hwethadr[6];

static int clkenable;
static unsigned int clkpertick;
static unsigned int clkperusec = 500;
static unsigned long _softcompare;

int copytoram __P((void *, void *));
void clearbss __P((void));


void clearbss(void)
{
	u_int count;
	u_int64_t *rdata;
	/*
	 *  Clear BSS.
	 */
	rdata = (u_int64_t *)CACHED_TO_UNCACHED(edata);
	rdata = (u_int64_t *)edata;
	while ((int)rdata & (sizeof(*rdata) - 1)) {
		*((char *)rdata) = 0;
		rdata = (char *)rdata + 1;
	}
	count = (end - edata) / sizeof(*rdata);
	while (count--) {
		*rdata++ = 0;
	}
}

int
copytoram(void *rom, void *ram)
{
	u_int count;
	u_int64_t *rdata, *pdata;

	/*
	 *  Copy ROM to RAM memory.
	 */
	pdata = (u_int64_t *)rom;
	rdata = (u_int64_t *)CACHED_TO_UNCACHED(ram);
	count = CACHED_TO_PHYS(end) - CACHED_TO_PHYS(ram);
	while (count > 0) {
		*rdata++ = *pdata++;
		count -= sizeof(*rdata);
	}

	/*
	 *  Verify copy ROM to RAM memory.
	 */
	pdata = (u_int64_t *)rom;
	rdata = (u_int64_t *)CACHED_TO_UNCACHED(ram);
	count = CACHED_TO_PHYS(end) - CACHED_TO_PHYS(ram);
	while (count > 0) {
		if (*rdata++ != *pdata++) {
			return ((int)rdata - sizeof(*rdata));	/* non zero */
		}
		count -= sizeof(*rdata);
	}
	clearbss();
	return (0);
}

/*
 *  Microtime returns the time that have elapsed since last 'tick'.
 *  To be able to do that we use the decrementer to figure out
 *  how much time have elapsed.
 */

void
microtime (tvp)
	struct timeval *tvp;
{
	static struct timeval lasttime;
	unsigned long count;
	long cycles;

	*tvp = time;

	/* work out how far we've progressed since the last "tick" */
	count = CPU_GetCOUNT64();
	cycles = count - (_softcompare - clkpertick);

	if (cycles >= 0)
		tvp->tv_usec += cycles / clkperusec;

	if (tvp->tv_usec >= 1000000) {
		tvp->tv_sec += tvp->tv_usec / 1000000;
		tvp->tv_usec %= 1000000;
	}

	if (tvp->tv_sec == lasttime.tv_sec &&
		tvp->tv_usec <= lasttime.tv_usec &&
		(tvp->tv_usec = lasttime.tv_usec + 1) > 1000000) {
		tvp->tv_sec++;
		tvp->tv_usec -= 1000000;
	}
	lasttime = *tvp;
}


void
startrtclock(hz)
	int hz;
{
	unsigned int freq;

	freq = tgt_pipefreq() / 2;           /* TB ticker frequency */

	/* get initial value of real time clock */
	time.tv_sec = tgt_gettime ();
	time.tv_usec = 0;
	if(time.tv_sec < 0) {                /* Bad clock ? */
	    time.tv_sec = 0;
	}

	clkpertick = freq / hz;
	clkperusec = freq / 1000000;
	_softcompare = CPU_GetCOUNT() + clkpertick;
	clkenable = 0;

	SBD_DISPLAY("RTCL",0);
}

void
enablertclock()
{
	clkenable = 1;
}
void
tgt_clkpoll ()
{
	struct clockframe cframe;
	unsigned long count;
	long cycles, ticks;

	if (!clkenable) {
		return;
	}
	cframe.pc = 0;
	cframe.sr = 0;
	cframe.cr = 0;

	/* poll the free-running clock */
	count = CPU_GetCOUNT64();
	cycles = count - _softcompare;

	if (cycles > 0) {

		/* as we are polling, we could have missed a number of ticks */
		ticks = (cycles / clkpertick) + 1;
		_softcompare += ticks * clkpertick;
		/* There is a race between reading count and setting compare
		 * whereby we could set compare to a value "below" the new
		 * clock value.  Check again to avoid an 80 sec. wait
		 */
		cycles = CPU_GetCOUNT64() - _softcompare;
		while (cycles > 0) {
		        _softcompare += clkpertick;
		        ticks++;
		        cycles = CPU_GetCOUNT64() - _softcompare;
		}
		while (ticks--) {
			hardclock(&cframe);
		}
	}
}

void
cpu_initclocks()
{
	printf("cpu_initclocks\n");
}

void
tgt_machreset()
{
	tgt_netreset();
}

void
delay(int microseconds)
{
	uint64_t total, start;

	start = CPU_GetCOUNT64();
	total = microseconds * clkperusec;
	while (total > (CPU_GetCOUNT64() - start));
}

extern void idle();
void delay1(int microseconds)
{
	int total, start;
	start = CPU_GetCOUNT();
	total = microseconds * clkperusec * 1000;

	while (total > (CPU_GetCOUNT() - start)) {
		idle();
	};
}

unsigned int get_timer(unsigned int base)
{
	unsigned int clkperms;
	unsigned long long now;

	clkperms = tgt_pipefreq() / 2 / 1000;
	now = CPU_GetCOUNT64();

	return (now) / clkperms - base;
}

unsigned int timer_get_us(void)
{
	unsigned int clkperms;
	unsigned long long now;

	clkperms = tgt_pipefreq() / 2 / 1000000;
	now = CPU_GetCOUNT64();

	return (now) / clkperms;
}


u_int __res_randomid(void);
u_int
__res_randomid()
{
	extern int ticks;
	return (ticks * CPU_GetCOUNT());    /* Just something... */
}

/*enable fpu regs*/
void tgt_fpuenable()
{
	asm(					\
	"csrrd $r4, 0x2;\n\t"			\
	"ori   $r4, $r4, 1;\n\t"		\
	"csrwr $r4, 0x2;\n\t"			\
	:::"$r4"
	);
}

/*disable fpu regs*/
void tgt_fpudisable()
{
	asm(					\
	"csrrd $r4, 0x2;\n\t"			\
	"ori   $r4, $r4, 1;\n\t"		\
	"xori  $r4, $r4, 1;\n\t"		\
	"csrwr $r4, 0x2;\n\t"			\
	:::"$r4"
	);
}

