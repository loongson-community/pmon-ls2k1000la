// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on acpi from coreboot
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */
#include "acpi_table.h"
#include "sys/param.h"
#include "sys/malloc.h"
#include "../mp/cpump.h"

extern unsigned long long memorysize_high_n[];

#define NODE_COUNT TOT_NODE_NUM

void table_checksum_init(char *buff, int size)
{
	int cksum_off;
	if ((strncmp(buff, RSDP_SIG, sizeof(RSDP_SIG) - 1) == 0) && (size != 20))
		cksum_off = OFFSET_OF(struct acpi_rsdp, ext_checksum);
	else if ((strncmp(buff, RSDP_SIG, sizeof(RSDP_SIG) - 1) == 0) && (size == 20))
		cksum_off = OFFSET_OF(struct acpi_rsdp, checksum);
	else
		cksum_off = OFFSET_OF(struct acpi_table_header, checksum);

	buff[cksum_off] = 0;
	buff[cksum_off] = checksum8(buff, size);

	return;
}

/*
 * IASL compiles the dsdt entries and writes the hex values
 * to a C array AmlCode[] (see dsdt.c).
 */
extern const unsigned char AmlCode[];

void acpi_fill_header(struct acpi_table_header *header, char *signature)
{
	memcpy(header->signature, signature, 4);
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, OEM_TABLE_ID, 8);
	header->oem_revision = 1;
	memcpy(header->aslc_id, ASLC_ID, 4);
	header->aslc_revision = EFI_ACPI_CREATOR_REVISION;
}

void acpi_align(struct acpi_ctx *ctx)
{
	ctx->current = (void *)ACPI_ALIGN((u64)ctx->current, 16);
}

void acpi_align64(struct acpi_ctx *ctx)
{
	ctx->current = (void *)ACPI_ALIGN((u64)ctx->current, 64);
}

void acpi_inc(struct acpi_ctx *ctx, uint amount)
{
	ctx->current += amount;
}

void acpi_inc_align(struct acpi_ctx *ctx, uint amount)
{
	ctx->current += amount;
	acpi_align(ctx);
}

/**
 * Add an ACPI table to the XSDT structure, recalculate length
 * and checksum.
 */
int acpi_add_table(struct acpi_ctx *ctx, void *table)
{
	int i;
	struct acpi_xsdt *xsdt;

	/*
	 * And now add the table info for the XSDT.
	 */
	xsdt = ctx->xsdt;

	for (i = 0; i < MAX_ACPI_TABLES; i++) {
		if (xsdt->entry[i] == 0)
			break;
	}


	/* Add table to the XSDT */
	xsdt->entry[i] = VA_TO_PHYS(table);

	/* Fix XSDT length */
	xsdt->header.length = sizeof(struct acpi_table_header) +
				(sizeof(u64) * (i + 1));

	/* Re-calculate checksum */
	xsdt->header.checksum = 0;
	table_checksum_init((u8 *)xsdt, xsdt->header.length);

	return 0;
}

static void acpi_write_rsdp(struct acpi_rsdp *rsdp, struct acpi_xsdt *xsdt)
{
	memset(rsdp, 0, sizeof(struct acpi_rsdp));

	memcpy(rsdp->signature, RSDP_SIG, 8);
	memcpy(rsdp->oem_id, OEM_ID, 6);

	rsdp->length = sizeof(struct acpi_rsdp);

	rsdp->xsdt_address = (void *)VA_TO_PHYS(xsdt);

	rsdp->revision = ACPI_RSDP_REV_ACPI_2_0;

	/* Calculate checksums */
	table_checksum_init((u8 *)rsdp, 20);
	table_checksum_init((u8 *)rsdp, sizeof(struct acpi_rsdp));
}

