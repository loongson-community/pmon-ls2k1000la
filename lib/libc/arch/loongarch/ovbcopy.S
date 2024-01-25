/*	$Id: ovbcopy.S,v 1.1.1.1 2006/09/14 01:59:06 root Exp $ */

#define	ABICALLS			# NO ABICALLS
#include <asm.h>


/*
 * memcpy(to, from, len)
 * {ov}bcopy(from, to, len)
 */
LEAF(memcpy)
	move	a3, a0			# swap from and to
	move	a0, a1
	move	a1, a3
ALEAF(bcopy)
ALEAF(ovbcopy)
	add.d	t0, a0, a2		# t0 = end of s1 region
	sltu	t1, a1, t0
	sltu	t2, a0, a1
	and	t1, t1, t2		# t1 = true if from < to < (from+len)
	beq	t1, zero, forward	# non overlapping, do forward copy
	slti	t2, a2, 12		# check for small copy

	li.d	a4, 1
	blt	a2, a4, 2f
	add.d	t1, a1, a2		# t1 = end of to region
1:
	ld.b	a4, t0, -1		# copy bytes backwards,
	addi.d	t0, t0, -1		#   doesnt happen often so do slow way
	addi.d	t1, t1, -1
	st.b	a4, t1, 0
	bne	t0, a0, 1b
2:
	move	a0, a3			#recover a0 register
	jirl	zero, ra, 0

forward:
	li.d	a5, 1
	blt	a2, a5, 2f
	add.d	a5, a2, a0		# compute ending address
1:
	ld.bu	a4, a0, 0		# copy bytes
	st.b	a4, a1, 0
	addi.d	a0, a0, 1
	addi.d	a1, a1, 1	# MMU BUG ? can not do -1(a1) at 0x80000000!!
	bne	a0, a5, 1b
2:
	move	a0, a3			#recover a0 register
	jirl	zero, ra, 0
END(memcpy)
