/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __ASM_ACPI_S3_H__
#define __ASM_ACPI_S3_H__

#include <linux/libata.h>

#define WAKEUP_BASE	0x600

/* PM1_STATUS register */
#define WAK_STS		(1 << 15)
#define PCIEXPWAK_STS	(1 << 14)
#define RTC_STS		(1 << 10)
#define SLPBTN_STS	(1 << 9)
#define PWRBTN_STS	(1 << 8)
#define GBL_STS		(1 << 5)
#define BM_STS		(1 << 4)
#define TMR_STS		(1 << 0)

/* PM1_CNT register */
#define SLP_EN		(1 << 13)
#define SLP_TYP_SHIFT	10
#define SLP_TYP		(7 << SLP_TYP_SHIFT)
#define SLP_TYP_S0	0
#define SLP_TYP_S1	1
#define SLP_TYP_S3	5
#define SLP_TYP_S4	6
#define SLP_TYP_S5	7

/* PM1_STS register */
#define RTC_EN		BIT(10)
#define PWRBTN_EN	BIT(8)

enum acpi_sleep_state {
	ACPI_S0,
	ACPI_S1,
	ACPI_S2,
	ACPI_S3,
	ACPI_S4,
	ACPI_S5,
};

struct acpi_fadt;

struct acpi_fadt *acpi_find_fadt(void);
/**
 * acpi_resume() - Do ACPI S3 resume
 *
 * This calls U-Boot wake up assembly stub and jumps to OS's wake up vector.
 *
 * @fadt:	FADT table pointer in the ACPI table
 * @return:	Never returns
 */
void acpi_resume(struct acpi_fadt *fadt);


#endif /* __ASM_ACPI_S3_H__ */
