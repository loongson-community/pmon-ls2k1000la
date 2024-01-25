/*	$Id: machdep.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2000 Opsycon AB  (http://www.opsycon.se)
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
 *	This product includes software developed by
 *	Opsycon Open System Consulting AB, Sweden.
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

/*
 * Pmon architecture dependent (and possibly cpu type dependent) code.
 */

#include <stdio.h>
#include <termio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <machine/cpu.h>
#include <machine/frame.h>
#include <machine/regnum.h>
#include <machine/trap.h>

#include <pmon.h>

extern unsigned long long memorysize;
extern u_int8_t end[];
extern u_int8_t start[];
#ifdef GODSONEV1
extern int global_div_num;
#endif
/*
 *  CPU type markers.
 */
#define CPU_ALL		0x000f		/* Belongs to mask */
#define	CPU_7		0x0001		/* RM7000 */
#define	CPU_5		0x0002		/* R5000 */
#define CPU_GOD1	0x0004
#define CPU_GOD2	0x0008
#define CPU_GOD3	0x0008
#define	CPU_41		0x0010		/* R4100 */
#define	F_FMT		0x1000		/* Field uses format */

struct RegMap {
	char	width;
	char	bit;
	short	flags;
	const char * const name;
	union {
		const char * const fmt;
/* XXX Tell me why i must cast initializers for the next union? */
		const char * const *vn;
	} fe;
};

static void dsp_rregs __P((int *));
static void dsp_fregs __P((int *));

static int cputype;

const char *ls3a5000_cpu_name[] = {
        [0]	= "3A5000",
        [1]	= "3A5000-LL",
        [2]	= "RESERVE",
        [3]	= "3A5000M",
        [4]	= "3B5000",
        [5]	= "3C5000L",
        [6]	= "RESERVE",
        [7]	= "3C5000L-LL",
        [8]	= "3A5000K",
        [9]	= "3A5000K-HV",
        [10]	= "3A5000i",
        [11]	= "3A5000i-HV",
        [12]	= "3A5000BM",
        [13]	= "RESERVE",
        [14]	= "RESERVE",
        [15]	= "3A5000-HV",
};

/*
 *  Return a int value of the processor type.
 */
int
md_cpuinfo()
{
	return (readl(PHYS_TO_UNCACHED(0x1fe00464)) >> 21) & 0xf;
}
/*
 *  Return a ascii version of the processor type.
 */
const char *
md_cpuname()
{
/* XXX make more sophisticated dealing with rev numbers and 52x0 more precisely */
	int id = md_cpuinfo();
	if(ls3a5000_cpu_name[id] == NULL) {
		tgt_printf("error! There is no defined chip model, please check the firmware supported version and this chip version\r\n");
		while(1);
	}
	return  ls3a5000_cpu_name[id];
}

/*
 *  Dump out exception info for an unscheduled exception
 */
void
md_dumpexc(struct trapframe *tf)
{
#if 0
	int w = 100;

	printf("\r\nException Cause=%s, SR=%p, PC=%p\r\n",
		md_getexcname(tf), (uint64_t)md_getsr(tf), md_getpc(tf));
	printf("CONTEXT=%llp, XCONTEXT=%llp\r\n", tf->context, tf->xcontext);
	printf("BADVADDR=%llp, ENTHI=%llp\r\n", tf->badvaddr, tf->enthi);
	printf("ENTLO0=%llp, ENTLO1=%llp\r\n\r\n", tf->entlo0, tf->entlo1);
	dsp_rregs(&w);
	printf("\r\n");
	md_do_stacktrace(0, 0, 0, 0);
#endif
}

/*
 *  Set PC to a new value.
 */
void
md_setpc(struct trapframe *tf, register_t pc)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	tf->csrepc = (uint64_t)pc;
}

/*
 *  This function clears all client registers for launch.
 */
void
md_clreg(struct trapframe *tf)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
//	md_fpsave(tf);
}

/*
 *  This function sets the client stack pointer.
 */
void
md_setsp(struct trapframe *tf, register_t sp)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	tf->sp = sp;
}

/*
 *  This function sets the client sr to the given value.
 */
void
md_setsr(struct trapframe *tf, register_t sr)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	//tf->csrectl = sr;
}

/*
 *  This function returns the value of the clients sr.
 */
register_t
md_getsr(struct trapframe *tf)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	return (tf->csrestat);
}

/*
 *  This function sets the client sr to do a trace.
 */
void
md_settrace()
{
}

/*
 *  This function sets the client sr to do a trace.
 */
void
md_setlr(struct trapframe *tf, register_t lr)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	tf->ra = lr;
}

extern char *heaptop,*allocp1;

static int address_in_heap(paddr_t addr)
{
	if (addr < CACHED_TO_PHYS(heaptop) && addr >= CACHED_TO_PHYS(allocp1))
		return 1;
	else return 0;
}

