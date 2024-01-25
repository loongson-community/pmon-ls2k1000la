/*
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

#ifndef _KERNEL
#define _KERNEL
#endif

#include <asm.h>
#include <regnum.h>
#include <cpu.h>
#include <pte.h>

#include "pmon/dev/ns16550.h"
#include "target/bonito.h"

#include "loongson3_def.h"

/*
 *	Register usage:
 *
 *	s0	link versus load offset, used to relocate absolute adresses.
 */

	.globl	_start
	.globl	start
	.globl	__main
_start:
start:
	.globl	stack
stack = start + LOCK_CACHE_SIZE		/* Place PMON stack in the end of RAM */
	/* enable perf counter as cp0 counter */
	li.w	t0, (0x1 << 16)
	csrxchg t0, t0, 0x200

	/* open auto flush SFB */
	li.w	t0, (0x1 << 9)
	csrxchg t0, t0, 0x80

	/* fast_ldq_dis close */
	li.w	t0, (0x1 << 12)
	csrxchg zero, t0, 0x80

	/* low power setting */
	li.w	t0, 0xe
	csrxchg t0, t0, 0xf0

	li.d	t0, UNCACHED_MEMORY_ADDR | 0x1
	csrwr   t0, 0x180
	li.d	t0, CACHED_MEMORY_ADDR | 0x11
	csrwr   t0, 0x181
/*
 * should before execution jr shutdown slave core
 * otherwise speculative execution cause error
 */
	/* SHUT_SLAVES */
	/* get core id */
	csrrd   t0, 0x20
	andi	t1, t0, 0x3
	andi    t2, t0, 0x1fc
	li.d	a0, 0x1fe001d0
	beqz    t2, 2f
	/* oher node only core 0 shutdown own node */
	bnez	t1, 3f
	/* Other nodes except node 0 */
	slli.d  t2, t2, (NODE_OFFSET - 2)
	or      a0, a0, t2
	st.w    zero,a0, 0
	b       3f
2:
	/* node 0 */
	li.d	a1, BOOTCORE_ID
	bne	a1, t1, 3f
	slli.w  a1, a1, 2
	li.d	t1, 0xf
	sll.w   a1, t1, a1
	st.w    a1, a0, 0

	/* MCC configure. */
	li.d	t0, 0x1fe00000
	li.d	t1, TOT_NODE_NUM
	slli.d	t1, t1, NODE_OFFSET
	or	t3, t0, t1
1:
#if	(TOT_NODE_NUM >= 2) && defined(MCC)
	/* MCC BI MODE */
	ldptr.d	t2, t0, 0x1514
	li.d	t1, 0x2
	or	t2, t2, t1
	stptr.d	t2, t0, 0x1514

	/* MCC MODE */
	ld.d	t2, t0, 0x400
	li.d	t1, 0x1000
	or	t2, t2, t1
	st.d	t2, t0, 0x400

	bl	chip_ver
	li.d	a1, 0x43
	bne	v0, a1, 2f
	/* Scache store fill */
	ld.d	t2, t0, 0x280
	li.d	t1, 0x80000000
	or	t2, t2, t1
	st.d	t2, t0, 0x280
2:
	/* Scache replace */
	ld.d	t2, t0, 0x284
	li.d	t1, 0x18
	or	t2, t2, t1
	st.d	t2, t0, 0x284
#endif
#ifndef	MCC
	/* mca clock */
	ldptr.w t1, t0, 0x180
	li.w	t2, ~((1 << 6) | (1 << 11))
	and     t1, t1, t2
	stptr.w t1, t0, 0x180
#endif
	li.d	t1, (1 << NODE_OFFSET)
	add.d	t0, t0, t1
	bne	t0, t3, 1b

	WATCHDOG_CLOSE

	/* spi speedup */
	li.d	t0, 0x1fe001f0
#ifdef	BONITO_100M
	li.w	t1, 0x47
	st.b    t1, t0, 0x4
#elif   BONITO_25M
	li.w	t1, 0x7
	st.b    t1, t0, 0x4
