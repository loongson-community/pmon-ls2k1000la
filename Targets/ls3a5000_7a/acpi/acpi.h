/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Core ACPI (Advanced Configuration and Power Interface) support
 *
 * Copyright 2019 Google LLC
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

#ifndef __ACPI_H__
#define __ACPI_H__

/* Length of an ACPI name string, excluding null terminator */
#define ACPI_NAME_LEN	4

/* Length of an ACPI name string including nul terminator */
#define ACPI_NAME_MAX	(ACPI_NAME_LEN + 1)

#if !defined(__ACPI__)

/**
 * struct acpi_ctx - Context used for writing ACPI tables
 *
 * This contains a few useful pieces of information used when writing
 *
 * @base: Base address of ACPI tables
 * @current: Current address for writing
 * @rsdp: Pointer to the Root System Description Pointer, typically used when
 *	adding a new table. The RSDP holds pointers to the RSDT and XSDT.
 * @rsdt: Pointer to the Root System Description Table
 * @xsdt: Pointer to the Extended System Description Table
 * @slit: Pointer to the System Locality Information Table
 * @nhlt: Intel Non-High-Definition-Audio Link Table (NHLT) pointer, used to
 *	build up information that audio codecs need to provide in the NHLT ACPI
 *	table
 * @len_stack: Stack of 'length' words to fix up later
 * @ltop: Points to current top of stack (0 = empty)
 */
struct acpi_ctx {
	void *base;
	void *current;
	struct acpi_rsdp *rsdp;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt;
};

#include <stdlib.h>
#include "cpu.h"

/* These constants must agree with the memory
 * map as the address and the ACPI pulls the
 * ACPI table entry point from in the acpi_init subroutine.
 */
#define ACPI_TABLE_PHYSICAL_ADDRESS PHYS_TO_CACHED(0xfefe000)
#define ACPI_TABLE_SIZE_LIMIT 0x100000

#define OFFSET_OF(type, field) ((u32) &(((type *)0)->field))

//
// Fixed ACPI Description Table Fixed Feature Flags
// All other bits are reserved and must be set to 0.
//
#define EFI_ACPI_2_0_WBINVD         (1 << 0)
#define EFI_ACPI_2_0_WBINVD_FLUSH   (1 << 1)
#define EFI_ACPI_2_0_PROC_C1        (1 << 2)
#define EFI_ACPI_2_0_P_LVL2_UP      (1 << 3)
#define EFI_ACPI_2_0_PWR_BUTTON     (1 << 4)
#define EFI_ACPI_2_0_SLP_BUTTON     (1 << 5)
#define EFI_ACPI_2_0_FIX_RTC        (1 << 6)
#define EFI_ACPI_2_0_RTC_S4         (1 << 7)
#define EFI_ACPI_2_0_TMR_VAL_EXT    (1 << 8)
#define EFI_ACPI_2_0_DCK_CAP        (1 << 9)
#define EFI_ACPI_2_0_RESET_REG_SUP  (1 << 10)
#define EFI_ACPI_2_0_SEALED_CASE    (1 << 11)
#define EFI_ACPI_2_0_HEADLESS       (1 << 12)
#define EFI_ACPI_2_0_CPU_SW_SLP     (1 << 13)

void loongson_acpi_init(void);

#endif /* __ACPI__ */

#endif