static void acpi_write_xsdt(struct acpi_xsdt *xsdt)
{
	struct acpi_table_header *header = &xsdt->header;

	/* Fill out header fields */
	acpi_fill_header(header, "XSDT");
	header->length = sizeof(struct acpi_xsdt);
	header->revision = 1;

	/* Entries are filled in later, we come with an empty set */

	/* Fix checksum */
	table_checksum_init((u8 *)xsdt, sizeof(struct acpi_xsdt));
}

void acpi_setup_base_tables(struct acpi_ctx *ctx, void *start)
{
	ctx->base = start;
	ctx->current = start;

	/* Align ACPI tables to 16 byte */
	acpi_align(ctx);

	/* We need at least an RSDP and an XSDT Table */
	ctx->rsdp = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_rsdp));
	ctx->xsdt = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_xsdt));

	/* clear all table memory */
	memset((void *)start, '\0', ctx->current - start);

	acpi_write_rsdp(ctx->rsdp, ctx->xsdt);

	acpi_write_xsdt(ctx->xsdt);
	/*
	 * Per ACPI spec, the FACS table address must be aligned to a 64 byte
	 * boundary.
	 */
	acpi_align64(ctx);
}

static void acpi_create_facs(struct acpi_facs *facs)
{
	memset((void *)facs, 0, sizeof(struct acpi_facs));

	memcpy(facs->signature, "FACS", 4);
	facs->length = sizeof(struct acpi_facs);
	facs->hardware_signature = 0;
	facs->firmware_waking_vector = 0;
	facs->global_lock = 0;
	facs->flags = 0;
	facs->x_firmware_waking_vector = 0;
	facs->version = 1;
}

void acpi_fadt_common(struct acpi_fadt *fadt, struct acpi_facs *facs,
			  void *dsdt)
{
	struct acpi_table_header *header = &fadt->header;

	memset((void *)fadt, '\0', sizeof(struct acpi_fadt));

	acpi_fill_header(header, "FACP");
	header->length = sizeof(struct acpi_fadt);
	header->revision = 3;
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, OEM_TABLE_ID, 8);
	header->oem_revision = 4;
	memcpy(header->aslc_id, ASLC_ID, 4);

	fadt->x_firmware_ctl	= VA_TO_PHYS(facs);
	fadt->x_dsdt			= VA_TO_PHYS(dsdt);

	fadt->preferred_pm_profile = ACPI_PM_DESKTOP;
}

void fill_fadt(struct acpi_fadt *fadt)
{
	fadt->sci_int = SCI_INT_VECTOR;

	fadt->pm1_evt_len = PM1_EVT_LEN;
	fadt->pm1_cnt_len = PM1_CNT_LEN;
	fadt->pm_tmr_len = PM_TM_LEN;
	fadt->gpe0_blk_len = GPE0_BLK_LEN;

	fadt->p_lvl2_lat = P_LVL2_LAT;
	fadt->p_lvl3_lat = P_LVL3_LAT;

	fadt->flags = FLAGS;

	fadt->reset_reg.space_id = RESET_REG_ADDRESS_SPACE_ID;
	fadt->reset_reg.bit_width = RESET_REG_BIT_WIDTH;
	fadt->reset_reg.addrl = RESET_REG_ADDRESS;

	fadt->reset_value = RESET_VALUE;

	fadt->x_pm1a_evt_blk.space_id = PM1a_EVT_BLK_ADDRESS_SPACE_ID;
	fadt->x_pm1a_evt_blk.bit_width = PM1a_EVT_BLK_BIT_WIDTH;
	fadt->x_pm1a_evt_blk.addrl = PM1a_EVT_BLK_ADDRESS;

	fadt->x_pm1a_cnt_blk.space_id = PM1a_CNT_BLK_ADDRESS_SPACE_ID;
	fadt->x_pm1a_cnt_blk.bit_width = PM1a_CNT_BLK_BIT_WIDTH;
	fadt->x_pm1a_cnt_blk.addrl = PM1a_CNT_BLK_ADDRESS;

	fadt->x_pm_tmr_blk.space_id = PM_TMR_BLK_ADDRESS_SPACE_ID;
	fadt->x_pm_tmr_blk.bit_width = PM_TMR_BLK_BIT_WIDTH;
	fadt->x_pm_tmr_blk.addrl = PM_TMR_BLK_ADDRESS;

	fadt->x_gpe0_blk.space_id = GPE0_BLK_ADDRESS_SPACE_ID;
	fadt->x_gpe0_blk.bit_width = GPE0_BLK_BIT_WIDTH;
	fadt->x_gpe0_blk.addrl = GPE0_BLK_ADDRESS;
}