#endif
//#define SPI_QUAD_IO
#ifdef	SPI_QUAD_IO
	/* spi quad_io */
	li.w	t1, 0xb
	st.b    t1, t0, 0x6
1:
	ld.bu   t2, t0, 0x6
	bne     t2, t1, 1b
#endif
3:

	/* clear Vint cofigure */
	li.d	t0, (0x7 << 16)
	csrxchg zero, t0, 0x4
	/* set ebase address */
	li.d	t0, 0x1c001000
	csrwr   t0, 0xc
	/* set TLB excption address */
	li.d	t0, 0x000000001c001000
	csrwr   t0, 0x88

	/* enable llexc */
	li.w	t0, (1 << 3)
	csrxchg zero, t0, 0xc1

	/* disable interrupt */
	li.d	t0, (1 << 2)
	csrxchg zero, t0, 0x0

	la	sp, stack
	la	gp, _gp

	/* don't change this code,jumping to cached address */
	li.d	t1, CACHED_MEMORY_ADDR
	bl     1f
1:
	addi.d  t0, ra, 12
	or      t0, t1, t0
	jirl    zero, t0, 0
	/* now pc run to 0x90xxxxxxxxxxxxxx */

	/* DA disable for 0x90xxxxxxxxxxxxxx and 0x80xxxxxxxxxxxx address can be used */
	li.w	t0, 0xb0
	csrwr   t0, 0x0

	/* calculate ASM stage print function s0 address */
	la      s0, start
	li.d	a0, PHYS_TO_UNCACHED(0x1c000000)
	/* if change locked cache address may need change the following code */
	sub.d   s0, s0, a0
	li.d	a0, 0x00000000ffff0000
	and     s0, s0, a0

	la	sp, stack
	la	gp, _gp

	/* slave core run to slave_main */
	csrrd   t0, 0x20
	andi    t0, t0, 0x1ff
	andi    t1, t0, 0x3             /* core id */
	andi    t2, t0, 0x1fc           /* node id << 2 */

	/* get current core position in reserved_core_mask */
	add.w	t2, t2, t1
	li.w	t1, 1
	sll.d	t1, t1, t2

	li.d	t3, RESERVED_COREMASK
	and     t3, t3, t1
	bnez    t3, wait_to_be_killed

	li.d	t2, BOOTCORE_ID
	bne     t0, t2, slave_main
	b       1f

wait_to_be_killed:
	b	wait_to_be_killed
1:
	li.d	a0, GS3_UART_BASE
	bl	initserial

	PRINTSTR("Shut down slave cores done!\r\n")

bsp_start:
	PRINTSTR("\r\nPMON LoongArch Initializing. Standby...\r\n")

#if (TOT_NODE_NUM == 1)
	/* enable rd interleave */
	li.d	t0, (UNCACHED_MEMORY_ADDR |0x1fe00000)
	ld.d    t2, t0, 0x400
	li.d	t1, (0x1 << 13)
	or      t2, t2, t1
	st.d    t2, t0, 0x400
#endif

	bl	locate

	/* this code start address is 0x500 */
#include "resume.S"

	/* all exception entry */
	.org 0x1000
	/* s0 in different stage should fixup */
	la      a0, start
	li.d	a1, PHYS_TO_UNCACHED(0x1c000000)
	sub.d   a0, a0, a1
	li.d	a1, 0x00000000ffff0000
	and     a0, a0, a1
	beq	a0, s0, 1f
	move	s0, zero
1:
	and     s0, s0, a0
	csrrd	t0, 0x8a
	andi	t0, t0, 0x1
	bnez	t0, 2f

	PRINTSTR("\r\nCPU Trigger exception!\r\n")
	PRINT_CSR(0x4);
	PRINT_CSR(0x5);
	PRINT_CSR(0x6);
	PRINT_CSR(0x7);
	PRINT_CSR(0x8);
1:
	b	1b
2:
	li.d	t1, CACHED_MEMORY_ADDR
	bl     1f