/*
 *  This function returns true if the address range given is
 *  invalid for load, eg will overwrite PMON or its working
 *  areas or other protected memory areas or load into
 *  existing memory.
 */
int
md_valid_load_addr(first_addr, last_addr)
	paddr_t first_addr;
	paddr_t last_addr;
{
	first_addr = CACHED_TO_PHYS(first_addr);
	last_addr = CACHED_TO_PHYS(last_addr);

	if (((first_addr < (paddr_t)CACHED_TO_PHYS(end)) && (last_addr >(paddr_t)CACHED_TO_PHYS(start))) ||
		(last_addr > (paddr_t)memorysize)
		|| address_in_heap(first_addr)
		|| address_in_heap(last_addr)
		) {
		return (1);
	}
	return (0);
}

/*
 *  This function sets the SP to the new value given if it's
 *  non zero. The old value of sp is returned.
 */
register_t
md_adjstack(struct trapframe *tf, register_t newsp)
{
	register_t oldsp;

	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	oldsp = tf->sp;
	if (newsp != 0) {
		tf->sp = newsp;
	}
	return (oldsp);
}

/*
 *  This function sets the arguments to client code so
 *  the client sees the arguments as if it was called
 *  with the arguments.
 */
void
md_setargs(struct trapframe *tf, register_t a1, register_t a2,
		register_t a3, register_t a4)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	tf->a0 = a1;
	tf->a1 = a2;
	tf->a2 = a3;
	tf->a3 = a4;
}

void
md_setentry(struct trapframe *tf, register_t pc)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	tf->csrepc = pc;
}

/*
 *  This function returns the PC value that is supposed
 *  to be used at restore to client state. Do not confuse
 *  with the value of the exception PC. (Diff on some arches).
 */
void *
md_getpc(struct trapframe *tf)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	return ((void *)tf->csrepc);
}

/*
 *  This function is called from exception(). It's purpose
 *  is to decode the exception and return the exception
 *  type to the caller for further processing.
 */
#define MYINB(port) *(volatile unsigned char *)(port)
#define MYOUTB(val,port) *(volatile unsigned char *)(port)=val
int
md_exc_type(struct trapframe *frame)
{
#if 0
	switch ((frame->cause & CR_EXC_CODE) >> CR_EXC_CODE_SHIFT) {
	case T_INT:
		{
		struct trapframe *cpuinfo;
		cpuinfo = cpuinfotab[0];
		printf("inb(0x20)=%x inb(0x21)=%x inb(0xa0)%x inb(0xa1)=%x\n", MYINB(PHYS_TO_UNCACHED(0x1fd00020)), MYINB(PHYS_TO_UNCACHED(0x1fd00021)), MYINB(PHYS_TO_UNCACHED(0x1fd000a0)), MYINB(PHYS_TO_UNCACHED(0x1fd000a1)));
		cpuinfo->sr |= 1;
		_go();
		}
	case T_BREAK:
	case T_TRAP:
		return (EXC_BPT);

	case T_IWATCH:
	case T_DWATCH:
		return (EXC_WTCH);

	case T_RES_INST:
#ifdef BONITOEL
		return (EXC_RES);
#endif
	case T_TLB_MOD:
	case T_TLB_LD_MISS:
	case T_TLB_ST_MISS:
	case T_ADDR_ERR_LD:
	case T_ADDR_ERR_ST:
	case T_BUS_ERR_IFETCH:
	case T_BUS_ERR_LD_ST:
	case T_SYSCALL:
	case T_COP_UNUSABLE:
	case T_OVFLOW:
#ifndef  HANDLE_FPE
	case T_FPE:
#endif
		return (EXC_BAD);
#ifdef HANDLE_FPE
	case T_FPE:
		{
		struct trapframe *cpuinfo;
		cpuinfo = cpuinfotab[0];
		cpuinfo->pc += 4;
		_go();
		}
#endif

	case T_VCEI:
	case T_VCED:
		return (EXC_BAD);
	}
	return (EXC_BAD);
#endif
}

/*
 *  This function returns the value of the RA reg
 */
register_t
md_getlink(struct trapframe *tf)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	return (tf->ra);
}

/*
 *  This function returns a pointer from the value of the
 *  PC reg when an exception have been taken.
 */
void *
md_get_excpc(struct trapframe *tf)
{
	if (tf == NULL)
		tf = cpuinfotab[whatcpu];
	return ((void *)(uint64_t)tf->csrepc);
}

