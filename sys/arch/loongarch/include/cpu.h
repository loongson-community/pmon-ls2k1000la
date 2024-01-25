/*	$OpenBSD: cpu.h,v 1.4 1998/09/15 10:50:12 pefo Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	Copyright (C) 1989 Digital Equipment Corporation.
 *	Permission to use, copy, modify, and distribute this software and
 *	its documentation for any purpose and without fee is hereby granted,
 *	provided that the above copyright notice appears in all copies.
 *	Digital Equipment Corporation makes no representations about the
 *	suitability of this software for any purpose.  It is provided "as is"
 *	without express or implied warranty.
 *
 *	from: @(#)cpu.h	8.4 (Berkeley) 1/4/94
 */

#ifndef _LOONGARCH_CPU_H_
#define _LOONGARCH_CPU_H_

#include <machine/psl.h>

#define KUSEG_ADDR		0x0
#define CACHED_MEMORY_ADDR	0x9000000000000000
#define UNCACHED_MEMORY_ADDR	0x8000000000000000
#define MAX_MEM_ADDR		PHYS_TO_UNCACHED(0x1e000000)
#define	RESERVED_ADDR		PHYS_TO_UNCACHED(0x1fc80000)
#define IS_CACHED_ADDR(x)	(!!(((x) & 0xff00000000000000ULL) == CACHED_MEMORY_ADDR))

#define CACHED_TO_PHYS(x)	VA_TO_PHYS(x)
#define UNCACHED_TO_PHYS(x)	VA_TO_PHYS(x)
#define PHYSADDR(x)		VA_TO_PHYS(x)
#ifndef _LOCORE
#define	PHYS_TO_CACHED(x)	((unsigned long long)(x) | CACHED_MEMORY_ADDR)
#define	PHYS_TO_UNCACHED(x) 	((unsigned long long)(x) | UNCACHED_MEMORY_ADDR)
#define	CACHED_TO_UNCACHED(x)	(PHYS_TO_UNCACHED(VA_TO_PHYS(x)))
#define UNCACHED_TO_CACHED(x)	(PHYS_TO_CACHED(VA_TO_PHYS(x)))
#define VA_TO_PHYS(x)		((unsigned long long)(x) & 0xffffffffffffULL)
#else
#define	PHYS_TO_CACHED(x)	((x) | CACHED_MEMORY_ADDR)
#define	PHYS_TO_UNCACHED(x) 	((x) | UNCACHED_MEMORY_ADDR)
#define	CACHED_TO_UNCACHED(x)	(PHYS_TO_UNCACHED(VA_TO_PHYS(x)))
#define UNCACHED_TO_CACHED(x)	(PHYS_TO_CACHED(VA_TO_PHYS(x)))
#define VA_TO_PHYS(x)		((x) & 0xffffffffffff)
#endif

#ifdef _KERNEL
/*
 * Location of exception vectors.
 */
#define TLB_MISS_EXC_VEC	CACHED_MEMORY_ADDR
#define XTLB_MISS_EXC_VEC	PHYS_TO_CACHED(0x80)
#define CACHE_ERR_EXC_VEC	CACHED_MEMORY_ADDR
#define GEN_EXC_VEC		CACHED_MEMORY_ADDR

/*
 * definitions of cpu-dependent requirements
 * referenced in generic code
 */
#define	COPY_SIGCODE		/* copy sigcode above user stack in exec */

#define	cpu_wait(p)		/* nothing */
#define cpu_swapout(p)		panic("cpu_swapout: can't get here");

#ifndef _LOCORE
/*
 * Arguments to hardclock and gatherstats encapsulate the previous
 * machine state in an opaque clockframe.
 */
struct clockframe {
	int	pc;	/* program counter at time of interrupt */
	int	sr;	/* status register at time of interrupt */
	int	cr;	/* cause register at time of interrupt */
};

#define	CLKF_USERMODE(framep)	(0)
#define	CLKF_BASEPRI(framep)	(0)
//#define	CLKF_PC(framep)		((framep)->pc)
#define	CLKF_INTR(framep)	(0)

/*
 * Preempt the current process if in interrupt from user mode,
 * or after the current trap/syscall if in system mode.
 */
#define	need_resched()	{ want_resched = 1; aston(); }

/*
 * Give a profiling tick to the current process when the user profiling
 * buffer pages are invalid.  On the PICA, request an ast to send us
 * through trap, marking the proc as needing a profiling tick.
 */
#define	need_proftick(p)	{ (p)->p_flag |= P_OWEUPC; aston(); }

/*
 * Notify the current process (p) that it has a signal pending,
 * process as soon as possible.
 */
