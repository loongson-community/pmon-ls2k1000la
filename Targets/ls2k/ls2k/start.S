/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited (www.loongson.cn)
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
#include "target/ls2k1000.h"

#include "target/cacheops.h"

#define DEBUG_LOCORE
#ifdef DEBUG_LOCORE
#define TTYDBG(x) \
	.section .rodata;98: .asciz x; .text; la a0, 98b; bl stringserial
#define TTYDBG_COM1(x) \
	.section .rodata;98: .asciz x; .text; la a0, 98b; bl stringserial_COM1
#else
#define TTYDBG(x)
#define TTYDBG_COM1(x)
#endif
#define	PRINTSTR TTYDBG
#define PRINT_CSR(offset)	\
	PRINTSTR("\r\ncsr 0x");	\
	li.w	a0, offset;	\
	bl	hexserial;	\
	PRINTSTR(" ->0x");	\
	csrrd	a0, offset;	\
	bl	hexserial64;	\
	PRINTSTR("\r\n");


#define CONFIG_CACHE_64K_4WAY 1
##define USEPCI

#define msize		s2

/*
 * Register usage:
 *
 * s0 link versus load offset, used to relocate absolute adresses.
 * s1 free
 * s2 memory size.
 * s3 st.dShape.
 * s4 Bonito base address.
 * s5 dbg.
 * s6 st.dCfg.
 * s7 rasave.
 * s8 L3 Cache size.
 */


	.globl	_start
	.globl	start
	.globl	__main
_start:
start:
	.globl	stack
stack = start + LOCK_CACHE_SIZE	 /* Place PMON stack in the end of 2M RAM */

    /*set all spi cs to 1, default input*/
    li.d t0, PHYS_TO_UNCACHED(0x1fff0220)
    li.w t1, 0xff 
    st.b t1, t0, 0x5

#ifdef BOOT_SPI_FREQ
    /* spi speedup */
    li.w  t1, (BOOT_SPI_FREQ<<4) | 0x7
#else
    li.w t1, 0x17
#endif
    st.b  t1, t0, 0x4


	/* enable perf counter as cp0 counter */
	li.w	t0, (0x1 << 16)
	csrxchg t0, t0, 0x200

	/* open auto flush SFB */
	li.w	t0, (0x1 << 9)
	csrxchg t0, t0, 0x80

	li.d	t0, UNCACHED_MEMORY_ADDR | 0xf
	csrwr	t0, 0x180
	li.d	t0, CACHED_MEMORY_ADDR | 0x1f
	csrwr	t0, 0x181



	li.d	t0, PHYS_TO_UNCACHED(0xfe00001000)
	li.d	t1, PHYS_TO_UNCACHED(0x1fe20000)
	st.w	t1,	t0,	0x10 	/* config bar for APB */
	ld.w	t2,	t0,	0x04
	ori	t2,	t2,	0x2
	st.w	t2,	t0,	0x04
	bl watchdog_close
	nop

//#define SHUT_SLAVES
#ifdef	SHUT_SLAVES
	li.d	a0, PHYS_TO_UNCACHED(0x1fe004d4)
	ld.w	t0, a0, 0
	li.w	a1, ~(1 << 1)
	and	t0, t0, a1
	st.w	t0, a0, 0
#endif
/*
 * should before execution jr shutdown slave core
 * otherwise speculative execution cause error
 */
	/* clear Vint cofigure */
	li.d	t0, (0x7 << 16)
	csrxchg zero, t0, 0x4
	/* set ebase address */
	li.d	t0, PHYS_TO_CACHED(0x1c001000)
	csrwr	t0, 0xc
	/* set TLB excption address */
	li.d	t0, 0x000000001c001000
	csrwr	t0, 0x88

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
	bl	1f
1:
	addi.d	t0, ra, 12
	or	t0, t1, t0
	jirl	zero, t0, 0
	/* now pc run to 0x90xxxxxxxxxxxxxx */

	/* DA disable for 0x90xxxxxxxxxxxxxx and 0x80xxxxxxxxxxxx address can be used */
	li.w	t0, 0xb0
	csrwr	t0, 0x0

	/* calculate ASM stage print function s0 address */
	la	s0, start
	li.d	a0, PHYS_TO_UNCACHED(0x1c000000)

	/* if change locked cache address may need change the following code */
	sub.d	s0, s0, a0
