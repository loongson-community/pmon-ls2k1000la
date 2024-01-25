/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2000 LOONGARCH Technologies, Inc.  All rights reserved.
 * Copyright (C) 2003  Maciej W. Rozycki
 */
#ifndef _ASM_LOONGARCHREGS_H
#define _ASM_LOONGARCHREGS_H


/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 *  Configure language
 */
#ifdef __ASSEMBLY__
#define _ULCAST_
#else
#define _ULCAST_ (unsigned long)
#endif

#ifndef __ASSEMBLY__

/*
 * Functions to access the r10k performance counter and control registers
 */
/*
 * Macros to access the system control coprocessor
 */
#define write_64bit_csr(register, value)			\
do {									\
	__asm__ __volatile__(					\
		"csrwr\t%z0, " #register "\n\t"			\
		: : "Jr" (value));				\
} while (0)
#define write_32bit_csr(register, val)			\
do {									\
		__asm__ __volatile__(					\
		"csrwr\t%z0, " #register "\n\t"			\
		: : "Jr" (value));				\
} while (0)

#define read_64bit_csr(register)				\
({	unsigned long __res;						\
	__asm__ __volatile__(					\
		"csrrd\t%0, " #register "\n\t"			\
		: "=r" (__res));				\
	__res;								\
})

#define read_32bit_csr(register)				\
({ unsigned int __res;								\
	__asm__ __volatile__(					\
		"csrrd\t%0, "#register"\n\t"			\
		: "=r" (__res));				\
	__res;								\
})

#define read_prid()		read_32bit_csr(0xc0)
#define read_apic_id()         read_32bit_csr(0x20)

/*
 * Macros to access the floating point coprocessor control registers
 */
/*
 * TLB operations.
 */

static inline void tlb_probe(void)
{
}

static inline void tlb_read(void)
{
}

static inline void tlb_write_indexed(void)
{
}

static inline void tlb_write_random(void)
{
}

#endif /* !__ASSEMBLY__ */

#endif /* _ASM_LOONGARCHREGS_H */
