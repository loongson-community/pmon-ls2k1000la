/*
 * Copyright (c) 2020 Baoqi Zhang (zhangbaoqi@loongson.cn)
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
 *	This product includes software developed for Rtmx, Inc by
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
#define DEBUG

#ifndef _KERNEL
#define _KERNEL
#endif

#include <asm.h>
#include <regnum.h>
#include <cpu.h>

#define STORE   st.d      /* 64 bit mode regsave instruction */
#define LOAD    ld.d      /* 64 bit mode regload instruction */
#define	RSIZE   8       /* 64 bit mode register size */
#define	RLOGSIZE 3

	.data
/*  Register area outgoing */
	.global DBGREG
	.common DBGREG, 128*8

/*  return value and jump buffer used when exiting program returns. */
	.global	retvalue
retvalue:
	.word	0
	.globl	go_return_jump
	.common go_return_jump, 12*8

/*
*  Exception trampoline copied down to RAM after initialization.
*/
	.text
	.globl  LoongArchException
	.globl  LoongArchExceptionEnd
LoongArchException:
	csrwr   t0, 0x30
	csrwr   t1, 0x31

	la.abs  t0, exception_handler
	jirl    zero, t0, 0
LoongArchExceptionEnd:

/*
 *  Restore register state and transfer to new PC value.
 */
LEAF(_go)
	la     t0, DBGREG
#if 0
	li.w	t1, 7
	csrxchg zero, t1, LA_CSR_CRMD
	li.w	t1, ~0x7
	ld.d    t2, t0, LA_CSR_CRMD + BASE_NUM
	csrxchg t2, t1, LA_CSR_CRMD
#endif
	ld.d    t1, t0, (LA_CSR_CRMD + BASE_NUM) * RSIZE
	csrwr   t1, LA_CSR_CRMD

	LOAD    t1, t0, (LA_CSR_PRMD + BASE_NUM) * RSIZE
	csrwr   t1, LA_CSR_PRMD

	LOAD    t1, t0, (LA_CSR_ECTL + BASE_NUM) * RSIZE
	csrwr   t1, LA_CSR_ECTL

	LOAD    t1, t0, (LA_CSR_EPC + BASE_NUM) * RSIZE
	csrwr   t1, LA_CSR_EPC

 	LOAD    ra, t0, RA * RSIZE
 	LOAD    gp, t0, GP * RSIZE
 	LOAD    sp, t0, SP * RSIZE
 	LOAD    a0, t0, A0 * RSIZE
 	LOAD    a1, t0, A1 * RSIZE
 	LOAD    a2, t0, A2 * RSIZE
 	LOAD    a3, t0, A3 * RSIZE
 	LOAD    a4, t0, A4 * RSIZE
 	LOAD    a5, t0, A5 * RSIZE
 	LOAD    a6, t0, A6 * RSIZE
 	LOAD    a7, t0, A7 * RSIZE
 	LOAD    t2, t0, T2 * RSIZE
 	LOAD    t3, t0, T3 * RSIZE
 	LOAD    t4, t0, T4 * RSIZE
 	LOAD    t5, t0, T5 * RSIZE
 	LOAD    t6, t0, T6 * RSIZE
 	LOAD    t7, t0, T7 * RSIZE
 	LOAD    t8, t0, T8 * RSIZE
 	LOAD    tp, t0, TP * RSIZE
 	LOAD    fp, t0, FP * RSIZE
 	LOAD    s0, t0, S0 * RSIZE
 	LOAD    s1, t0, S1 * RSIZE
 	LOAD    s2, t0, S2 * RSIZE
 	LOAD    s3, t0, S3 * RSIZE
 	LOAD    s4, t0, S4 * RSIZE
 	LOAD    s5, t0, S5 * RSIZE
 	LOAD    s6, t0, S6 * RSIZE
 	LOAD    s7, t0, S7 * RSIZE
 	LOAD    s8, t0, S8 * RSIZE

 	csrrd   t0, 0x30
 	csrrd   t1, 0x31
    
	ertn
END(_go)

/*
 *  Top return address set to this func so a program returning
 *  is catched and control gracefully passed to PMON2000.
 */
LEAF(_exit)
	LA	a0, go_return_jump
	la	a1, longjmp
	jirl	zero, a1, 0
END(_exit)


/*
 *  Main exception handler. Not really a leaf routine but not a normal
 *  function either. Save away the entire cpu state end enter exception mode.
 */
LEAF(exception_handler)
	/* clear    space */
	la     t1, start
	addi.d  t0, t1, -1024