const char *
md_getexcname(struct trapframe *tf)
{
#if 0
	switch ((tf->cause & CR_EXC_CODE) >> CR_EXC_CODE_SHIFT) {

	case T_INT:
		return ("interrupt pending");

	case T_TLB_MOD:
		return ("TLB modified");

	case T_TLB_LD_MISS:
		return ("TLB miss on load or ifetch");

	case T_TLB_ST_MISS:
		return ("TLB miss on store");

	case T_ADDR_ERR_LD:
		return ("address error on load or ifetch");

	case T_ADDR_ERR_ST:
		return ("address error on store");

	case T_BUS_ERR_IFETCH:
		return ("bus error on ifetch");

	case T_BUS_ERR_LD_ST:
		return ("bus error on load or store");

	case T_SYSCALL:
		return ("system call");

	case T_BREAK:
		return ("breakpoint");

	case T_RES_INST:
		return ("reserved instruction");

	case T_COP_UNUSABLE:
		return ("coprocessor unusable");

	case T_OVFLOW:
		return ("arithmetic overflow");

	case T_TRAP:
		return ("trap instruction");

	case T_VCEI:
		return ("virtual coherency instruction");

	case T_FPE:
		return ("floating point");

	case T_IWATCH:
		return ("iwatch");

	case T_DWATCH:
		return ("dwatch");

	case T_VCED:
		return ("virtual coherency data");
	}
	return ("exception unknown");
#endif
}

/*
 *  ator returns either 32 bit or 64 bit conversion
 *  depending on CPU word length.
 */
int
md_ator(register_t *vp, char *p, int base)
{
#if __loongarch32
	return (atob(vp,p,base));
#else
	return (llatob(vp, p, base));
#endif
}

/*
 *  Returns true and sets the location pointed by vp to the value
 *  of the specified register or false if not recognized. Register
 *  names can be n the form <regno> r<nr> or sp.
 *  Some special registers are also detected.
 */

/*
 *  Floating point status register.
 */
#if 0
char * const rmvalues[] = {
    "rn", "rz", "rp", "rm", 0
};

const struct RegMap fsrmap[] = {
    {1, 24, CPU_ALL|F_FMT, "fs", {" %*b"}},
    {1, 23, CPU_ALL|F_FMT, "c",  {" %*b"}},
    {1, 17, CPU_ALL|F_FMT, "ce", {" %*b"}},
    {1, 16, CPU_ALL|F_FMT, "cv", {" %*b"}},
    {1, 15, CPU_ALL|F_FMT, "cz", {" %*b"}},
    {1, 14, CPU_ALL|F_FMT, "co", {" %*b"}},
    {1, 13, CPU_ALL|F_FMT, "cu", {" %*b"}},
    {1, 12, CPU_ALL|F_FMT, "ci", {" %*b"}},
    {1, 11, CPU_ALL|F_FMT, "ev", {" %*b"}},
    {1, 10, CPU_ALL|F_FMT, "ez", {" %*b"}},
    {1, 9,  CPU_ALL|F_FMT, "eo", {" %*b"}},
    {1, 8,  CPU_ALL|F_FMT, "eu", {" %*b"}},
    {1, 7,  CPU_ALL|F_FMT, "ei", {" %*b"}},
    {1, 6,  CPU_ALL|F_FMT, "fv", {" %*b"}},
    {1, 5,  CPU_ALL|F_FMT, "fz", {" %*b"}},
    {1, 4,  CPU_ALL|F_FMT, "fo", {" %*b"}},
    {1, 3,  CPU_ALL|F_FMT, "fu", {" %*b"}},
    {1, 2,  CPU_ALL|F_FMT, "fi", {" %*b"}},
    {2, 0,  CPU_ALL,       "rm", {(const char * const)rmvalues}},
    {0}
};

const char * const ksuvalues[] = {
    "kern", "supv", "user", "????", 0
};

const struct RegMap statmap[] = {
    {1, 31, CPU_5|CPU_7|F_FMT, "xx",  {" %*b"}},
    {4, 28, CPU_ALL|F_FMT,     "cu",  {" %*b"}},
    {1, 27, CPU_ALL|F_FMT,     "rp",  {" %*b"}},
    {1, 26, CPU_ALL|F_FMT,     "fr",  {" %*b"}},
    {1, 25, CPU_ALL|F_FMT,     "re",  {" %*b"}},
    {1, 24, CPU_ALL|F_FMT,     "its", {" %*b"}},
    {1, 22, CPU_ALL|F_FMT,     "bev", {" %*b"}},
    {1, 21, CPU_ALL|F_FMT,     "ts",  {" %*b"}},
    {1, 20, CPU_ALL|F_FMT,     "sr",  {" %*b"}},
    {1, 18, CPU_ALL|F_FMT,     "ch",  {" %*b"}},
    {1, 17, CPU_ALL|F_FMT,     "ce",  {" %*b"}},
    {1, 16, CPU_ALL|F_FMT,     "de",  {" %*b"}},
    {8, 8,  CPU_ALL|F_FMT,     "im",  {" %*b"}},
    {1, 7,  CPU_ALL|F_FMT,     "kx",  {" %*b"}},
    {1, 6,  CPU_ALL|F_FMT,     "sx",  {" %*b"}},
    {1, 5,  CPU_ALL|F_FMT,     "ux",  {" %*b"}},
    {2, 3,  CPU_ALL,	   "ksu", {(const char * const)ksuvalues}},
    {1, 2,  CPU_ALL|F_FMT,     "erl", {" %*b"}},
    {1, 1,  CPU_ALL|F_FMT,     "exl", {" %*b"}},
    {1, 0,  CPU_ALL|F_FMT,     "ie",  {" %*b"}},
    {0}
};