1:
	addi.d  t0, ra, 12
	or      t0, t1, t0
	jirl    zero, t0, 0

	li.d	t0, 0xb0
	csrwr	t0, 0
	PRINTSTR("\r\nTLB exception!\r\n");
	PRINT_CSR(0x89);
	PRINT_CSR(0x8a);
1:
	b	1b

locate:

	/*sram ctrl*/
	LS3A5000_SRAM_CTRL
#ifdef	VOLTAGE_CTRL
	li.d	t0, TOT_NODE_NUM
#if	(TOT_NODE_NUM >= 8) && defined(BOARD_3C5000L_2W_V01)
	li.w	t1, 1
#else
	move	t1, zero
#endif
1:
	TTYDBG("\r\Node ")
	move	a0, t1
	bl	hexserial
	TTYDBG("\r\nN Voltage  write :\r\n")
	li.w	a0, VOL_mV(1200)
	move	a1, t1
	bl	v_n_ctrl
	TTYDBG("\r\nN Voltage  read :\r\n")
	move	a0, t1
	bl	v_n_ctrl_read

	TTYDBG("\r\nP Voltage write :\r\n")
#if	(CORE_FREQ > 2400)
	li.w	a0, VOL_mV(1050)
#else
	li.w	a0, VOL_mV(950)
#endif
	move	a1, t1
	bl	v_p_ctrl

#if	(TOT_NODE_NUM >= 8)
	addi.w	t1, t1, 4
#else
	addi.w	t1, t1, 1
#endif
	blt	t1, t0, 1b
#endif

	/* Read sys_clk_sel */
	TTYDBG ("\r\nPHYS_TO_UNCACHED(0x1fe00190)  : ")
	li.d	t2,PHYS_TO_UNCACHED(0x1fe00190)
	ld.d    t1, t2, 0x0
	srli.d  a0, t1, 32
	bl      hexserial
	or      a0, zero, t1
	bl      hexserial
	PRINTSTR ("\r\nCPU CLK SEL : ")
	srli.d  t1, t1, 32
	andi    a0, t1, 0x1f
	bl      hexserial
	PRINTSTR ("\r\nMEM CLK SEL : ")
	srli.d  t0, t1, 5
	andi    a0, t0, 0x1f
	bl      hexserial
	PRINTSTR ("\r\nHT CLK SEL : ")
	srli.d  t0, t1, 10
	andi    a0, t0, 0x3f
	bl      hexserial
	PRINTSTR ("\r\n")

	li.d	s2, TOT_NODE_NUM << NODE_OFFSET
	li.d	s3, 0x0000100000000000
	/* Using s1 to passing the node id */
	li.d	s1, 0x0000000000000000
clk_set_loop:
#include "loongson3_clksetting.S"
	add.d	s1, s1, s3
	bne	s1, s2, clk_set_loop

##########################################

	/*disable 0x3ff0_0000 that routing of configuration register space */
	li.d	t0, PHYS_TO_UNCACHED(0x1fe00400)
	ld.w    t1, t0, 0x0
	li.w	t2, (1 << 9)
	or      t1, t1, t2
	st.w    t1, t0, 0x0

	li.d	s1, TOT_NODE_NUM
	move    s2, zero
1:
	STABLE_COUNTER_CLK_EN
#if	(TOT_NODE_NUM >=  2)
	SET_GPIO_FUNC_EN(1 << 13)
#endif
	SET_NODEMASK(TOT_NODE_NUM - 0x1)
	addi.w  s2, s2, 0x1
	bne     s2, s1, 1b

#if	(TOT_NODE_NUM >=  2)
	/*sync stable counter*/
	GPIO_CLEAR_OUTPUT(1 << 12)
	GPIO_SET_OUTPUT(1 << 12)
#else
	/*stable_reset*/
	li.d	t0, PHYS_TO_UNCACHED(0x1fe00420)
	ld.w    t1, t0, 0x0
	li.w	t2, (1 << 21)
	or      t1, t1, t2
	st.w    t1, t0, 0x0
	ld.w    t1, t0, 0x0
	li.w	t2, (1 << 21)
	xor     t1, t1, t2
	st.w    t1, t0, 0x0
#endif