#	li.d	a0, 0x00000000ffff0000
#	and	s0, s0, a0

	la	sp, stack
	la	gp, _gp

	/* slave core run to slave_main */
	csrrd   t0, 0x20
	andi    t0, t0, 0x3ff
	li.d    a0, CACHED_MEMORY_ADDR
	andi    t1, t0, 0x3             /* core id */
	slli.d  t2, t1, 18
	or      a0, t2, a0              /* 256KB offset for the each core */
	andi    t2, t0, 0xc             /* node id */
	slli.d  t2, t2, 42
	or      a0, t2, a0              /* get the L2 cache address */

	slli.d  t1, t1, 8
	or      t1, t2, t1

	li.d    t2, NODE0_CORE0_BUF0
	or      t1, t2, t1

	li.d    t3, RESERVED_COREMASK
	andi    t3, t3, 0xf
	li.d    t1, 0x1
	sll.w   t1, t1, t0
	and     t3, t1, t3
	bnez    t3, wait_to_be_killed

	li.d    t2, BOOTCORE_ID
	bne     t0, t2, slave_main

	li.d    v0, PHYS_TO_UNCACHED(0x1fe004d4)
	ld.w    t2, v0, 0
	xori    t2, t2, SHUTDOWN_MASK
	st.w    t2, v0, 0

	b       1f

wait_to_be_killed:
	b	wait_to_be_killed
	nop
1:
	li.d	a0, COM1_BASE_ADDR
	bl	initserial

#ifdef	SHUT_SLAVES
	PRINTSTR("Shut down slave cores done!\r\n")
#else
	PRINTSTR("NOT Shut down slave cores\r\n")
#endif

//	li.d	a0, COM1_BASE_ADDR
//	bl	initserial

bsp_start:
	PRINTSTR("\r\nPMON2020 LA2K1000 Initializing. Standby...\r\n")
	dbar 0
	ibar 0
	nop

1:

	bl	locate			/* Get current execute address */
	nop

	/* this code start address is 0x500 */
//#include "resume.S"

	/* all exception entry */
	.org 0x1000

	/* s0 in different stage should fixup */
	la	a0, start
	li.d	a1, PHYS_TO_UNCACHED(0x1c000000)
	sub.d	a0, a0, a1
	li.d	a1, 0x00000000ffff0000
	and	a0, a0, a1
	beq	a0, s0, 1f
	or	s0, zero, zero
1:
	and	s0, s0, a0
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
	bl	1f
1:
	addi.d	t0, ra, 12
	or	t0, t1, t0
	jirl	zero, t0, 0

	li.d	t0, 0xb0
	csrwr	t0, 0
	PRINTSTR("\r\nTLB exception!\r\n");
	PRINT_CSR(0x89);
	PRINT_CSR(0x8a);
1:
	b	1b

locate:
#ifdef HS0636
	bl	HS0636_beep_on
#else
	bl	beep_on
#endif
#ifdef  GPIO_RESET
    bl  gpio_reset_start
#endif

#ifdef HS0636
	bl	HS0636_beep_off
#else
	li.d	a0, 0x1000000
1:
	addi.d	a0, a0, -1
	bnez	a0, 1b
	bl	beep_off
#endif
#ifdef  GPIO_RESET
    bl  gpio_reset_end
#endif

	li.d	t0, PHYS_TO_UNCACHED(0xfe00001000)
	li.d	t1, PHYS_TO_UNCACHED(0x1fe20000)
	st.w	t1,	t0,	0x10 	/* config bar for APB */
	ld.w	t2,	t0,	0x04
	ori	t2,	t2,	0x2
	st.w	t2,	t0,	0x04

	/* ACPI Power Button Status clear */
	li.d	t0, PHYS_TO_UNCACHED(0x1fe27000)
	ld.w	t1, t0, 0xc
	ori	t1, t1, 0x100
	st.w	t1, t0, 0xc


	//pcie signal test copy
	li.d	t0, PHYS_TO_UNCACHED(0x1fe00000)

	li.w	t1, 0xc2492331
	st.w	t1, t0, 0x580
	st.w	t1, t0, 0x5a0

	li.w	t1, 0xff3ff0a8
	st.w	t1, t0, 0x584
	st.w	t1, t0, 0x5a4

	li.w	t1, 0x27fff
	st.w	t1, t0, 0x588
	st.w	t1, t0, 0x5a8