const char * const excodes[] = {
    "Int",  "MOD",  "TLBL", "TLBS",
    "AdEL", "AdES", "IBE",  "DBE",
    "Sys",  "Bp",   "RI",   "CpU",
    "Ovf",  "Trap", "VCEI", "FPE",
    "Cp2",  "Resv", "Resv", "Resv",
    "Resv", "Resv", "Resv", "Wtch",
    "Resv", "Resv", "Resv", "Resv",
    "Resv", "Resv", "Resv", "VCED",
    0
};


const struct RegMap causemap[] = {
    {1, 31, CPU_ALL|F_FMT,	  "bd",       {" %*b"}},
    {2, 28, CPU_ALL|F_FMT,	  "ce",       {" %*d"}},
    {1, 26, CPU_7|F_FMT,	    "w2",       {" %*b"}},
    {1, 25, CPU_7|F_FMT,	    "w1",       {" %*b"}},
    {1, 24, CPU_7|F_FMT,	    "iv",       {" %*b"}},
    {16, 8, CPU_7|F_FMT,	    "ipending", {" %*b"}},
    {8,  8, (CPU_ALL&~CPU_7)|F_FMT, "ipending", {" %*b"}},
    {4,  2, CPU_ALL,		"excode",   {(const char * const)excodes}},
    {0}
};

/* RM7000 interrupt control register */
const struct RegMap icrmap[] = {
    {8, 8, CPU_ALL|F_FMT, "im", {" %*b"}},
    {1, 7, CPU_ALL|F_FMT, "te", {" %*b"}},
    {5, 0, CPU_ALL|F_FMT, "vs", {" %*x"}},
    {0}};

const struct RegMap pridmap[] = {
    {8, 8, CPU_ALL|F_FMT, "imp", {" %*d"}},
    {8, 0, CPU_ALL|F_FMT, "rev", {" %*d"}},
    {0}
};

const char * const epvalues[] = {
    "DD", "DDx", "DDxx", "DxDx", "DDxxx", "DDxxxx", "DxxDxx", "DDxxxxx",
    "DxxxDxxx", "9?", "10?", "11?", "12?", "13?", "14?", "15?", 0};

const char * const ssvalues_5000[] = {
    "512Kb", "1Mb", "2Mb", "None", 0
};

const char * const coherencyvalues[] = {
    "wthr/na", "wthr/a", "uncd", "!chrnt",
    "excl", "excl/w", "updt", "rsvd",
    0};

const struct RegMap cfgmap[] =
{
    {1, 31, (CPU_ALL&~(CPU_5|CPU_7))|F_FMT, "cm", {" %*b"}},
    {1, 31, CPU_7|F_FMT,		    "sc", {" %*b"}},
    {3, 28, CPU_ALL|F_FMT,		  "ec", {" %*d"}},
    {4, 24, CPU_ALL,			"ep", {(const char * const)epvalues}},
    {2, 22, (CPU_ALL&~CPU_41)|F_FMT,	"sb", {" %*d"}},
    {1, 23, CPU_41|F_FMT,		   "ad", {" %*d"}},
    {1, 21, (CPU_ALL&~(CPU_5|CPU_7))|F_FMT, "ss", {" %*b"}},
    {1, 20, (CPU_ALL&~(CPU_5|CPU_7))|F_FMT, "sw", {" %*b"}},
    {2, 20, CPU_5,			  "ss", {(const char * const)ssvalues_5000}},
    {2, 20, CPU_7|F_FMT,		    "pi", {" %*d"}},
    {2, 18, CPU_ALL|F_FMT,		  "ew", {" %*d"}},
    {1, 17, (CPU_ALL&~CPU_7)|F_FMT,	 "sc", {" %*b"}},
    {1, 17, CPU_7|F_FMT,		    "tc", {" %*b"}},
    {1, 16, CPU_ALL|F_FMT,		  "sm", {" %*b"}},
    {1, 15, CPU_ALL|F_FMT,		  "be", {" %*b"}},
    {1, 14, CPU_ALL|F_FMT,		  "em", {" %*b"}},
    {1, 13, CPU_ALL|F_FMT,		  "eb", {" %*b"}},
    {1, 12, CPU_41|F_FMT,		   "cs", {" %*b"}},
    {1, 12, CPU_5|F_FMT,		    "se", {" %*b"}},
    {1, 12, CPU_7|F_FMT,		    "te", {" %*b"}},
    {3,  9, CPU_ALL|F_FMT,		  "ic", {" %*d"}},
    {3,  6, CPU_ALL|F_FMT,		  "dc", {" %*d"}},
    {1,  5, CPU_ALL|F_FMT,		  "ib", {" %*b"}},
    {1,  4, CPU_ALL|F_FMT,		  "db", {" %*b"}},
    {1,  3, (CPU_ALL&~(CPU_7|CPU_5))|F_FMT, "cu", {" %*b"}},
    {1,  3, CPU_ALL|F_FMT,		  "se", {" %*b"}},
    {3,  0, CPU_ALL|F_FMT,		  "k0", {" %*x"}},
    {0}};

