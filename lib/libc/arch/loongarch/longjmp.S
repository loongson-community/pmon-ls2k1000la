/*	$OpenBSD$ */

/*
 * Copyright (c) 2000 Opsycon AB  (www.opsycon.se)
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

#define	ABICALLS			# NO ABICALLS
#define _KERNEL
#include <machine/asm.h>

/*
 * Amount to take off of the stack for the benefit of the debugger.
 */
#define START_FRAME	((4 * 4) + 4 + 4)

#define	STORE	st.d	/* 64 bit mode regsave instruction */
#define	LOAD	ld.d	/* 64 bit mode regload instruction */
#define	RSIZE	8	/* 64 bit mode register size */

/*  WARNING! If size of jmpbuf changes change go_return_jump as well! */

LEAF(setjmp)
	STORE   s0, a0, RSIZE * 0
	STORE   s1, a0, RSIZE * 1
	STORE   s2, a0, RSIZE * 2
	STORE   s3, a0, RSIZE * 3
	STORE   s4, a0, RSIZE * 4
	STORE   s5, a0, RSIZE * 5
	STORE   s6, a0, RSIZE * 6
	STORE   s7, a0, RSIZE * 7
	STORE   s8, a0, RSIZE * 8
	STORE   sp, a0, RSIZE * 9
	STORE   gp, a0, RSIZE * 10
	STORE   ra, a0, RSIZE * 11
	li.w	v0, 0                    # Setjmp return
	jirl    zero, ra, 0
END(setjmp)

LEAF(longjmp)
	LOAD    ra, a0, RSIZE *11
	LOAD    s0, a0, RSIZE * 0
	LOAD    s1, a0, RSIZE * 1
	LOAD    s2, a0, RSIZE * 2
	LOAD    s3, a0, RSIZE * 3
	LOAD    s4, a0, RSIZE * 4
	LOAD    s5, a0, RSIZE * 5
	LOAD    s6, a0, RSIZE * 6
	LOAD    s7, a0, RSIZE * 7
	LOAD    s8, a0, RSIZE * 8
	LOAD    sp, a0, RSIZE * 9
	LOAD    gp, a0, RSIZE *10
	move    a0, a1
	jirl    zero, ra, 0
END(longjmp)