void acpi_create_fadt(struct acpi_fadt *fadt, struct acpi_facs *facs,
		void *dsdt)
{
	struct acpi_table_header *header = &fadt->header;

	acpi_fadt_common(fadt, facs, dsdt);
	fill_fadt(fadt);
	table_checksum_init((u8 *)fadt, header->length);
}

static int acpi_create_madt_lapic(struct acpi_madt_lapic *lapic, u8 index)
{
	struct processor_info *info = malloc(sizeof(struct processor_info));

	get_processor_info(index, info);

	lapic->type = ACPI_APIC_LAPIC;
	lapic->length = sizeof(struct acpi_madt_lapic);
	lapic->processor_id = index + 1;
	lapic->apic_id = info->p_id;
	if (info->flag & PROCESSOR_ENABLED_BIT) {
		lapic->flags = LOCAL_APIC_FLAG_ENABLED;
	} else {
		lapic->flags = 0;
	}

	free((void *)info);
	return lapic->length;
}

int acpi_create_madt_lapics(u64 current)
{
	int tmp;
	int total_length = 0;
	int cpu_num = 0;
	int processor_id = 0;

	for (tmp = 0; tmp < TOT_NODE_NUM * CORES_PER_NODE; tmp++) {
		int length;
		length = acpi_create_madt_lapic(
			(struct acpi_madt_lapic *)current, tmp);

		current += length;
		total_length += length;
	}

	return total_length;
}

int acpi_create_madt_ioapic(struct acpi_madt_ioapic *ioapic)
{

	ioapic->type = ACPI_APIC_IOAPIC;
	ioapic->length = sizeof(struct acpi_madt_ioapic);
	ioapic->reserved = 0;
	ioapic->gsi_base = EFI_ACPI_INT_BASE;
	ioapic->ioapic_id = EFI_ACPI_IOIPIC_ID;
	ioapic->ioapic_addr = EFI_ACPI_IOAPIC_ADDR;
#if	(TOT_7A_NUM == 2)
	struct acpi_madt_ioapic *ioapic1;
	if(EFI_ACPI_IO_APIC_COUNT > 1) {
		ioapic1 = (struct acpi_madt_ioapic *)(ioapic + 1);
		ioapic1->type = ACPI_APIC_IOAPIC;
		ioapic1->length = sizeof(struct acpi_madt_ioapic);
		ioapic1->reserved = 0;
		ioapic1->gsi_base = EFI_ACPI_INT_BASE1;
		ioapic1->ioapic_id = EFI_ACPI_IOIPIC_ID1;
		ioapic1->ioapic_addr = EFI_ACPI_IOAPIC_ADDR;

	}
#endif

	return EFI_ACPI_IO_APIC_COUNT * ioapic->length;
}

u64 acpi_fill_madt(u64 current)
{
	current += acpi_create_madt_lapics(current);
	current += acpi_create_madt_ioapic((struct acpi_madt_ioapic *)current);

	return current;
}

static void acpi_create_madt(struct acpi_madt *madt)
{
	struct acpi_table_header *header = &(madt->header);
	u64 current = (u64)madt + sizeof(struct acpi_madt);

	memset((void *)madt, 0, sizeof(struct acpi_madt));

	/* Fill out header fields */
	acpi_fill_header(header, "APIC");
	header->length = sizeof(struct acpi_madt);
	header->revision = ACPI_MADT_REV_ACPI_2_0;

	madt->lapic_addr = LAPIC_DEFAULT_BASE;
	madt->flags = 0;

	current = acpi_fill_madt(current);

	/* (Re)calculate length and checksum */
	header->length = current - (u64)madt;

	table_checksum_init((u8 *)madt, header->length);
}