const struct RegMap cerrmap[] = {
    {1, 31, CPU_ALL|F_FMT, "er", {" %*b"}},
    {1, 30, CPU_ALL|F_FMT, "ec", {" %*b"}},
    {1, 29, CPU_ALL|F_FMT, "ed", {" %*b"}},
    {1, 28, CPU_ALL|F_FMT, "et", {" %*b"}},
    {1, 27, CPU_ALL|F_FMT, "es", {" %*b"}},
    {1, 26, CPU_ALL|F_FMT, "ee", {" %*b"}},
    {1, 25, CPU_ALL|F_FMT, "eb", {" %*b"}},
    {1, 24, CPU_ALL|F_FMT, "ei", {" %*b"}},
    {1, 23, CPU_ALL|F_FMT, "ew", {" %*b"}},
    {22, 0, CPU_ALL|F_FMT, "sidx", {" %*x"}},	/* cheat lsb to get unshifted address */
    {3,  0, CPU_ALL|F_FMT, "pidx", {" %*x"}},
    {0}};

const struct RegMap enthimap[] = {
    {27, 13, CPU_ALL|F_FMT, "vpn2", {" %*x"}},
    {8,   0, CPU_ALL|F_FMT, "asid", {" %*d"}},
    {0}};


const struct RegMap entlomap[] = {
    {24, 6, CPU_ALL|F_FMT, "pfn", {" %*x"}},
    {3, 3, CPU_ALL,	"c", {(const char * const)coherencyvalues}},
    {1, 2, CPU_ALL|F_FMT,  "d", {" %*b"}},
    {1, 1, CPU_ALL|F_FMT,  "v", {" %*b"}},
    {1, 0, CPU_ALL|F_FMT,  "g", {" %*b"}},
    {0}};


#endif

#define TROFF(x) ((uint64_t)&(((struct trapframe *)0)->x) / sizeof(register_t))

const struct RegList {
	int		regoffs;	/* Location where register is stored */
	const struct RegMap *regmap;	/* Bit map for unpacking register info */
	const char	*regname;	/* Register name */
	const char	*regaltname;	/* Alternate register name */
	const int	flags;		/* Register status flags */
#define R_GPR	0x00010000			/* General purpose reg */
#define	R_64BIT	0x00020000			/* Width can be 64 bits */
#define	R_RO	0x00040000			/* Read only register */
#define	R_FLOAT	0x00080000			/* Floating point register */
} reglist[] = {
    {TROFF(zero),     0,	"zero", "0",     CPU_ALL | R_GPR | R_64BIT | R_RO},
    {TROFF(ra),       0,	"ra",   "1",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(gp),       0,	"gp",   "2",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(sp),       0,	"sp",   "3",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(a0),       0,	"a0",   "4",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(a1),       0,	"a1",   "5",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(a2),       0,	"a2",   "6",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(a3),       0,	"a3",   "7",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(a4),       0,	"a4",   "8",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(a5),       0,	"a5",   "9",     CPU_ALL | R_GPR | R_64BIT},
    {TROFF(a6),       0,	"a6",   "10",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(a7),       0,	"a7",   "11",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t0),       0,	"t0",   "12",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t1),       0,	"t1",   "13",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t2),       0,	"t2",   "14",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t3),       0,	"t3",   "15",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t4),       0,	"t4",   "16",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t5),       0,	"t5",   "17",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t6),       0,	"t6",   "18",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t7),       0,	"t7",   "19",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(t8),       0,	"t8",   "20",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(tp),       0,	"tp",   "21",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(fp),       0,	"fp",   "22",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s0),       0,	"s0",   "23",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s1),       0,	"s1",   "24",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s2),       0,	"s2",   "25",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s3),       0,	"s3",   "26",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s4),       0,	"s4",   "27",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s5),       0,	"s5",   "28",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s6),       0,	"s6",   "29",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s7),       0,	"s7",   "30",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(s8),       0,	"s8",   "31",    CPU_ALL | R_GPR | R_64BIT},
    {TROFF(csrcrmd),     0,	"csrcrmd", "csrcrmd",  CPU_ALL | R_64BIT},
    {TROFF(csrprmd),     0,	"csrprmd", "csrprmd",  CPU_ALL | R_64BIT},
    {TROFF(csrectl),    0,	"csrectl","csrectl", CPU_ALL | R_64BIT},
    {TROFF(csrestat),     0,	"csrestat", "csrestat",  CPU_ALL | R_64BIT},
    {TROFF(csrepc),      0,	"csrepc",  "csrepc",   CPU_ALL | R_64BIT},
    {TROFF(csrbadv),     0,	"csrbadv", "csrbadv",  CPU_ALL | R_64BIT},
    {TROFF(csrbadi),  0,	"csrbadi",  "csrbadi",   CPU_ALL | R_64BIT},

