/*	$Id: exception.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2000-2002 Opsycon AB
 * Copyright (c) 2002 Patrik Lindergren (www.lindergren.com)
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
 *	This product includes software developed by Opsycon AB.
 *	This product includes software developed by Patrik Lindergren.
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
#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

#include <signal.h>
#include <machine/cpu.h>
#include <machine/frame.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <pmon.h>
#include <debugger.h>
#include "mod_debugger.h"

#include "../arch/loongarch/loongarchregs.h"

extern void _go __P((void));

/*
 *  For each CPU this table have a pointer to its cpuinfo area.
 */
struct trapframe *cpuinfotab[8];
#if NMOD_DEBUGGER > 0
/*
 *  exception()
 *      An exception has been generated. Need to sort out from where.
 *      frame is a pointer to cpu data saved on the PMON2000 stack.
 */
void
exception(frame)
	struct trapframe *frame;
{
	/*now pmon do not support interrupt so run to while(1) */
	unsigned long long val;

	val = read_64bit_csr(0x8a);
	if (val & 0x1) {
		/* tlb refill exception set CRMD to DA mode rewrite it*/
		write_64bit_csr(0x0, 0xb0);
		printf("\nTLB refill epc  0x%llx\n", val & (~0x3ULL));
		printf("TLB refill badv 0x%llx\n", read_64bit_csr(0x89));
		while(1);
	}

	printf("\nLoongArch frame address 0x%llx\n",frame);
	printf("csr_crmd   0x%llx\n",  frame->csrcrmd);
	printf("csr_prmd   0x%llx\n",  frame->csrprmd);
	printf("csr_exctl  0x%llx\n",  frame->csrectl);
	printf("csr_exstat 0x%llx\n",  frame->csrestat);
	printf("csr_epc    0x%llx\n",  frame->csrepc);
	printf("csr_badv   0x%llx\n",  frame->csrbadv);
	printf("csr_badi   0x%llx\n",  frame->csrbadi);
	while(1);
}
#else
void
exception(frame)
	struct trapframe *frame;
{
}
#endif /* NMOD_DEBUGGER */