static void acpi_create_srat(struct acpi_srat *srat)
{
	u8 node;
	u8 core;
	u32 srattablesize;
	u64 tablehandle = 0;
	u64 node_memorysize[TOT_NODE_NUM];
	u8 i = 0;

	struct acpi_srat_lapic_lsapic_affinity *procstruct;
	struct acpi_srat_memory_affinity *mem_struct;
	void *tempptr = srat + 1;
	struct acpi_table_header *header = &(srat->header);

	node_memorysize[0] = memorysize_high_n[0] - 0x10000000;
	for (i = 1; i < TOT_NODE_NUM ; i++) {
		node_memorysize[i] = memorysize_high_n[i];
	}

	srattablesize = sizeof(*srat) + TOT_NODE_NUM * CORES_PER_NODE * sizeof(*procstruct);

	srattablesize += sizeof(*mem_struct);	//for node0 low mem
	for (node = 0; node < TOT_NODE_NUM; node++) {
		if(node_memorysize[node]) {
			srattablesize += sizeof(*mem_struct);
		}
	}

	memset((void *)srat, 0, srattablesize);

	acpi_fill_header(header, "SRAT");
	header->length = srattablesize;
	header->revision = EFI_ACPI_3_0_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION;
	header->checksum = 0xd0;
	srat->reserved1 = 1;
	srat->reserved2 = RESERVED64;

	procstruct = tempptr;
	for (node = 0; node < TOT_NODE_NUM; node++) {
		for (core = 0; core < CORES_PER_NODE; core++) {
			procstruct->type = PROC_TYPE;
			procstruct->length = PROC_LENGTH;
			procstruct->proximitydomain7to0 = node;
			procstruct->apicid = node * CORES_PER_NODE + core;
			procstruct->flags = PROC_FLAGS;
			procstruct->localsapiceid = PROC_SAPIC_EID;
			procstruct->proximitydomain31to8[0] = PROC_PROX_DOMAIN_31TO8;
			procstruct->proximitydomain31to8[1] = PROC_PROX_DOMAIN_31TO8;
			procstruct->proximitydomain31to8[2] = PROC_PROX_DOMAIN_31TO8;
			procstruct->reserved[0] = RESERVED8;
			procstruct->reserved[1] = RESERVED8;
			procstruct->reserved[2] = RESERVED8;
			procstruct->reserved[3] = RESERVED8;
			procstruct++;
		}
	}

	tempptr = procstruct;
	mem_struct = tempptr;

	node = 0;
	/* node0 low mem map */
	mem_struct->type = MEM_TYPE;
	mem_struct->length = MEM_LENGTH;
	mem_struct->proximitydomain = node;
	mem_struct->reserved1 = RESERVED16;
	mem_struct->addressbaselow = 0x00000000;
	mem_struct->addressbasehigh = 0x00000000 | (node << 12);
	mem_struct->lengthlow = 0x10000000;
	mem_struct->lengthhigh = 0x00000000;
	mem_struct->reserved2 = RESERVED32;
	mem_struct->flags = MEM_FLAGS;
	mem_struct->reserved3 = RESERVED64;
	mem_struct++;

	mem_struct->type = MEM_TYPE;
	mem_struct->length = MEM_LENGTH;
	mem_struct->proximitydomain = node;
	mem_struct->reserved1 = RESERVED16;
	mem_struct->addressbaselow = (HIGH_MEM_WIN_BASE_ADDR + 0x10000000) & 0xffffffff;
	mem_struct->addressbasehigh = ((HIGH_MEM_WIN_BASE_ADDR + 0x10000000) >> 32) & 0xffffffff | (node << (NODE_OFFSET - 32));
	mem_struct->lengthlow = node_memorysize[node] & 0xffffffff;
	mem_struct->lengthhigh = (node_memorysize[node] >> 32) & 0xffffffff;
	mem_struct->reserved2 = RESERVED32;
	mem_struct->flags = MEM_FLAGS;
	mem_struct->reserved3 = RESERVED64;
	mem_struct++;

	/* hi mem map */
	for (node = 1; node < TOT_NODE_NUM; node++) {
		if (node_memorysize[node]) {
			mem_struct->type = MEM_TYPE;
			mem_struct->length = MEM_LENGTH;
			mem_struct->proximitydomain = node;
			mem_struct->reserved1 = RESERVED16;
			mem_struct->addressbaselow = HIGH_MEM_WIN_BASE_ADDR & 0xffffffff;
			mem_struct->addressbasehigh = (HIGH_MEM_WIN_BASE_ADDR >> 32) & 0xffffffff | (node << (NODE_OFFSET - 32));
			mem_struct->lengthlow = node_memorysize[node] & 0xffffffff;
			mem_struct->lengthhigh = (node_memorysize[node] >> 32) & 0xffffffff;
			mem_struct->reserved2 = RESERVED32;
			mem_struct->flags = MEM_FLAGS;
			mem_struct->reserved3 = RESERVED64;
			mem_struct++;
		}
	}

	tempptr = mem_struct;

	if (((u8 *)tempptr - (u8 *)srat) != srattablesize)
		printf("srat table init error!\n");
	table_checksum_init((u8 *)srat, header->length);
}