/* XXX Float regs must be in order so printing pairs work OK! */
    {TROFF(f0),       0,	"f0",	  "f0",       CPU_ALL | R_FLOAT},
    {TROFF(f1),       0,	"f1",	  "f1",       CPU_ALL | R_FLOAT},
    {TROFF(f2),       0,	"f2",	  "f2",       CPU_ALL | R_FLOAT},
    {TROFF(f3),       0,	"f3",	  "f3",       CPU_ALL | R_FLOAT},
    {TROFF(f4),       0,	"f4",	  "f4",       CPU_ALL | R_FLOAT},
    {TROFF(f5),       0,	"f5",	  "f5",       CPU_ALL | R_FLOAT},
    {TROFF(f6),       0,	"f6",	  "f6",       CPU_ALL | R_FLOAT},
    {TROFF(f7),       0,	"f7",	  "f7",       CPU_ALL | R_FLOAT},
    {TROFF(f8),       0,	"f8",	  "f8",       CPU_ALL | R_FLOAT},
    {TROFF(f9),       0,	"f9",	  "f9",       CPU_ALL | R_FLOAT},
    {TROFF(f10),      0,	"f10",	 "f10",      CPU_ALL | R_FLOAT},
    {TROFF(f11),      0,	"f11",	 "f11",      CPU_ALL | R_FLOAT},
    {TROFF(f12),      0,	"f12",	 "f12",      CPU_ALL | R_FLOAT},
    {TROFF(f13),      0,	"f13",	 "f13",      CPU_ALL | R_FLOAT},
    {TROFF(f14),      0,	"f14",	 "f14",      CPU_ALL | R_FLOAT},
    {TROFF(f15),      0,	"f15",	 "f15",      CPU_ALL | R_FLOAT},
    {TROFF(f16),      0,	"f16",	 "f16",      CPU_ALL | R_FLOAT},
    {TROFF(f17),      0,	"f17",	 "f17",      CPU_ALL | R_FLOAT},
    {TROFF(f18),      0,	"f18",	 "f18",      CPU_ALL | R_FLOAT},
    {TROFF(f19),      0,	"f19",	 "f19",      CPU_ALL | R_FLOAT},
    {TROFF(f20),      0,	"f20",	 "f20",      CPU_ALL | R_FLOAT},
    {TROFF(f21),      0,	"f21",	 "f21",      CPU_ALL | R_FLOAT},
    {TROFF(f22),      0,	"f22",	 "f22",      CPU_ALL | R_FLOAT},
    {TROFF(f23),      0,	"f23",	 "f23",      CPU_ALL | R_FLOAT},
    {TROFF(f24),      0,	"f24",	 "f24",      CPU_ALL | R_FLOAT},
    {TROFF(f25),      0,	"f25",	 "f25",      CPU_ALL | R_FLOAT},
    {TROFF(f26),      0,	"f26",	 "f26",      CPU_ALL | R_FLOAT},
    {TROFF(f27),      0,	"f27",	 "f27",      CPU_ALL | R_FLOAT},
    {TROFF(f28),      0,	"f28",	 "f28",      CPU_ALL | R_FLOAT},
    {TROFF(f29),      0,	"f29",	 "f29",      CPU_ALL | R_FLOAT},
    {TROFF(f30),      0,	"f30",	 "f30",      CPU_ALL | R_FLOAT},
    {TROFF(f31),      0,	"f31",	 "f31",      CPU_ALL | R_FLOAT},
#if 0
    {TROFF(fsr),      fsrmap,   "fsr",	 "fsr",      CPU_ALL},
#if __loongarch64
    {TROFF(xcontext), 0,	"c0_xcontext", "xcontext", CPU_ALL | R_64BIT},
#endif
    {TROFF(icr),     icrmap,    "c0c_icr", "icr",	  CPU_7},
    {TROFF(ipllo),   0,	 "c0c_ipllo", "ipllo",      CPU_7},
    {TROFF(iplhi),   0,	 "c0c_iplhi", "iplhi",      CPU_7},