/* cfg pcie copy*/
	li.d	a0, 0x4fff1002
	bl	ls2k_pcie_phy_write
	li.d	a0, 0x4fff1102
	bl	ls2k_pcie_phy_write
	li.d	a0, 0x4fff1202
	bl	ls2k_pcie_phy_write
	li.d	a0, 0x4fff1302
	bl	ls2k_pcie_phy_write

	li.d	t0, PHYS_TO_UNCACHED(0x1fe00430)
	ld.w	t1, t0, 0
	li.w	t2, 0x30000
	or	t1, t1, t2	//pcie enable
	st.w	t1, t0, 0

/* pcie 0 port 0 */
	li.d	a0, 9
	bl	ls2k_pcie0_port_conf
/* pcie 0 port 1 */
	li.d	a0, 10
	bl	ls2k_pcie0_port_conf
/* pcie 0 port 2 */
	li.d	a0, 11
	bl	ls2k_pcie0_port_conf
/* pcie 0 port 3 */
	li.d	a0, 12
	bl	ls2k_pcie0_port_conf
/* pcie 1 port 0 */
	li.d	a0, 13
	bl	ls2k_pcie1_port_conf
/* pcie 1 port 1 */
	li.d	a0, 14
	bl	ls2k_pcie1_port_conf

	PRINTSTR("\r\ninitserial good ^_^...\r\n")


#include "la_2k1000_clksetting.S"


	/* Config SATA : use internel clock */
	li.d	t0, PHYS_TO_UNCACHED(0x1fe00000)
    	li.w      t1, 0x30c31cf9
    	st.w    t1, t0, 0x454
#ifdef SATA_USE_EXTERNAL_CLK
    	li.w      t1, 0xf300040f
#else
    	li.w      t1, 0xf300040d
#endif
    	st.w    t1, t0, 0x450
#if 1 /* Config SATA TX signal*/
    li.d     t1, 0x1403f1002
    st.d      t1, t0, 0x458
#endif


	PRINTSTR("\r\nUSE internel SATA ref clock\r\n")
	li.d	t1, PHYS_TO_UNCACHED(0x1fe00450)

	ld.d	a0, t1, 0
	bl	hexserial64
	nop

 	// Fix the Gmac0  multi-func to enable Gmac1
	li.d	t0, PHYS_TO_UNCACHED(0x1fe03800)
	li.d	a0, 0xffffff0000ffffff
	st.d	a0, t0, 0x08

	li.d	t0, PHYS_TO_UNCACHED(0xfe00001800)
	li.w	a0, 0x0080ff08
	st.w	a0, t0, 0x0c

	// Set the invalid BAR to read only
	li.d	t0, PHYS_TO_UNCACHED(0x1fe03800)
	li.d	a0, 0xff00ff0000fffff0
	st.d	a0, t0, 0x00
	st.d	a0, t0, 0x08
	st.d	a0, t0, 0x10
	st.d	a0, t0, 0x18
	st.d	a0, t0, 0x20
	st.d	a0, t0, 0x28
	st.d	a0, t0, 0x30
	st.d	a0, t0, 0x38
	st.d	a0, t0, 0x40
	st.d	a0, t0, 0x48
	st.d	a0, t0, 0x50

	li.d	t0, PHYS_TO_UNCACHED(0x1fe00430)
	ld.w 	a2, t0, 0
	// enable pcie0 and pcie1, dvo0 and dvo1 pin output
	li.w	t1, 0x30012
	or	a2, a2, t1
   	st.w    a2, t0, 0


	li.d	t0, PHYS_TO_UNCACHED(0x1fe00420)
	ld.w	a2, t0, 0
	//enable sdio,pwm0, i2c0, i2c1, nand, sata, i2s, gmac1
	//no hda, no ac97
#ifdef CONFIG_REG0
    li.w    t1, CONFIG_REG0
#else
	li.w	t1, 0x001f49
#endif
	or	a2, a2, t1
	st.w	a2, t0, 0


start_now:

	bnez	s0, 1f

	li.w	a0, 128
	la	ra, init_loongarch
	jirl	zero, ra, 0
1:

#include "ddr_entry.S"

	PRINTSTR("\r\nlock scache ")
	li.d	a0, LOCK_CACHE_BASE
	bl	hexserial
	PRINTSTR(" - ")
	li.d	a0, LOCK_CACHE_BASE + LOCK_CACHE_SIZE
	bl	hexserial

	li.d	t0, PHYS_TO_UNCACHED(0x1fe00200)
	li.d	t1, ~(LOCK_CACHE_SIZE - 1)
	st.d	t1, t0, 0x40
	li.d	t1, (LOCK_CACHE_BASE & 0xffffffffffff) | (1 << 63)
	st.d	t1, t0, 0x0

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
	li.d	t0, PHYS_TO_CACHED(0x1c00000c)
	li.d	t1, 0xfffff
	bl	1f
