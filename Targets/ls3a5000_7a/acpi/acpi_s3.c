// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

#include <linux/libata.h>
#include "acpi_s3.h"
#include "acpi_tables/acpi_table.h"

void acpi_resume(struct acpi_fadt *fadt)
{
	void *wake_vec;

	wake_vec = acpi_find_wakeup_vector(fadt);
	if (!wake_vec) {
		printf("wake up vec not found!\n");
		return;
	}

	printf("wake entry: 0x%llx. Jump to kenel...\n", (u64)wake_vec);
	//unused sp
	__asm__ __volatile__(
			"or	$r1, %0, $r0\n"
			"jirl	$r0, $r1, 0\n"
			: : "r" (wake_vec));

	while (1)
		;
}