u64 acpi_fill_slit(u64 current, struct acpi_slit *slit)
{
	u8 array_entry[NODE_COUNT][NODE_COUNT];
	void *temp = slit + 1;

	if (slit->number_of_system_localities == 1) {
		array_entry[0][0] = 10;
	} else if (slit->number_of_system_localities == 2) {
		array_entry[0][0] = 10;
		array_entry[0][1] = 15;
		array_entry[1][0] = 15;
		array_entry[1][1] = 10;
	} else if (slit->number_of_system_localities == 4) {
		array_entry[0][0] = 10;
		array_entry[0][1] = 15;
		array_entry[0][2] = 20;
		array_entry[0][3] = 18;
		array_entry[1][0] = 15;
		array_entry[1][1] = 10;
		array_entry[1][2] = 16;
		array_entry[1][3] = 24;
		array_entry[2][0] = 20;
		array_entry[2][1] = 16;
		array_entry[2][2] = 10;
		array_entry[2][3] = 12;
		array_entry[3][0] = 18;
		array_entry[3][1] = 24;
		array_entry[3][2] = 12;
		array_entry[3][3] = 10;
	} else {
		//no support
	}
	memcpy(temp, array_entry[0], sizeof(array_entry));
	return current + sizeof(array_entry);
}

static void acpi_create_slit(struct acpi_slit *slit)
{
	struct acpi_table_header *header = &slit->header;
	u64 current = (u64)slit + sizeof(struct acpi_slit);

	memset((void *)slit, 0, sizeof(struct acpi_slit));
	/* Fill out header fields */
	acpi_fill_header(header, "SLIT");
	header->length = sizeof(struct acpi_slit);
	header->revision = ACPI_SLIT_REV_ACPI_2_0;
	slit->number_of_system_localities = NODE_COUNT;

	current = acpi_fill_slit(current, slit);

	/* (Re)calculate length and checksum */
	header->length = current - (u64)slit;

	header->checksum = 0;
	table_checksum_init((u8 *)slit, header->length);
}
static int acpi_create_mcfg_mmconfig(struct acpi_mcfg_mmconfig *mmconfig, u64 base,
		u16 seg_nr, u8 start, u8 end)
{
	memset(mmconfig, 0, sizeof(*mmconfig));
	mmconfig->base_address = base;
	mmconfig->pci_segment_group_number = seg_nr;
	mmconfig->start_bus_number = start;
	mmconfig->end_bus_number = end;

	return sizeof(struct acpi_mcfg_mmconfig);
}