1:
	and	t1, ra, t1
	add.d	ra, t1, t0
	jirl	zero, ra, 0


#ifdef SHUT_SLAVES
	PRINTSTR("Wake up other cores\r\n")
	li.d    t0, PHYS_TO_UNCACHED(0x1fe004d4)
	ld.w	t1, t0, 0
	ori	t1, t1, 0x2
	st.w	t1, t0, 0
#endif


	PRINTSTR("run in cache.\r\n")

	or	s0, zero, zero
	move    a0, msize
	la	ra, cache_main
	jirl	zero, ra, 0




LEAF(clear_mailbox)
	csrrd   t0, 0x20

	andi    t0, t0, 0x3ff
	andi    t1, t0, 0x3
	slli.d  t1, t1, 0x8
	andi    t2, t0, 0xc
	slli.d  t2, t2, 42
	or      t1, t2, t1
	li.d     t2, NODE0_CORE0_BUF0
	or      t1, t1, t2
	st.d    zero, t1, FN_OFF
	st.d    zero, t1, SP_OFF
	st.d    zero, t1, GP_OFF
	st.d    zero, t1, A1_OFF

	jirl zero, ra, 0x0
END(clear_mailbox)

.global slave_main
slave_main:
	/* a0, ap own cpu number
	 * t2, bsp mail box address
	 */
	bl      clear_mailbox

	/* t1 store mailbox base so don't changing it */
waitforinit:
	li.w      t0, 0x2

idle1000:
	addi.w  t0, t0, -1
	bnez    t0, idle1000
	/*csr finally filled the low 32 bits*/
	ld.w    t0, t1, FN_OFF
	beqz    t0, waitforinit

	ld.d    t0, t1, FN_OFF

	or      ra, t0,zero

	li.d     t3, CACHED_MEMORY_ADDR

	ld.d    t0, t1, SP_OFF
	or      t0, t0, t3
	or      sp, t0, zero

	ld.d    t0, t1, GP_OFF
	or      t0, t0, t3
	or      gp, t0, zero

	# slave core jump to kernel, byebye
	jirl    zero, ra, 0x0
	//end    slave_main

LEAF(get_cpuprid)
	csrrd	a0, 0xc0
	jirl	zero, ra, 0
END(get_cpuprid)

/*
 * Simple character printing routine used before full initialization
 */
/* baud rate definitions, matching include/termios.h */
#define B0	0
#define B50	50
#define B75	75
#define B110	110
#define B134	134
#define B150	150
#define B200	200
#define B300	300
#define B600	600
#define B1200	1200
#define B1800	1800
#define B2400	2400
#define B4800	4800
#define B9600	9600
#define B19200	19200
#define B38400	38400
#define B57600	57600
#define B115200	115200

#define UART_REF_CLK	25000000
#define UART_DIV_HI	(((UART_REF_CLK + (115200*8)) / (115200*16)) >> 8)
#define UART_DIV_LO	(((UART_REF_CLK + (115200*8)) / (115200*16)) & 0xff)
/*************************************
 *used: a0~a1
 *************************************/
LEAF(initserial)
	li.d	a0, COM1_BASE_ADDR
	li.w	a1, 0x80
	st.b	a1, a0, 3

//	li.w	a1, UART_DIV_HI
	li.w	a1, 0
	st.b	a1, a0, 1
//	li.w	a1, UART_DIV_LO
	li.w	a1, 0x36
	st.b	a1, a0, 0

	li.w	a1, 3	#CFCR_8BITS
	st.b	a1, a0, 3

	li.w	a1, 71
	st.b	a1, a0, 2
	jirl	zero, ra, 0
END(initserial)

#define UART_REF_CLK_PLL	(GMAC_FREQ * GMAC_DIV / SB_DIV * 1000000)	//SB PLL
#define UART_DIV_HI_2		(((UART_REF_CLK_PLL + (115200*8)) / (115200*16)) >> 8)
#define UART_DIV_LO_2		(((UART_REF_CLK_PLL + (115200*8)) / (115200*16)) & 0xff)
LEAF(initserial_later)
	li.d	a0, COM1_BASE_ADDR
	li.w	a1, 0x80
	st.b	a1, a0, 3

	li.w	a1, 0
	st.b	a1, a0, 1