#if 0
    {0, 0, "C0_TLB", "TLB", 0, (F_ALL | F_RO)},
#endif
    {TROFF(index),   0,	 "c0_index",  "index",      CPU_ALL},
    {TROFF(random),  0,	 "c0_random", "random",     CPU_ALL},
    {TROFF(config),  cfgmap,    "c0_config", "config",     CPU_ALL},
    {TROFF(prid),    pridmap,   "c0_prid",   "prid",       CPU_ALL},
#endif
    {-1}};

static void unpack_fields __P((char *, char *, const struct RegList *, register_t));

int
md_getreg(vp, p)
	register_t *vp;
	char *p;
{
	register_t *rp;

	if (md_getregaddr(&rp, p)) {
		*vp = *rp;
		return (1);
	}
	return (0);
}

/*
 *  Get address to register given as name.
 */
int
md_getregaddr(r, p)
	register_t **r;
	char *p;
{
	int i;

 	for (i = 0; reglist[i].regoffs >= 0; i++) {
		if (!(reglist->flags & cputype)) {
			continue;
		}
		if (strcasecmp(p, reglist[i].regname) == 0) {
			break;
		}
		if (strcasecmp(p, reglist[i].regaltname) == 0) {
			break;
		}
	}
	if (reglist[i].regoffs < 0) {
		return (0);
	}
	if (r) {
		*r = (register_t *)cpuinfotab[whatcpu] + reglist[i].regoffs;
	}
	return (1);
}

static void
unpack_fields(hp, vp, reglist, reg)
	char *hp;
	char *vp;
	const struct RegList *reglist;
	register_t reg;
{
#if 0
	int nl, vl;
	register_t xv;
	const char * const *vs;
	const struct RegMap *regmap;

	hp += sprintf(hp, "	  ");
	vp += sprintf(vp, "	  ");
	regmap = reglist->regmap;
	while (regmap->width) {
		if (!(regmap->flags & cputype)) {
			regmap++;
			continue;
		}

		xv = (reg >> regmap->bit) & ((1 << regmap->width) -1);
		nl = strlen(regmap->name);
		if (regmap->flags & F_FMT) {
			vl = sprintf(vp, regmap->fe.fmt, 0, xv);
		}
		else {
			vl = nl;
			for (vs = regmap->fe.vn; *vs; vs++) {
				int z = strlen(*vs);
				if (z > vl) {
					vl = z;
				}
			}
		}
		if (vl > nl)
			nl = vl;
		else
			vl = nl; /* Ugly! :) */
		hp += sprintf(hp, " %~*s", nl, regmap->name);
		if (regmap->flags & F_FMT) {
			vp += sprintf(vp, regmap->fe.fmt, vl, xv);
		}
		else {
			vp += sprintf(vp, " %*s", vl, regmap->fe.vn[xv]);
		}
		regmap++;
	}
#endif
}

int
md_disp_as_reg(r, c, w)
	register_t *r;
	char *c;
	int *w;
{
#if 0
	int i;
	char vbuf[100];

	for (i = 0; reglist[i].regoffs >= 0; i++) {
		if (!(reglist->flags & cputype)) {
			continue;
		}
		if (strcasecmp(c, reglist[i].regname) == 0 ||
			 strcasecmp(c, reglist[i].regaltname) == 0) {
			/* Use saved regvalue if value not given */
			register_t regval = *((register_t *)cpuinfotab[whatcpu] + reglist[i].regoffs);
			char *vf;

			if (r != NULL) {
				regval = *r;
			}

			if (matchenv("regsize") == REGSZ_64 &&
			   reglist[i].flags & R_64BIT) {
				vf = "%8s: %016llx ";
			}
			else {
				vf = "%8s: %08x ";
			}
			sprintf(prnbuf, vf, reglist[i].regname, regval);
			(void)more(prnbuf, w, moresz);

			if (reglist[i].regmap) {
				unpack_fields(prnbuf, vbuf, &reglist[i], regval);
				(void)more(prnbuf, w, moresz);
				(void)more(vbuf, w, moresz);
			}
			return (1);
		}
	}
	return (0);
#endif
}

const Optdesc md_r_opts[] = {
	{"*", "display all registers"},
	{"r*", "display all general registers"},
	{"f*", "display all fp registers"},
	{"reg value", "set specified register"},
	{0}
};