static u64 acpi_fill_mcfg(u64 current)
{
	current += acpi_create_mcfg_mmconfig
		((struct acpi_mcfg_mmconfig *)current,
		 CONFIG_PCIE_EXT_CON_BASE, 0x0, 0x0, 0xff);
	if(EFI_ACPI_ALLOCATION_STRUCTURE_COUNT > 1)
		current += acpi_create_mcfg_mmconfig
			((struct acpi_mcfg_mmconfig *)current,
			CONFIG_PCIE_EXT_CON_BASE1, 0x1, 0x0, 0xff);


	return current;
}

/* MCFG is defined in the PCI Firmware Specification 3.0 */
static void acpi_create_mcfg(struct acpi_mcfg *mcfg)
{
	struct acpi_table_header *header = &(mcfg->header);
	u64 current = (u64)mcfg + sizeof(struct acpi_mcfg);

	memset((void *)mcfg, 0, sizeof(struct acpi_mcfg));

	/* Fill out header fields */
	acpi_fill_header(header, "MCFG");
	header->length = sizeof(struct acpi_mcfg);
	header->revision = EFI_ACPI_OEM_MCFG_REVISION;

	current = acpi_fill_mcfg(current);

	/* (Re)calculate length and checksum */
	header->length = current - (u64)mcfg;
	table_checksum_init((u8 *)mcfg, header->length);
}

u64 write_acpi_tables(void *start)
{
	struct acpi_ctx sctx, *ctx = &sctx;
	struct acpi_facs *facs;
	struct acpi_table_header *dsdt;
	struct acpi_fadt *fadt;
	struct acpi_madt *madt;
	struct acpi_srat *srat;
	struct acpi_slit *slit;
	struct acpi_mcfg *mcfg;
	int ret = 0;

	printf("ACPI: ACPI tables init.\n");

	acpi_setup_base_tables(ctx, start);

	facs = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_facs));

	acpi_create_facs(facs);

	dsdt = ctx->current;

	/* Put the table header first */
	memcpy(dsdt, &AmlCode, sizeof(struct acpi_table_header));
	acpi_inc(ctx, sizeof(struct acpi_table_header));

	/* Copy in the AML code itself if any (after the header) */
	memcpy(ctx->current,
		(char *)&AmlCode + sizeof(struct acpi_table_header),
		dsdt->length - sizeof(struct acpi_table_header));
	acpi_inc(ctx, dsdt->length - sizeof(struct acpi_table_header));
	dsdt->length = ctx->current - (void *)dsdt;
	acpi_align(ctx);

	/*
	 * Recalculate the length and update the DSDT checksum since we patched
	 * the GNVS address. Set the checksum to zero since it is part of the
	 * region being checksummed.
	 */
	dsdt->checksum = 0;
	table_checksum_init((u8 *)dsdt, dsdt->length);

	fadt = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_fadt));
	acpi_create_fadt(fadt, facs, dsdt);
	acpi_add_table(ctx, fadt);

	madt = ctx->current;
	acpi_create_madt(madt);
	acpi_inc_align(ctx, madt->header.length);
	acpi_add_table(ctx, madt);

	srat = ctx->current;
	acpi_create_srat(srat);
	acpi_inc_align(ctx, srat->header.length);
	acpi_add_table(ctx, srat);

	slit = ctx->current;
	acpi_create_slit(slit);
	acpi_inc_align(ctx, slit->header.length);
	acpi_add_table(ctx, slit);

	mcfg = ctx->current;
	acpi_create_mcfg(mcfg);
	acpi_inc_align(ctx, mcfg->header.length);
	acpi_add_table(ctx, mcfg);

	printf("Init acpi table OK!\n");
	return ret;
}