//	li.w	a1, 0xd
	li.w	a1, 68
	st.b	a1, a0, 0
1:
	ld.b	a0, a0, 0
	bne	a1, a0, 1b

	li.d	a0, COM1_BASE_ADDR
	li.w	a1, 3	#CFCR_8BITS
	st.b	a1, a0, 3

	li.w	a1, 71
	st.b	a1, a0, 2
	jirl	zero, ra, 0
END(initserial_later)

/******************************************************
 *used: a0~a2
 ******************************************************/
LEAF(tgt_putchar)
	li.d	a1, COM1_BASE_ADDR
1:
	ld.bu	a2, a1, 0x5
	andi	a2, a2, 0x20
	beqz	a2, 1b

	st.b	a0, a1, 0
	//	or	a2, zero, a1

	jirl	zero, ra, 0
END(tgt_putchar)

/******************************************************
 *used: a0~a4, s0
 ******************************************************/
LEAF(stringserial)
	or	a4, ra, zero
	sub.d	a3, a0, s0
	ld.bu	a0, a3, 0
1:
	beqz	a0, 2f

	bl	tgt_putchar

	addi.d	a3, a3, 1
	ld.bu	a0, a3, 0
	b	1b

2:
	ori	ra, a4, 0
	jirl	zero, ra, 0
END(stringserial)

/*****************************************************
 *used: a0~a5, s0
 *****************************************************/
LEAF(hexserial)
	ori	a4, ra, 0
	ori	a3, a0, 0
	li.d	a5, 8
1:
	rotri.w a0, a3, 28
	or	a3, a0, zero
	andi	a0, a0, 0xf

	la	a1, hexchar
	sub.d	a1, a1, s0

	add.d	a1, a1, a0
	ld.bu	a0, a1, 0

	bl	tgt_putchar

	addi.d	a5, a5, -1
	bnez	a5, 1b

	ori	ra, a4, 0
	jirl	zero, ra, 0
END(hexserial)

/*****************************************************
 *used: a0~a5, s0
 *****************************************************/
LEAF(hexserial64)
	ori	a4, ra, 0
	ori	a3, a0, 0
	li.d	a5, 16
1:
	rotri.d a0, a3, 60
	or	a3, a0, zero
	andi	a0, a0, 0xf

	la	a1, hexchar
	sub.d	a1, a1, s0

	add.d	a1, a1, a0
	ld.bu	a0, a1, 0

	bl	tgt_putchar

	addi.d	a5, a5, -1
	bnez	a5, 1b

	ori	ra, a4, 0
	jirl	zero, ra, 0
END(hexserial64)

	.section .rodata
hexchar:
	.ascii	"0123456789abcdef"
	.text
	.align 5

LEAF(tgt_testchar)
	li.d	a0, COM1_BASE_ADDR
	ld.bu	a1, a0, NSREG(NS16550_LSR)
	andi	a0, a1, LSR_RXRDY
	jirl	zero, ra, 0
END(tgt_testchar)

LEAF(tgt_getchar)
	li.d	a0, COM1_BASE_ADDR
1:
	ld.bu	a1, a0, NSREG(NS16550_LSR)
	andi	a1, a1, LSR_RXRDY
	beqz	a1, 1b
	ld.b	a0, a0, NSREG(NS16550_DATA)
	jirl	zero, ra, 0
END(tgt_getchar)

/*
 * a0 [0:15] phy configure address, [16:31] phy configure data
 */
LEAF(ls2k_pcie_phy_write)
	li.d	a1, 0x100000000
	or	a0, a1, a0
	li.d	a1, PHYS_TO_UNCACHED(0x1fe00590)
	st.d	a0, a1, 0x0
	st.d	a0, a1, 0x20
1:
	ld.w	a0, a1, 0x4
	andi	a0, a0, (1 << 2)
	beqz	a0, 1b
	jirl	zero, ra, 0
END(ls2k_pcie_phy_write)

/*
 * a0 is device number
 */
LEAF(ls2k_pcie0_port_conf)
#if defined(FORCE_PCIE_GEN1) && FORCE_PCIE_GEN1 & 0xf
    li.w    t0, 0x1
    andi    t1, a0, 0x7
    sll.w   t0, t0, t1
