/*
 * Copyright (C) 2000  Maciej W. Rozycki
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#ifndef _ASM_DIV64_H
#define _ASM_DIV64_H
#include <linux/bitops.h>
#if BITS_PER_LONG == 64
# define do_div(n,base) ({					\
	uint32_t __base = (base);				\
	uint32_t __rem;						\
	__rem = ((uint64_t)(n)) % __base;			\
	(n) = ((uint64_t)(n)) / __base;				\
	__rem;							\
 })
#else
/*
 * No traps on overflows for any of these...
 */

/*  note: at($1) -> a0($r4) 
 *  use .set push, .set pop ??  
 */

#define do_div64_32(res, high, low, base) ({ \
        unsigned long __quot, __mod; \
        unsigned long __cf, __tmp, __tmp2, __i; \
        \
        __asm__(" \
                "or	%2, $r0, 0\n\t" \
                "or	%3, $r0, 0\n\t" \
                "b      1f\n\t" \
                "li.d    %4, 0x21\n" \
                "0:\n\t" \
                "slli.w $r4, %0, 0x1\n\t" \             
                "srli.w %3, %0, 0x1f\n\t" \
                "or     %0, $r4, %5\n\t" \
                "slli.w %1, %1, 0x1\n\t" \
                "slli.w %2, %2, 0x1\n" \
                "1:\n\t" \
                "bnez   %3, 2f\n\t" \
                "sltu   %5, %0, %z6\n\t" \
                "bnez   %5, 3f\n\t" \
                "2:\n\t" \
                "addi.d %4,%4,-1\n\t" \
                "sub.d  %0, %0, %z6\n\t" \
                "addi.d %2, %2, 1\n" \
                "3:\n\t" \
                "bnez   %4, 0b\n\t" \
                "srli.w %5, %1, 0x1f\n\t" \
                ".set   pop" \
                : "=&r" (__mod), "=&r" (__tmp), "=&r" (__quot), "=&r" (__cf), \
                  "=&r" (__i), "=&r" (__tmp2) \
                : "Jr" (base), "0" (high), "1" (low)); \
        \
        (res) = __quot; \
        __mod; })
//#else
//#define do_div64_32(res, high, low, base)
//#endif                                

#define do_div(n, base) ({ \
        unsigned long long __quot; \
        unsigned long __upper, __low, __high, __mod; \
        \
        __quot = (n); \
        __high = __quot >> 32; \
        __low = __quot; \
        __upper = __high; \
        \
        if (__high) \
        __asm__( \
                "div.wu  %z1,%z2,%z3              \n"     \
                "mod.wu  %z0,%z2,%z3              \n"\
                : "=r" (__upper), "=r" (__high) \
                : "Jr" (__high), "Jr" (base));  \
        \
        __mod = do_div64_32(__low, __upper, __low, base); \
        \
        __quot = __high; \
        __quot = __quot << 32 | __low; \
        (n) = __quot; \
        __mod; })
#endif
#endif /* _ASM_DIV64_H */