#include "loongson3_ht1_32addr_trans.S"
	PRINTSTR("loongson3_ht1_32addr_trans.S End\r\n")

/*
 *  Reset and initialize l1 caches to a known state.
 */
	PRINTSTR("\r\nlock scache ")
	li.d	a0, LOCK_CACHE_BASE
	bl	hexserial
	PRINTSTR(" - ")
	li.d	a0, LOCK_CACHE_BASE + LOCK_CACHE_SIZE
	bl	hexserial

	li.d	t0, PHYS_TO_UNCACHED(0x1fe00000)
	li.d	t1, ~(LOCK_CACHE_SIZE - 1)
	st.d	t1, t0, 0x248
	li.d	t1, (LOCK_CACHE_BASE & 0xffffffffffff) | (1 << 63)
	st.d	t1, t0, 0x208

	PRINTSTR("\r\nLock Scache Done.\r\n")
	/* copy flash code to scache */
	li.d	a1, PHYS_TO_CACHED(0x1c000000)
	la	a0, start
	la	a2, edata
1:
	ld.d	a3, a1, 0
	st.d	a3, a0, 0
	addi.d	a0, a0, 8
	addi.d	a1, a1, 8
	bne	a2, a0, 1b

	PRINTSTR("copy text section to lock cache done.\r\n")
	/*clear cache mem BSS in this space*/
	la	a0, _edata
	la	a1, _end
1:
	st.d	zero, a0, 0
	addi.d	a0, a0, 8
	blt	a0, a1, 1b

	li.d	a0, LOCK_CACHE_BASE
	li.d	a1, LOCK_CACHE_BASE + LOCK_CACHE_SIZE
1:
	/* ensure locked cache address accessible so this code can`t remove*/
	ld.d	zero, a0, 0
	addi.d	a0, a0, 0x40
	bne	a1, a0, 1b

	/* jump to locked cache address */
	/* ra addr is 0xffffffff9fcxxxxx */
	li.d	t0, PHYS_TO_CACHED(0x9000000c)
	li.d	t1, 0xfffff
	bl	1f
1:
	and	t1, ra, t1
	add.d	t1, t1, t0
	jirl	zero, t1, 0

	li.d	t1, TOT_NODE_NUM
	move	t2, zero
1:
	slli.d	t3, t2, 44
	/* clken_percore enable */
	li.d	a0, PHYS_TO_UNCACHED(0x1fe00420)
	or	a0, a0, t3
	/* close INT_encode */
	ld.w	a1, a0, 0x4
	li.w	t0, ~((1 << 17) | (1 << 16))
	and	a1, t0, a1
	st.w	a1, a0, 0x4
	/* SE */
	li.d	a0, PHYS_TO_UNCACHED(0x1fe00404)
	or	a0, a0, t3
	ld.w	a1, a0, 0
	li.w	a2, 0xf00000
	or	a1, a1, a2
	st.w	a1, a0, 0

	addi.w	t2, t2, 1
	bne	t1, t2, 1b

	PRINTSTR("run in cache.\r\n")
	move	s0, zero

	la	ra, cache_stage
	jirl	zero, ra, 0

LEAF(clear_mailbox)
	csrrd   t0, 0x20
	andi    t0, t0, 0x3ff
	andi    t1, t0, 0x3	/* core id */
	slli.d  t1, t1, 0x8
	andi    t2, t0, 0x1fc	/* node id */
	slli.d  t2, t2, (NODE_OFFSET - 2)
	or      t1, t2, t1
	li.d	t2, NODE0_CORE0_BUF0
	or      t1, t1, t2
	st.d    zero, t1, FN_OFF
	st.d    zero, t1, SP_OFF
	st.d    zero, t1, GP_OFF
	st.d    zero, t1, A1_OFF

	jirl zero, ra, 0x0
END(clear_mailbox)

LEAF(get_core_id)
       csrrd   a0, 0x20
       andi    a0, a0, 0x3ff
       jirl    zero, ra, 0x0
END(get_core_id)

