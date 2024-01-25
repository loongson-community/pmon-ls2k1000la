// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

/* Ls7a ACPI Register */
#define ACPI_PMCON_RESUME_REG_OFFSET	0x4
#define ACPI_PM1_STS_REG_OFFSET			0xc
#define ACPI_PM1_CNT_REG_OFFSET			0x14
#define ACPI_GPE0_STS_REG_OFFSET		0x28
#define ACPI_GPE0_EN_REG_OFFSET			0x2c
#define ACPI_RST_CNT_REG_OFFSET			0x30
#define ACPI_RTC1_OFFSET				0x50
#define ACPI_RTC2_OFFSET				0x54
#define ACPI_RESET_VAL					0x1
#define ACPI_PM1_STS_CLEAN_VAL			0xffffffff
#define ACPI_SHUT_DOWN_VAL				0x3c00


#define STORE_STR_CODE_EPA				0xbfc00500
#define AcpiRegBase						0xb00d0000
#define STORE_ACPI_REG_BASE_ADDR		(0xaf000000 | 0x200b0)

#define write32(addr, data)		(*(volatile unsigned int*)(addr) = (data))
#define read32(addr)			(*(volatile unsigned int*)(addr))

#define ACPI_REG_WRITE32(Offset, Val) \
	write32((read32(STORE_ACPI_REG_BASE_ADDR) + Offset), Val)



void acpi_base_addr_store(void)
{
	/* Store ACPI controler base addr in memory */
	write32(STORE_ACPI_REG_BASE_ADDR, AcpiRegBase);

	ACPI_REG_WRITE32(ACPI_RTC1_OFFSET, STORE_STR_CODE_EPA);
}