#endif
    addi.w  t1, a0, -9
	slli.w	a0, a0, 11
	li.d	a1, PHYS_TO_UNCACHED(0xfe0800000c)
	or	a1, a1, a0

	li.w	a2, 0xfff9ffff
	ld.w	a3, a1, 0x0
	and	a3, a3, a2
	li.w	a2, 0x20000
	or	a3, a3, a2
	st.w	a3, a1, 0x0

	li.d	a1, PHYS_TO_UNCACHED(0xfe0700001c)
	or	a1, a1, a0
	ld.w	a3, a1, 0x0
	li.w	a2, (0x1 << 26)
	or	a3, a3, a2
	st.w	a3, a1, 0x0

	li.d	a1, PHYS_TO_UNCACHED(0xfe00000000)
	or	a1, a1, a0
	ld.w	a3, a1, 0x78
	li.w	a2, ~(0x7 << 12)
	and	a3, a3, a2
	li.w	a2, 0x1000
	or	a3, a3, a2
	st.w	a3, a1, 0x78
#if defined(FORCE_PCIE_GEN1) && FORCE_PCIE_GEN1 & 0xf
    andi    a3, t0, FORCE_PCIE_GEN1
    beqz    a3, 1f
    ld.w    a2, a1, 0xa0
    or      a2, a2, 3
    xor     a2, a2, 2
    st.w    a2, a1, 0xa0
1:
#endif
    li.w	a2, 0x11000000
	st.w	a2, a1, 0x10

	li.d	a1, PHYS_TO_UNCACHED(0x11000000)

	li.w	a2, ~((0x7 << 18) | (0x7 << 2))
	ld.w	a3, a1, 0x54
	and	a3, a3, a2
	st.w	a3, a1, 0x54

	ld.w	a3, a1, 0x58
	and	a3, a3, a2
	st.w	a3, a1, 0x58
#ifdef PCIE_LANE_FLIP
    li.w    a3, (PCIE_LANE_FLIP)
#else
    li.w    a3, 0
#endif
    slli.w  t1, t1, 0x1
    srl.w   a3, a3, t1
    andi    a3, a3, 0x3
    li.w    a2, 0xff204c
	or	    a2, a2, a3
	st.w	a2, a1, 0x0

	jirl	zero, ra, 0
END(ls2k_pcie0_port_conf)


LEAF(ls2k_pcie1_port_conf)
#if defined(FORCE_PCIE_GEN1) && FORCE_PCIE_GEN1 & 0x30
    li.w    t0, 0x1
    and     t1, a0, 0x7
    sll.w   t0, t0, t1
#endif
    addi.w  t1, a0, -9
	slli.w	a0, a0, 11
	li.d	a1, PHYS_TO_UNCACHED(0xfe0800000c)
	or	a1, a1, a0

	li.w	a2, 0xfff9ffff
	ld.w	a3, a1, 0x0
	and	a3, a3, a2
	li.w	a2, 0x20000
	or	a3, a3, a2
	st.w	a3, a1, 0x0

	li.d	a1, PHYS_TO_UNCACHED(0xfe0700001c)
	or	a1, a1, a0
	ld.w	a3, a1, 0x0
	li.w	a2, (0x1 << 26)
	or	a3, a3, a2
	st.w	a3, a1, 0x0

	li.d	a1, PHYS_TO_UNCACHED(0xfe00000000)
	or	a1, a1, a0
	ld.w	a3, a1, 0x78
	li.w	a2, ~(0x7 << 12)
	and	a3, a3, a2
	li.w	a2, 0x1000
	or	a3, a3, a2
	st.w	a3, a1, 0x78
#if defined(FORCE_PCIE_GEN1) && FORCE_PCIE_GEN1 & 0x30
    andi    a3, t0, FORCE_PCIE_GEN1
    beqz    a3, 1f
    ld.w    a2, a1, 0xa0
    or      a2, a2, 3
    xor     a2, a2, 2
    st.w    a2, a1, 0xa0
1:
#endif
	li.w	a2, 0x10000000
	st.w	a2, a1, 0x10

	li.d	a1, PHYS_TO_UNCACHED(0x10000000)

	li.w	a2, ~((0x7 << 18) | (0x7 << 2))
	ld.w	a3, a1, 0x54
	and	a3, a3, a2
	st.w	a3, a1, 0x54

	ld.w	a3, a1, 0x58
	and	a3, a3, a2
	st.w	a3, a1, 0x58

#ifdef PCIE_LANE_FLIP
    li.w    a3, (PCIE_LANE_FLIP)