1:
	STORE   zero, t0, 0x0
	addi.d  t0, t0, 0x8
	bne     t1, t0, 1b

	// store registers in space
	la     t1, start
	addi.d  t0, t1, -1024
	STORE   ra, t0, RA * RSIZE
	STORE   gp, t0, GP * RSIZE
	STORE   sp, t0, SP * RSIZE
	STORE   a0, t0, A0 * RSIZE
	STORE   a1, t0, A1 * RSIZE
	STORE   a2, t0, A2 * RSIZE
	STORE   a3, t0, A3 * RSIZE
	STORE   a4, t0, A4 * RSIZE
	STORE   a5, t0, A5 * RSIZE
	STORE   a6, t0, A6 * RSIZE
	STORE   a7, t0, A7 * RSIZE
	STORE   t2, t0, T2 * RSIZE
	STORE   t3, t0, T3 * RSIZE
	STORE   t4, t0, T4 * RSIZE
	STORE   t5, t0, T5 * RSIZE
	STORE   t6, t0, T6 * RSIZE
	STORE   t7, t0, T7 * RSIZE
	STORE   t8, t0, T8 * RSIZE
	STORE   tp, t0, TP * RSIZE
	STORE   fp, t0, FP * RSIZE
	STORE   s0, t0, S0 * RSIZE
	STORE   s1, t0, S1 * RSIZE
	STORE   s2, t0, S2 * RSIZE
	STORE   s3, t0, S3 * RSIZE
	STORE   s4, t0, S4 * RSIZE
	STORE   s5, t0, S5 * RSIZE
	STORE   s6, t0, S6 * RSIZE
	STORE   s7, t0, S7 * RSIZE
	STORE   s8, t0, S8 * RSIZE

	csrrd   t2, LA_CSR_CRMD
	STORE   t2, t0, (LA_CSR_CRMD + BASE_NUM)  * RSIZE
	csrrd   t2, LA_CSR_PRMD
	STORE   t2, t0, (LA_CSR_PRMD + BASE_NUM)  * RSIZE
	csrrd   t2, LA_CSR_ECTL

	li.d	t1, ~0x800
	and     t1, t2, t1
	csrwr   t1, LA_CSR_ECTL

	STORE   t2, t0, (LA_CSR_ECTL + BASE_NUM) * RSIZE
	csrrd   t2, LA_CSR_ESTAT
	STORE   t2, t0, (LA_CSR_ESTAT + BASE_NUM) * RSIZE
	csrrd   t2, LA_CSR_EPC
	STORE   t2, t0, (LA_CSR_EPC + BASE_NUM)   * RSIZE
	csrrd   t2, LA_CSR_BADV
	STORE   t2, t0, (LA_CSR_BADV + BASE_NUM)  * RSIZE
	csrrd   t2, LA_CSR_BADI
	STORE   t2, t0, (LA_CSR_BADI + BASE_NUM)  * RSIZE

	la     t1, exception
	or      a0, t0, zero
	jirl    zero, t1, 0
END(exception_handler)

/*
 *  Save/restore floating point registers.
 *  Call with a0 pointing at frame.
 */
LEAF(md_fpsave)
	jirl	zero, ra, 0
END(md_fpsave)

LEAF(md_fprestore)
	jirl	zero, ra, 0
END(md_fprestore)

/*
 *  Get CP0 count register value
 */
LEAF(CPU_GetCOUNT64)
LEAF(CPU_GetCOUNT)
	csrrd   v0, 0x201
	srli.d  v0, v0, 1
	jirl    zero, ra, 0
END(CPU_GetCOUNT)
END(CPU_GetCOUNT64)

LEAF(cpu_set_tlb_ebase)
	li.d	t0, CACHED_MEMORY_ADDR
	csrwr   t0, 0x88
	jirl    zero, ra, 0
END(cpu_set_tlb_ebase)

LEAF(cpu_set_ebase)
	/*clear Vint cofigure*/
	csrrd   t0, 0x4
	li.d	t1, ~0x70000
	and     t0, t0, t1
	csrwr   t0, 0x4
	/*set ebase*/
	li.d	t0, CACHED_MEMORY_ADDR
	csrwr   t0, 0xc
	jirl    zero, ra, 0
END(cpu_set_ebase)

/*
 *  Helper routine to move a quad word in one operation.
 */
LEAF(movequad)
	LOAD	a1, a1, 0
	STORE	a1, a0, 0
	jirl	zero, ra, 0
END(movequad)

/*
 *  Return CPU type.
 */
LEAF(md_cputype)
	csrrd	a0, 0xc0
	jirl    zero, ra, 0
END(md_cputype)


/*
 *  execute a break instruction.
 */
LEAF(_pmon_break)
	break	0
	jirl    zero, ra, 0
END(_pmon_break)
