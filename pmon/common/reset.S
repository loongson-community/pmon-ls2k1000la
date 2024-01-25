/*
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Copyright (C) 2007 Lemote, Inc. & Institute of Computing Technology
 * Author: Fuxin Zhang, zhangfx@lemote.com
 */

/* the following code is exported to kernel, we need to be sure that there are
 * safe to be called by 64bit code, so we use the reserved the area 0x8ffc0000
 * as the new stack */

/* this is leaf function exported by PMON to kernel */
#include <regdef.h>
#include "cpu.h"
	.text
	.global poweroff_kernel
poweroff_kernel:
	csrrd	t0, 0x0
	ori	t0, t0, (1 << 2)
	xori	t0, t0, (1 << 2)
	csrwr	t0, 0x0
	li.d	sp, PHYS_TO_CACHED(0xffc0000)
	la	t0, tgt_poweroff
	jirl	zero, t0, 0
1:	b	1b
	//end	poweroff_kernel

	.global reboot_kernel
reboot_kernel:
	csrrd	t0, 0x0
	ori	t0, t0, (1 << 2)
	xori	t0, t0, (1 << 2)
	csrwr	t0, 0x0
	li.d	sp, PHYS_TO_CACHED(0xffc0000)
	la	t0, tgt_reboot
	jirl	zero, t0, 0
1:	b	1b
	//end	reboot_kernel
