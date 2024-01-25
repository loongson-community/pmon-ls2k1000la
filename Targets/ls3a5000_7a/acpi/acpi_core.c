// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

#include <libfdt_env.h>
#include "acpi_tables/acpi_table.h"
#include <efi_api.h>

static const efi_guid_t acpi_guid = EFI_ACPI_TABLE_GUID;

static char rsdp_checksum_check(char *buff, int size)
{
	int cksum_val;

	cksum_val = checksum8(buff, size);

	return cksum_val;
}

static struct acpi_rsdp *acpi_valid_rsdp(struct acpi_rsdp *rsdp)
{
	if (strncmp((char *)rsdp, RSDP_SIG, sizeof(RSDP_SIG) - 1) != 0)
		return NULL;
	if (rsdp_checksum_check((char *)rsdp, 20) != 0)
		return NULL;
	if ((rsdp->revision > 1) && (rsdp_checksum_check((char *)rsdp, rsdp->length) != 0))
		return NULL;

	return rsdp;
}

struct acpi_fadt *acpi_find_fadt(void)
{
	char *p, *end;
	struct acpi_rsdp *rsdp = NULL;
	struct acpi_xsdt *xsdt;
	struct acpi_fadt *fadt = NULL;
	int i;

	rsdp = acpi_valid_rsdp((struct acpi_rsdp *)ACPI_TABLE_PHYSICAL_ADDRESS);
	if (!rsdp) {
		printf("RSDP invalid or not fount.\n");
		return NULL;
	}

	xsdt = (struct acpi_xsdt *)PHYS_TO_CACHED(rsdp->xsdt_address);
	end = (char *)xsdt + xsdt->header.length;
	for (i = 0; ((char *)&xsdt->entry[i]) < end; i++) {
		fadt = (struct acpi_fadt *)PHYS_TO_CACHED(xsdt->entry[i]);
		if (strncmp((char *)fadt, "FACP", 4) == 0)
			break;
		fadt = NULL;
	}

	if (!fadt)
		return NULL;

	return fadt;
}

void *acpi_find_wakeup_vector(struct acpi_fadt *fadt)
{
	struct acpi_facs *facs;
	u64 facs_p;
	void *wake_vec;

	facs_p = fadt->x_firmware_ctl;
	facs = (struct acpi_facs *)PHYS_TO_CACHED(facs_p);
	if (!facs) {
		printf("No FACS found, wake up from S3 not possible.\n");
		return NULL;
	}
	wake_vec = (void *)facs->firmware_waking_vector;
	return PHYS_TO_CACHED(wake_vec);
}

void loongson_acpi_init(void)
{
	mp_init();
	write_acpi_tables((void *)ACPI_TABLE_PHYSICAL_ADDRESS);

	/* And expose them to our EFI payload */
	return efi_install_configuration_table(&acpi_guid,
			(void *)VA_TO_PHYS(ACPI_TABLE_PHYSICAL_ADDRESS));
}