slave_main:
#ifdef ACPI_SUPPORT
	bl	clear_mailbox

	bl	get_core_id	//get a0
	/*
	 * don't changing the following register
	 * a0, ap own cpu number set by get_core_id
	 * t2, node 0 mail box address set by clear_mailbox
	 */
1:
	li.w	t0, 0x1000

2:
	addi.w  t0, t0, -1
	bnez    t0, 2b

	ld.w    t0, t2, FN_OFF  //mailbox0
	beqz    t0, 1b
	ld.d    t0, t2, FN_OFF

	move    ra, t0

	ld.d    a1, t2, A1_OFF  //mailbox3

	li.d	t3, PHYS_TO_CACHED(0x84000000)

	slli.d  a2, a0, 0x9
	or      t0, a2, t3
	move	sp,	t0
	addi.d  sp, sp, -8

	jirl    zero, ra, 0x0		# jump to initlialize AP info function with "a0=ap cpu number"

	.globl asm_wait_for_kernel
asm_wait_for_kernel:
#endif
	/*
	 * don't changing the following register
	 * t2, each node mail box address
	 */
	bl      clear_mailbox

waitforinit:
	li.w	t0, 0x1000

idle1000:
	addi.w  t0, t0, -1
	bnez    t0, idle1000
	/*csr finally filled the low 32 bits*/
	ld.w    t0, t1, FN_OFF
	beqz    t0, waitforinit

	ld.d    t0, t1, FN_OFF
	move    ra, t0

	li.d	t3, CACHED_MEMORY_ADDR

	ld.d    t0, t1, SP_OFF
	or      t0, t0, t3
	move    sp, t0

	ld.d    t0, t1, GP_OFF
	or      t0, t0, t3
	move    gp, t0

#ifdef	ACPI_SUPPORT
	ld.d    a1, t1, A1_OFF
	or	a1, a1, t3
#endif
	/* slave core jump to kernel, byebye */
	jirl    zero, ra, 0x0
	/* end slave_main */


LEAF(get_cpuprid)
	csrrd	a0, 0xc0
	jirl    zero, ra, 0
END(get_cpuprid)
/*************************************
 *used: a0~a1
 *************************************/
LEAF(initserial)
	li.d	a0, PHYS_TO_UNCACHED(0x1fe001e0)

	li.d	a1, 0x80
	st.b    a1, a0, 3

#if 1
#ifdef BONITO_100M
	/* divider, highest possible baud rate, for 100M crystal*/
	li.d	a1, 0x36
#else
	/* divider, highest possible baud rate, for 25M crystal*/
	li.d	a1, 0x0d
#endif
#else
	/*33M*/
	li.d	a1, 0x12
#endif
	st.b    a1, a0, 0
	/* divider, highest possible baud rate*/
	li.d	a1, 0x0
	st.b    a1, a0, 1
	li.d	a1, 3
	st.b    a1, a0, 3

	li.d	a1, 0
	st.b    a1, a0, 1

	li.d	a1, 71
	st.b    a1, a0, 2
	jirl    zero, ra, 0
END(initserial)

/******************************************************
 *used: a0~a2
 ******************************************************/
LEAF(tgt_putchar)
	li.d	a1, GS3_UART_BASE
1:
	ld.bu   a2, a1, 0x5
	andi    a2, a2, 0x20
	beqz    a2, 1b

	st.b    a0, a1, 0
	//    or      a2, zero, a1

	jirl    zero, ra, 0
END(tgt_putchar)

/******************************************************
 *used: a0~a4, s0
 ******************************************************/
LEAF(stringserial)
	or      a4, ra, zero
	sub.d   a3, a0, s0
	ld.bu   a0, a3, 0
1:
	beqz    a0, 2f

	bl      tgt_putchar

	addi.d  a3, a3, 1
	ld.bu   a0, a3, 0
	b       1b

2:
	ori     ra, a4, 0
	jirl    zero, ra, 0
END(stringserial)

/*****************************************************
 *used: a0~a5, s0
 *****************************************************/
LEAF(hexserial)
	ori     a4, ra, 0
	ori     a3, a0, 0
	li.d	a5, 8