#else
    li.w    a3, 0
#endif
    slli.w  t1, t1, 0x1
    srl.w   a3, a3, t1
    andi    a3, a3, 0x3
    li.w    a2, 0xff204c
	or	    a2, a2, a3
	st.w	a2, a1, 0x0

	jirl	zero, ra, 0
END(ls2k_pcie1_port_conf)

#ifndef BEEP_GPIO
#define BEEP_GPIO 39
#endif
#ifdef HS0636
LEAF(HS0636_beep_on)
//enable output
	li.d	t1, PHYS_TO_UNCACHED(0x1fe00500)

	li.w	t2, (1<<(BEEP_GPIO&31))
#if BEEP_GPIO > 31
	ld.w	t0, t1, 4
#else
    ld.w	t0, t1, 0
#endif            
	or	    t0, t0, t2
	xor	    t0, t0, t2
#if BEEP_GPIO > 31
	st.w    t0, t1, 4
#else
	st.w    t0, t1, 0
#endif

	li.d	t1, PHYS_TO_UNCACHED(0x1fe00510)
	li.w	t3, 0x80
222:
	li.w	t2, (1<<(BEEP_GPIO&31))
#if BEEP_GPIO > 31
	ld.w	t0, t1, 4
#else
    ld.w	t0, t1, 0
#endif            
	or	    t0, t0, t2
	xor	    t0, t0, t2
#if BEEP_GPIO > 31
	st.w    t0, t1, 4
#else
	st.w    t0, t1, 0
#endif

	li.w	t4, 0x2000
1:
	addi.w	t4, t4, -1
	nop
	bnez	t4, 1b

	li.w	t2, (1<<(BEEP_GPIO&31))
#if BEEP_GPIO > 31
	ld.w	t0, t1, 4
#else
    ld.w	t0, t1, 0
#endif            
	or	    t0, t0, t2
#if BEEP_GPIO > 31
	st.w    t0, t1, 4
#else
	st.w    t0, t1, 0
#endif


	li.w	t4, 0x2000
1:
	addi.w	t4, t4, -1
	nop
	bnez	t4, 1b

	addi.w	t3, t3, -1
	nop
	bnez	t3, 222b

	jirl	zero, ra, 0
	nop
END(HS0636_beep_on)
LEAF(HS0636_beep_off)
	/* set gpio35 low */
	li.d	t1, PHYS_TO_UNCACHED(0x1fe00500)
	li.w	t2, (1<<(BEEP_GPIO&31))
#if BEEP_GPIO > 31
	ld.w	t0, t1, 0x14
#else
    ld.w	t0, t1, 0x10
#endif            
	or	    t0, t0, t2
	xor	    t0, t0, t2
#if BEEP_GPIO > 31
	st.w    t0, t1, 0x14
#else
	st.w    t0, t1, 0x10
#endif

	jirl	zero, ra, 0
	nop
END(HS0636_beep_off)
#endif
LEAF(beep_on)
	/* enable gpio35 output */
	li.d	t1, PHYS_TO_UNCACHED(0x1fe00500)
#if BEEP_GPIO > 31
	ld.w	t0, t1, 4
#else
    ld.w	t0, t1, 0
#endif            
	li.w	t2, (1<<(BEEP_GPIO&31))
	or	    t0, t0, t2
	xor	    t0, t0, t2
#if BEEP_GPIO > 31
	st.w    t0, t1, 4
#else
	st.w    t0, t1, 0
#endif
	/* set gpio35 high */
#if BEEP_GPIO > 31
	ld.w	t0, t1, 0x14
#else
    ld.w	t0, t1, 0x10
#endif            
	or	    t0, t0, t2
#if BEEP_GPIO > 31
	st.w    t0, t1, 0x14
#else
	st.w    t0, t1, 0x10
#endif

	jirl	zero, ra, 0
	nop
END(beep_on)

LEAF(beep_off)
	/* enable gpio35 input */
	li.d	t1, PHYS_TO_UNCACHED(0x1fe00500)
#if BEEP_GPIO > 31
	ld.w	t0, t1, 4
#else
    ld.w	t0, t1, 0
#endif            
	li.w	t2, (1<<(BEEP_GPIO&31))
	or	    t0, t0, t2
#if BEEP_GPIO > 31
	st.w    t0, t1, 4
#else
	st.w    t0, t1, 0
#endif
	/* set gpio35 low */