static void
dsp_rregs(w)
	int *w;
{
	int i;
	char *hp;
	char values[100], *vp;
	char *hf, *vf;

	if (matchenv ("regsize") == REGSZ_64) {
		hf = " %~16s";
		vf = " %016llx";
	} else {
		hf = " %~8s";
		vf = " %08x";
	}

	hp = prnbuf;
	vp = values;
#ifdef GODSONEV1
	hp += sprintf(hp, hf, "div_num");
	vp += sprintf(vp, vf, global_div_num);
#endif
	for (i = 0; reglist[i].regoffs >= 0; i++) {
		if (!(reglist[i].flags & R_GPR) || !(reglist[i].flags & cputype)) {
			continue;
		}
		hp += sprintf(hp, hf, reglist[i].regname);
		vp += sprintf(vp, vf,
		    *((register_t *)cpuinfotab[whatcpu] + reglist[i].regoffs));
		if ((vp - values) > 65) {
			(void)more(prnbuf, w, moresz);
			sprintf(prnbuf, "%s", values);
			(void)more(prnbuf, w, moresz);
			hp = prnbuf;
			vp = values;
		}
	}
	if (vp > values) {
		(void)more(prnbuf, w, moresz);
		sprintf(prnbuf, "%s", values);
		(void)more(prnbuf, w, moresz);
	}
}

static void
dsp_fregs(w)
	int *w;
{
#if 0
	int i;
	char *hp;
	char values[100], *vp;
	char *hf, *vf;

	if (matchenv ("fpfmt") == REGSZ_NONE) {
		hf = " %~16s ";
		vf = " %016llx ";

		hp = prnbuf;
		vp = values;
		for (i = 0; reglist[i].regoffs >= 0; i++) {
			if (!(reglist[i].flags & R_FLOAT) || !(reglist[i].flags & cputype)) {
				continue;
			}
			hp += sprintf(hp, hf, reglist[i].regname);
			vp += sprintf(vp, vf,
			    *((register_t *)cpuinfotab[whatcpu] + reglist[i].regoffs));
			if ((vp - values) > 65) {
				(void)more(prnbuf, w, moresz);
				sprintf(prnbuf, "%s", values);
				(void)more(prnbuf, w, moresz);
				hp = prnbuf;
				vp = values;
			}
		}
		if (vp > values) {
			(void)more(prnbuf, w, moresz);
			sprintf(prnbuf, "%s", values);
			(void)more(prnbuf, w, moresz);
		}
	}
	else {
		printf("TODO: FP printout in float mode, only 'none' so far!\n");
	}
	sprintf(prnbuf, " ");
	(void)more(prnbuf, w, moresz);
	md_disp_as_reg(NULL, "fsr", w);
#endif
}

/*
 *  Machine dependent register display/modify code.
 */
int
md_registers(ac, av)
	int ac;
	char *av[];
{
	register_t *rp;
	register_t rn;
	int w = moresz;

	ioctl (STDIN, CBREAK, NULL);
	switch (ac) {
	case 1: /* No args, display general registers */
		dsp_rregs(&w);
		break;

	case 2:
		if (strcmp(av[1], "*") == 0) {
			dsp_rregs(&w);
			sprintf(prnbuf, " ");
			(void)more(prnbuf, &w, moresz);
			md_disp_as_reg(NULL, "csrestat", &w);
			md_disp_as_reg(NULL, "csrepc", &w);
			md_disp_as_reg(NULL, "csrbadv", &w);
			md_disp_as_reg(NULL, "csrbadi", &w);
		}
		else if (strcmp(av[1], "r*") == 0) {
			dsp_rregs(&w);
		}
		else if (strcmp(av[1], "f*") == 0) {
			dsp_fregs(&w);
		}
		else {
			if (!md_disp_as_reg(NULL, av[1], &w)) {
				printf("%s: unkown.\n", av[1]);
			}
		}
		break;

	case 3:
		if (md_getregaddr(&rp, av[1]) == 0) {
			printf("%s: unkown.\n", av[1]);
			return (-1);
		}
		if (!get_rsa_reg(&rn, av[2])) {
			return (-1);
		}
		*rp = rn;
		break;
	}

	return (0);
}


/*
 *  Cache handling.
 */

void
flushcache()
{
	CPU_FlushCache();
}

void
flushicache(void *p, unsigned long s)
{
	CPU_FlushICache(CACHED_MEMORY_ADDR, cpu_icache_size);
}

void
flushdcache(void *p, unsigned long s)
{
       CPU_FlushDCache(CACHED_MEMORY_ADDR, cpu_dcache_size);
}

void
syncicache(void *p, unsigned long s)
{
	CPU_FlushDCache((vm_offset_t)p, s);
	CPU_FlushICache((vm_offset_t)p, s);
}

void
flush_cache (type, adr)
	int type;
	void *adr;
{

	switch (type) {
	case ICACHE:
		flushicache((void *)0, memorysize);
		break;

	case DCACHE:
		flushdcache((void *)0, memorysize);
		break;

	case IADDR:
		syncicache((void *)((uint64_t)adr & ~3), 4);
		break;

	case ICACHE|DCACHE:
		flushcache();
		break;
	}
}