1:
	rotri.w a0, a3, 28
	or      a3, a0, zero
	andi    a0, a0, 0xf

	la     a1, hexchar
	sub.d   a1, a1, s0

	add.d   a1, a1, a0
	ld.bu   a0, a1, 0

	bl      tgt_putchar

	addi.d  a5, a5, -1
	bnez    a5, 1b

	ori     ra, a4, 0
	jirl    zero, ra, 0
END(hexserial)


/*****************************************************
 *used: a0~a5, s0
 *****************************************************/
LEAF(hexserial64)
	ori     a4, ra, 0
	ori     a3, a0, 0
	li.d	a5, 16
1:
	rotri.d a0, a3, 60
	or      a3, a0, zero
	andi    a0, a0, 0xf

	la     a1, hexchar
	sub.d   a1, a1, s0

	add.d   a1, a1, a0
	ld.bu   a0, a1, 0

	bl      tgt_putchar

	addi.d  a5, a5, -1
	bnez    a5, 1b

	ori     ra, a4, 0
	jirl    zero, ra, 0
END(hexserial64)

	.section .rodata
hexchar:
	.ascii  "0123456789abcdef"
	.text
	.align 5

LEAF(watchdog_close)
	WATCHDOG_CLOSE
	jirl zero, ra, 0
END(watchdog_close)

LEAF(tgt_testchar)
	li.d	a0, GS3_UART_BASE
	ld.bu	a1, a0, NSREG(NS16550_LSR)
	andi	a0, a1, LSR_RXRDY
	jirl	zero, ra, 0
END(tgt_testchar)

LEAF(tgt_getchar)
	li.d	a0, GS3_UART_BASE
1:
	ld.bu	a1, a0, NSREG(NS16550_LSR)
	andi	a1, a1, LSR_RXRDY
	beqz    a1, 1b
	ld.b	a0, a0, NSREG(NS16550_DATA)
	jirl	zero, ra, 0

END(tgt_getchar)

/*
3A5000HV	2.5GHz@1.25V
3A5000		2.5GHz@1.20V
3A5000LL	2.3GHz@1.15V
3A5000M		2.0GHz@1.05V

3A5000K		1.5GHz@0.90V
3A5000K-HV	2.0GHz@1.10V
3A5000i		1.5GHz@0.90V
3A5000i-HV	2.0GHz@1.10V

3B5000		2.3GHz@1.15V

3C5000L		2.2GHz@1.15V
3C5000L-LL	2.0GHz@1.05V
*/

LEAF(chip_ver)
	li.d	a0, PHYS_TO_UNCACHED(0x1fe00460)
	ld.w	a2, a0, 0x4
	li.d	a1, 29
	srl.w	a4, a2, a1
	li.d	a1, 13
	srl.w	v0, a2, a1
	andi	v0, v0, 0xff

	li.d    a1, 0x44
	bne	v0, a1, 1f
	li.d    v0, 0x43
1:
	li.d	a1, 0x43
	beq	v0, a1, 3f
	li.d	a1, 0x42
	beq	v0, a1, 3f
	li.d	a1, 0x41
	beq	v0, a1, 1f

	bnez	v0, 2f
	li.d	a1, 7
	srl.w	v0, a2, a1
	andi	a3, v0, 0x7
	li.d	a1, 2
	li.d	v0, 0x42
	beq	a3, a1, 3f
2:
	li.d    v0, 0x43  //default BA version
3:
	li.d	a1, 21
	srl.w	a2, a2, a1
	andi	a2, a2, 0xf

	li.d	a3, 0x5
	li.d	v1, 0x1
	beq	a2, a3, 5f

	li.d	v1, 0x1
	bne	a2, v1, 4f
	beqz	a4, 5f
4:
	li.d	v1, 0xf
5:
	jirl	zero, ra, 0
END(chip_ver)

LEAF(ls7a_version)
	li.d	a0, PHYS_TO_UNCACHED(0xefdfe000100)
	ld.bu	a0, a0, 0x8
	jirl	zero, ra, 0
END(ls7a_version)
#include "ls3a5000_vctrl.S"