#if BEEP_GPIO > 31
	ld.w	t0, t1, 0x14
#else
    ld.w	t0, t1, 0x10
#endif            
	or	    t0, t0, t2
	xor	    t0, t0, t2
#if BEEP_GPIO > 31
	st.w    t0, t1, 0x14
#else
	st.w    t0, t1, 0x10
#endif

	jirl	zero, ra, 0
	nop
END(beep_off)
#ifdef GPIO_RESET
LEAF(gpio_reset_start)
	/* enable gpio35 output */
	li.d	t1, PHYS_TO_UNCACHED(0x1fe00500)
#if GPIO_RESET > 31
	ld.w	t0, t1, 4
#else
    ld.w	t0, t1, 0
#endif            
	li.w	t2, (1<<(GPIO_RESET&31))
	or	    t0, t0, t2
	xor	    t0, t0, t2
#if GPIO_RESET > 31
	st.w    t0, t1, 4
#else
	st.w    t0, t1, 0
#endif
	/* set gpio35 high */
#if GPIO_RESET > 31
	ld.w	t0, t1, 0x14
#else
    ld.w	t0, t1, 0x10
#endif            
	or	    t0, t0, t2
	xor	    t0, t0, t2
#if GPIO_RESET > 31
	st.w    t0, t1, 0x14
#else
	st.w    t0, t1, 0x10
#endif

	jirl	zero, ra, 0
	nop
END(gpio_reset_start)

LEAF(gpio_reset_end)
	/* enable gpio35 input */
	li.d	t1, PHYS_TO_UNCACHED(0x1fe00500)
#if GPIO_RESET > 31
	ld.w	t0, t1, 4
#else
    ld.w	t0, t1, 0
#endif            
	li.w	t2, (1<<(GPIO_RESET&31))
	or	    t0, t0, t2
	xor	    t0, t0, t2
#if GPIO_RESET > 31
	st.w    t0, t1, 4
#else
	st.w    t0, t1, 0
#endif
	/* set gpio35 low */
#if GPIO_RESET > 31
	ld.w	t0, t1, 0x14
#else
    ld.w	t0, t1, 0x10
#endif            
	or	    t0, t0, t2
#if GPIO_RESET > 31
	st.w    t0, t1, 0x14
#else
	st.w    t0, t1, 0x10
#endif

	jirl	zero, ra, 0
	nop
END(gpio_reset_end)
#endif



LEAF(watchdog_open)
	//disable watch DOG.
#if 1	//use internal watch dog
	li.d	t1, PHYS_TO_UNCACHED(0x1ff6c000)
	li.w	t2, 0x2fffffff
	st.w	t2, t1, 0x38

	ld.w	t2, t1, 0x30
	li.w	t3, 0x2
	or	t2, t2, t3
	st.w	t2, t1, 0x30	//enable watchdog

	li.w	t2, 0x1
	st.w	t2, t1, 0x34	//set watchdog time
#else	//GPIO0:WDI, GPIO1=0:close, GPIO1=1:1~3s
	/* multi cfg */
	li.d	t1, LS2K1000_GPIO_MULTI_CFG
	ld.w	t2, t1, 0x0
	li.w	t3, ~0xff
	and	t2, t2, t3
	st.w	t2, t1, 0x0

	/*gpio1 output high*/
	li.d	t1, LS2K1000_GPIO_00_63_DIR
	ld.w	t2, t1, 0x0
	li.w	t3, ~0x2
	and	t2, t2, t3
	st.w	t2, t1, 0x0

	ld.w	t2, t1, 0x10
	ori	t2, t2, 2
	st.w	t2, t1, 0x10
	nop
#endif
	jirl	zero, ra, 0
END(watchdog_open)

LEAF(watchdog_close)
	//disable watch DOG.
	/*gpio 3 output zero*/
	li.d	t1, PHYS_TO_UNCACHED(0x1fe00500)
	li.w	t2, ~0x8
	ld.w 	t3, t1, 0
	and 	t2, t2, t3
	st.w 	t2, t1, 0

	li.w	t2, ~0x8
	ld.w 	t3, t1, 0x10
	and 	t2, t2, t3
	st.w 	t2, t1, 0x10

	jirl	zero, ra, 0
END(watchdog_close)

    .text
    .global  nvram_offs
    .align 12
nvram_offs:
    .dword 0x0
    .align 12
#######################################


    .section .rdata
    .global ddr2_reg_data
    .align  5
#include "loongson_mc2_param.S"
