#include <stdlib.h>
#include <stdio.h>
#include "bootparam.h"
#include "sys/param.h"
#include "../common/smbios/smbios.h"
#include "csum.h"

#ifndef STR_FUNC_ADDR
#define STR_FUNC_ADDR PHYS_TO_UNCACHED(0x1c000500)
#endif

#if defined(BOOT_PARAMS_BPI)
void bp_checksum(char *buff, int size)
{
	int cksum_off;

	cksum_off = OFFSET_OF(struct ext_list_hdr, checksum);
	buff[cksum_off] = 0;

	buff[cksum_off] = checksum8(buff, size);

	return;
}

void addlist(struct boot_params *bp, struct ext_list_hdr *header)
{
	struct ext_list_hdr *last;

	if (bp->elh == NULL) {
		 bp->elh = header;
	} else {
		for (last = bp->elh; last->next; last = last->next)
			;
		last->next = header;
		bp_checksum((u8 *)last, last->len);
	}
	header->next = NULL;
	bp_checksum((u8 *)header, header->len);
}

int init_boot_param(struct boot_params *bp)
{
	u64 tab_offset = sizeof(struct boot_params);
	char sign_bp[8] = {'B', 'P', 'I', '0', '1', '0', '0', '0'};

	bp->flags = 0;

#ifdef SOC_CPU
	sign_bp[7] = '1';
	bp->flags |= BPI_FLAGS_SOC_CPU;
#ifdef DTB
#include "target/load_dtb.h"
	if ((systab.tables[fdt_table_id].table = setup_dtb(0, NULL)) != 0ULL) {
		const efi_guid_t fdt_guid = EFI_FDT_GUID;
		guidcpy(&systab.tables[fdt_table_id].guid, &fdt_guid);
		systab.nr_tables++;
	}
#endif
#endif
	init_systab();
	memcpy(bp, sign_bp, 8);
	bp->efitab = &systab;
	bp->elh = NULL;

#ifdef MEM_TAB
	tab_offset += init_mem(bp, tab_offset);
#endif
#ifdef VBIOS_TAB
	tab_offset += init_vbios(bp, tab_offset);
#endif
#ifdef SMBIOS_SUPPORT
	loongson_smbios_init();
#endif
#ifdef ACPI_SUPPORT
	loongson_acpi_init();
#endif
	return bp;
}

#elif defined(BOOT_PARAMS_EFI)
extern void poweroff_kernel(void);
extern void reboot_kernel(void);

struct board_devices *board_devices_info();
struct interface_info *init_interface_info();
struct irq_source_routing_table *init_irq_source();
struct system_loongson *init_system_loongson();
struct efi_cpuinfo_loongson *init_cpu_info();
struct efi_memory_map_loongson * init_memory_map();
void init_efi(struct efi_loongson *efi);
struct loongson_special_attribute *init_special_info();


void init_efi(struct efi_loongson *efi)
{
	init_smbios(&(efi->smbios));
}

void init_reset_system(struct efi_reset_system_t *reset)
{
	reset->Shutdown = &poweroff_kernel;
	reset->ResetWarm = &reboot_kernel;
#ifdef LS_STR
	reset->ResetCold = STR_FUNC_ADDR;
#endif
}

void init_smbios(struct smbios_tables *smbios)
{
	smbios->vers = 0;
	smbios->vga_bios = init_vga_bios();
	init_loongson_params(&(smbios->lp));
}

void init_loongson_params(struct loongson_params *lp)
{
	lp->memory_offset = (unsigned long long)init_memory_map() - (unsigned long long)lp;
	lp->cpu_offset = (unsigned long long)init_cpu_info() - (unsigned long long)lp;
	lp->system_offset = (unsigned long long)init_system_loongson() - (unsigned long long)lp;
	lp->irq_offset = (unsigned long long)init_irq_source() - (unsigned long long)lp;
	lp->interface_offset = (unsigned long long)init_interface_info() - (unsigned long long)lp;
	lp->boarddev_table_offset = (unsigned long long)board_devices_info() - (unsigned long long)lp;
	lp->special_offset = (unsigned long long)init_special_info() - (unsigned long long)lp;

	printf("memory_offset = 0x%llx;cpu_offset = 0x%llx; system_offset = 0x%llx; irq_offset = 0x%llx; interface_offset = 0x%llx;\n", lp->memory_offset, lp->cpu_offset, lp->system_offset, lp->irq_offset, lp->interface_offset);
}

int init_boot_param(struct boot_params *bp)
{
	init_efi(&(bp->efi));
	init_reset_system(&(bp->reset_system));
}
#endif