#define	signotify(p)	aston()

#define aston()		(astpending = 1)

volatile int astpending;	/* need to trap before returning to user mode */
int	want_resched;	/* resched() was called */

/*
 * CPU identification, from PRID register.
 */
union cpuprid {
	int	cpuprid;
	struct {
#if BYTE_ORDER == BIG_ENDIAN
		unsigned int	pad1:16;	/* reserved */
		unsigned int	cp_imp:8;	/* implementation identifier */
		unsigned int	cp_majrev:4;	/* major revision identifier */
		unsigned int	cp_minrev:4;	/* minor revision identifier */
#else
		unsigned int	cp_minrev:4;	/* minor revision identifier */
		unsigned int	cp_majrev:4;	/* major revision identifier */
		unsigned int	cp_imp:8;	/* implementation identifier */
		unsigned int	pad1:16;	/* reserved */
#endif
	} cpu;
};

/*
 * CTL_MACHDEP definitions.
 */
#define	CPU_CONSDEV		1	/* dev_t: console terminal device */
#define	CPU_MAXID		2	/* number of valid machdep ids */

#define CTL_MACHDEP_NAMES { \
	{ 0, 0 }, \
	{ "console_device", CTLTYPE_STRUCT }, \
}

#endif /* !_LOCORE */
#endif /* _KERNEL */
#if defined(_KERNEL) && !defined(_LOCORE)
union	cpuprid CpuProcessorId;

struct tlb;
struct user;

void flushcache(void);
void flushicache(void *, unsigned long);
void flushdcache(void *, unsigned long);
void syncicache(void *, unsigned long);
void delay(int);
unsigned int get_timer(unsigned int base);

void	CPU_ConfigCache(void);
void	CPU_SetWIRED(int);
void	CPU_SetPID(int);
unsigned int	CPU_GetCOUNT(void);
unsigned long CPU_GetCOUNT64(void);
unsigned int	CPU_GetCONFIG(void);
void	CPU_SetCOMPARE(unsigned int);
int	CPU_SetSR(unsigned int, unsigned int);

void	CPU_FlushCache(void);
void	CPU_FlushDCache(unsigned long, unsigned long);
void	CPU_HitFlushDCache(unsigned long, unsigned long);
void	CPU_IOFlushDCache(unsigned long, unsigned long, int);
void	CPU_FlushICache(unsigned long, unsigned long);

unsigned int   cpu_icache_size;        /* Bytes per way */
unsigned int   cpu_icache_way; /* Number of ways */
unsigned int   cpu_icache_sets;        /* Number of lines per set */
unsigned int   cpu_icache_linesz;      /* Size of line in bytes */
unsigned int   cpu_dcache_size;
unsigned int   cpu_dcache_way;
unsigned int   cpu_dcache_sets;
unsigned int   cpu_dcache_linesz;
unsigned int   cpu_l2_cache_size;
unsigned int   cpu_l2_cache_way;
unsigned int   cpu_l2_cache_sets;
unsigned int   cpu_l2_cache_linesz;
unsigned int   cpu_l3_cache_size;
unsigned int   cpu_l3_cache_way;
unsigned int   cpu_l3_cache_sets;
unsigned int   cpu_l3_cache_linesz;

#define Index_store_tag_I      0
#define Index_store_tag_D      1
#define Index_store_tag_V      2
#define Index_Store_Tag_S      3
#define Index_Inv_I            8
#define Index_Inv_Wtbk_D       9
#define Index_Inv_Wtbk_V       10
#define Index_Inv_Wtbk_S       11
#define Hit_Inv_I              16
#define Hit_Inv_Wtbk_D         17
#define Hit_Inv_Wtbk_S         19
#define Hit_Invalidate_D        0x11
#define Hit_Writeback_Inv_D     0x15

void	CPU_TLBFlush(int);
void	CPU_TLBFlushAddr(unsigned long);
void	CPU_TLBWriteIndexed(int, struct tlb *);
void	CPU_TLBUpdate(unsigned long, unsigned);
void	CPU_TLBRead(int, struct tlb *);

void	wbflush(void);
void	savectx(struct user *, int);
int	copykstack(struct user *);
void	switch_exit(void);
void	MipsSaveCurFPState(struct proc *);

extern unsigned int cpu_counter_interval;  /* Number of counter ticks/tick */
extern unsigned int cpu_counter_last;      /* Last compare value loaded    */

#else
#ifndef _LOCORE
void delay(int);
#endif
#endif /* _KERNEL */
//
// Location of the saved registers relative to ZERO.
// Usage is p->p_regs[XX].
//

#endif /* !_LOONGARCH_CPU_H_ */
