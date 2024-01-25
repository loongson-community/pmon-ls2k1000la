/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Helpers for ACPI table generation
 *
 * Copyright 2019 Google LLC
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

#ifndef __ACPI_TABLE_H__
#define __ACPI_TABLE_H__

#include <linux/libata.h>
#include <acpi/acpi.h>
#include "cpu.h"

#define SIGNATURE_16(A, B)        ((A) | (B << 8))
#define SIGNATURE_32(A, B, C, D)  (SIGNATURE_16 (A, B) | \
		(SIGNATURE_16 (C, D) << 16))
#define SIGNATURE_64(A, B, C, D, E, F, G, H) \
    (SIGNATURE_32 (A, B, C, D) | \
	 ((u64) (SIGNATURE_32 (E, F, G, H)) << 32))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define RSDP_SIG				"RSD PTR "
#define OEM_ID					"LOONGS"
#define OEM_TABLE_ID			"LOONGSON"
#define ASLC_ID					"LIUX"

#define EFI_ACPI_CREATOR_REVISION 0x01000013


/* TODO(sjg@chromium.org): Figure out how to get compiler revision */
#define ASL_REVISION					0
#define EFI_ACPI_2_0_SYSTEM_MEMORY		0

#define ACPI_RSDP_REV_ACPI_1_0			0
#define ACPI_RSDP_REV_ACPI_2_0			2

#if !defined(__ACPI__)

struct acpi_ctx;

/*
 * RSDP (Root System Description Pointer)
 * Note: ACPI 1.0 didn't have length, xsdt_address, and ext_checksum
 */
struct acpi_rsdp {
	char signature[8];	/* RSDP signature */
	u8 checksum;		/* Checksum of the first 20 bytes */
	char oem_id[6];		/* OEM ID */
	u8 revision;		/* 0 for ACPI 1.0, others 2 */
	u32 rsdt_address;	/* Physical address of RSDT (32 bits) */
	u32 length;		/* Total RSDP length (incl. extended part) */
	u64 xsdt_address;	/* Physical address of XSDT (64 bits) */
	u8 ext_checksum;	/* Checksum of the whole table */
	u8 reserved[3];
} __attribute__((packed));

/* Generic ACPI header, provided by (almost) all tables */
struct acpi_table_header {
	char signature[ACPI_NAME_LEN];	/* ACPI signature (4 ASCII chars) */
	u32 length;		/* Table length in bytes (incl. header) */
	u8 revision;		/* Table version (not ACPI version!) */
	u8 checksum;	/* To make sum of entire table == 0 */
	char oem_id[6];		/* OEM identification */
	char oem_table_id[8];	/* OEM table identification */
	u32 oem_revision;	/* OEM revision number */
	char aslc_id[4];	/* ASL compiler vendor ID */
	u32 aslc_revision;	/* ASL compiler revision number */
} __attribute__((packed));

struct acpi_gen_regaddr {
	u8 space_id;    /* Address space ID */
	u8 bit_width;   /* Register size in bits */
	u8 bit_offset;  /* Register bit offset */
	u8 access_size; /* Access size */
	u32 addrl;  /* Register address, low 32 bits */
	u32 addrh;  /* Register address, high 32 bits */
} __attribute__((packed));

/* A maximum number of 32 ACPI tables ought to be enough for now */
#define MAX_ACPI_TABLES		32

/* XSDT (Extended System Description Table) */
struct acpi_xsdt {
	struct acpi_table_header header;
	u64 entry[MAX_ACPI_TABLES];
} __attribute__((packed));

/* SLIT (System Locality Distance Information Table) */
struct acpi_slit {
	struct acpi_table_header header;
	u64 number_of_system_localities;
} __attribute__((packed));

/* FADT Preferred Power Management Profile */
enum acpi_pm_profile {
	ACPI_PM_UNSPECIFIED = 0,
	ACPI_PM_DESKTOP,
	ACPI_PM_MOBILE,
	ACPI_PM_WORKSTATION,
	ACPI_PM_ENTERPRISE_SERVER,
	ACPI_PM_SOHO_SERVER,
	ACPI_PM_APPLIANCE_PC,
	ACPI_PM_PERFORMANCE_SERVER,
	ACPI_PM_TABLET
};

//
// FADT Definitions
//
#define RESERVED        0x00
#define PM_PROFILE      0x01
#define SCI_INT_VECTOR  0x006f//0x6f = 47 + 64
#define PM1_EVT_LEN     0x08
#define PM1_CNT_LEN     0x04
#define PM_TM_LEN       0x04
#define GPE0_BLK_LEN    0x08
#define P_LVL2_LAT      0x0065 // > 0x64, not support C2
#define P_LVL3_LAT      0x03e9 // > 0x3e8, not support C3
#define FLAGS           (EFI_ACPI_2_0_WBINVD | \
		EFI_ACPI_2_0_PROC_C1 | EFI_ACPI_2_0_SLP_BUTTON | \
		EFI_ACPI_2_0_RESET_REG_SUP)

#define ACPI_BASE_ADDR                 0x00000000100d0000

//RST_CNT
#define RESET_REG_ADDRESS_SPACE_ID     EFI_ACPI_2_0_SYSTEM_MEMORY
#define RESET_REG_BIT_WIDTH            0x20
#define RESET_REG_BIT_OFFSET           0x00
#define RESET_REG_ADDRESS              (ACPI_BASE_ADDR + 0x30)
#define RESET_VALUE                    0x01

//PM1_STS & PM1_EN
#define PM1a_EVT_BLK_ADDRESS_SPACE_ID  EFI_ACPI_2_0_SYSTEM_MEMORY
#define PM1a_EVT_BLK_BIT_WIDTH         0x40
#define PM1a_EVT_BLK_BIT_OFFSET        0x00
#define PM1a_EVT_BLK_ADDRESS           (ACPI_BASE_ADDR + 0x0c)

//PM1_CNT
#define PM1a_CNT_BLK_ADDRESS_SPACE_ID  EFI_ACPI_2_0_SYSTEM_MEMORY
#define PM1a_CNT_BLK_BIT_WIDTH         0x20
#define PM1a_CNT_BLK_BIT_OFFSET        0x00
#define PM1a_CNT_BLK_ADDRESS           (ACPI_BASE_ADDR + 0x14)

//PM_TMR
#define PM_TMR_BLK_ADDRESS_SPACE_ID    EFI_ACPI_2_0_SYSTEM_MEMORY
#define PM_TMR_BLK_BIT_WIDTH           0x20
#define PM_TMR_BLK_BIT_OFFSET          0x00
#define PM_TMR_BLK_ADDRESS             (ACPI_BASE_ADDR + 0x18)

//GPE0_STS & GPE0_EN
#define GPE0_BLK_ADDRESS_SPACE_ID      EFI_ACPI_2_0_SYSTEM_MEMORY
#define GPE0_BLK_BIT_WIDTH             0x40
#define GPE0_BLK_BIT_OFFSET            0x00
#define GPE0_BLK_ADDRESS               (ACPI_BASE_ADDR + 0x28)

/**************************************************************************/
/* FADT flags for p_lvl2_lat and p_lvl3_lat */
#define ACPI_FADT_C2_NOT_SUPPORTED	101
#define ACPI_FADT_C3_NOT_SUPPORTED	1001

#define BIT(a) (1 << a)

/* FADT Boot Architecture Flags */
#define ACPI_FADT_LEGACY_FREE		0x00
#define ACPI_FADT_LEGACY_DEVICES	BIT(0)
#define ACPI_FADT_8042			BIT(1)
#define ACPI_FADT_VGA_NOT_PRESENT	BIT(2)
#define ACPI_FADT_MSI_NOT_SUPPORTED	BIT(3)
#define ACPI_FADT_NO_PCIE_ASPM_CONTROL	BIT(4)

/* FADT Feature Flags */
#define ACPI_FADT_WBINVD		BIT(0)
#define ACPI_FADT_WBINVD_FLUSH		BIT(1)
#define ACPI_FADT_C1_SUPPORTED		BIT(2)
#define ACPI_FADT_C2_MP_SUPPORTED	BIT(3)
#define ACPI_FADT_POWER_BUTTON		BIT(4)
#define ACPI_FADT_SLEEP_BUTTON		BIT(5)
#define ACPI_FADT_FIXED_RTC		BIT(6)
#define ACPI_FADT_S4_RTC_WAKE		BIT(7)
#define ACPI_FADT_32BIT_TIMER		BIT(8)
#define ACPI_FADT_DOCKING_SUPPORTED	BIT(9)
#define ACPI_FADT_RESET_REGISTER	BIT(10)
#define ACPI_FADT_SEALED_CASE		BIT(11)
#define ACPI_FADT_HEADLESS		BIT(12)
#define ACPI_FADT_SLEEP_TYPE		BIT(13)
#define ACPI_FADT_PCI_EXPRESS_WAKE	BIT(14)
#define ACPI_FADT_PLATFORM_CLOCK	BIT(15)
#define ACPI_FADT_S4_RTC_VALID		BIT(16)
#define ACPI_FADT_REMOTE_POWER_ON	BIT(17)
#define ACPI_FADT_APIC_CLUSTER		BIT(18)
#define ACPI_FADT_APIC_PHYSICAL		BIT(19)
#define ACPI_FADT_HW_REDUCED_ACPI	BIT(20)
#define ACPI_FADT_LOW_PWR_IDLE_S0	BIT(21)

enum acpi_address_space_type {
	ACPI_ADDRESS_SPACE_MEMORY = 0,	/* System memory */
	ACPI_ADDRESS_SPACE_IO,		/* System I/O */
	ACPI_ADDRESS_SPACE_PCI,		/* PCI config space */
	ACPI_ADDRESS_SPACE_EC,		/* Embedded controller */
	ACPI_ADDRESS_SPACE_SMBUS,	/* SMBus */
	ACPI_ADDRESS_SPACE_PCC = 0x0a,	/* Platform Comm. Channel */
	ACPI_ADDRESS_SPACE_FIXED = 0x7f	/* Functional fixed hardware */
};

enum acpi_address_space_size {
	ACPI_ACCESS_SIZE_UNDEFINED = 0,
	ACPI_ACCESS_SIZE_BYTE_ACCESS,
	ACPI_ACCESS_SIZE_WORD_ACCESS,
	ACPI_ACCESS_SIZE_DWORD_ACCESS,
	ACPI_ACCESS_SIZE_QWORD_ACCESS
};

/* FADT (Fixed ACPI Description Table) */
struct acpi_fadt {
	struct acpi_table_header header;
	u32 firmware_ctrl;
	u32 dsdt;
	u8 res1;
	u8 preferred_pm_profile;
	u16 sci_int;
	u32 smi_cmd;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 s4bios_req;
	u8 pstate_cnt;
	u32 pm1a_evt_blk;
	u32 pm1b_evt_blk;
	u32 pm1a_cnt_blk;
	u32 pm1b_cnt_blk;
	u32 pm2_cnt_blk;
	u32 pm_tmr_blk;
	u32 gpe0_blk;
	u32 gpe1_blk;
	u8 pm1_evt_len;
	u8 pm1_cnt_len;
	u8 pm2_cnt_len;
	u8 pm_tmr_len;
	u8 gpe0_blk_len;
	u8 gpe1_blk_len;
	u8 gpe1_base;
	u8 cst_cnt;
	u16 p_lvl2_lat;
	u16 p_lvl3_lat;
	u16 flush_size;
	u16 flush_stride;
	u8 duty_offset;
	u8 duty_width;
	u8 day_alrm;
	u8 mon_alrm;
	u8 century;
	u16 iapc_boot_arch;
	u8 res2;
	u32 flags;
	struct acpi_gen_regaddr reset_reg;
	u8 reset_value;
	u16 arm_boot_arch;
	u8 minor_revision;
	u64 x_firmware_ctl;
	u64 x_dsdt;
	struct acpi_gen_regaddr x_pm1a_evt_blk;
	struct acpi_gen_regaddr x_pm1b_evt_blk;
	struct acpi_gen_regaddr x_pm1a_cnt_blk;
	struct acpi_gen_regaddr x_pm1b_cnt_blk;
	struct acpi_gen_regaddr x_pm2_cnt_blk;
	struct acpi_gen_regaddr x_pm_tmr_blk;
	struct acpi_gen_regaddr x_gpe0_blk;
	struct acpi_gen_regaddr x_gpe1_blk;
} __attribute__((packed));

/* FADT TABLE Revision values - note these do not match the ACPI revision */
#define ACPI_FADT_REV_ACPI_1_0		1
#define ACPI_FADT_REV_ACPI_2_0		3
#define ACPI_FADT_REV_ACPI_3_0		4
#define ACPI_FADT_REV_ACPI_4_0		4
#define ACPI_FADT_REV_ACPI_5_0		5
#define ACPI_FADT_REV_ACPI_6_0		6

#define LAPIC_DEFAULT_BASE			0x1fe01400
/* MADT TABLE Revision values - note these do not match the ACPI revision */
#define ACPI_MADT_REV_ACPI_2_0		1
#define ACPI_MADT_REV_ACPI_3_0		2
#define ACPI_MADT_REV_ACPI_4_0		3
#define ACPI_MADT_REV_ACPI_5_0		3
#define ACPI_MADT_REV_ACPI_6_0		5

#define ACPI_MCFG_REV_ACPI_3_0		1


/* SLIT TABLE Revision values - note these do not match the ACPI revision */
#define ACPI_SLIT_REV_ACPI_2_0		1
#define ACPI_SLIT_REV_ACPI_3_0		2
#define ACPI_SLIT_REV_ACPI_4_0		3
#define ACPI_SLIT_REV_ACPI_5_0		3
#define ACPI_SLIT_REV_ACPI_6_0		5

/* IVRS Revision Field */
#define IVRS_FORMAT_FIXED	0x01	/* Type 10h & 11h only */
#define IVRS_FORMAT_MIXED	0x02	/* Type 10h, 11h, & 40h */

/* FACS flags */
#define ACPI_FACS_S4BIOS_F		BIT(0)
#define ACPI_FACS_64BIT_WAKE_F		BIT(1)

/* FACS (Firmware ACPI Control Structure) */
struct acpi_facs {
	char signature[ACPI_NAME_LEN];	/* "FACS" */
	u32 length;			/* Length in bytes (>= 64) */
	u32 hardware_signature;		/* Hardware signature */
	u32 firmware_waking_vector;	/* Firmware waking vector */
	u32 global_lock;		/* Global lock */
	u32 flags;			/* FACS flags */
	u64 x_firmware_waking_vector;	/* X FW waking vector */
	u8 version;			/* Version 2 */
	u8 res1[3];
	u32 ospm_flags;			/* OSPM enabled flags */
	u8 res2[24];
} __attribute__((packed));

/* MADT flags */
#define ACPI_MADT_PCAT_COMPAT	BIT(0)

/* MADT (Multiple APIC Description Table) */
struct acpi_madt {
	struct acpi_table_header header;
	u32 lapic_addr;			/* Local APIC address */
	u32 flags;			/* Multiple APIC flags */
} __attribute__((packed));

/* MADT: APIC Structure Type*/
enum acpi_apic_types {
	ACPI_APIC_LAPIC	= 0,		/* Processor local APIC */
	ACPI_APIC_IOAPIC,		/* I/O APIC */
	ACPI_APIC_IRQ_SRC_OVERRIDE,	/* Interrupt source override */
	ACPI_APIC_NMI_SRC,		/* NMI source */
	ACPI_APIC_LAPIC_NMI,		/* Local APIC NMI */
	ACPI_APIC_LAPIC_ADDR_OVERRIDE,	/* Local APIC address override */
	ACPI_APIC_IOSAPIC,		/* I/O SAPIC */
	ACPI_APIC_LSAPIC,		/* Local SAPIC */
	ACPI_APIC_PLATFORM_IRQ_SRC,	/* Platform interrupt sources */
	ACPI_APIC_LX2APIC,		/* Processor local x2APIC */
	ACPI_APIC_LX2APIC_NMI,		/* Local x2APIC NMI */
};

/* MADT: Processor Local APIC Structure */

#define LOCAL_APIC_FLAG_ENABLED		BIT(0)

struct acpi_madt_lapic {
	u8 type;		/* Type (0) */
	u8 length;		/* Length in bytes (8) */
	u8 processor_id;	/* ACPI processor ID */
	u8 apic_id;		/* Local APIC ID */
	u32 flags;		/* Local APIC flags */
} __attribute__((packed));

#define EFI_ACPI_IOIPIC_ID		0x0
#define EFI_ACPI_IOAPIC_ADDR		0x10000000
#define EFI_ACPI_INT_BASE		0x40

#define EFI_ACPI_INT_BASE1		0x80
#define EFI_ACPI_IO_APIC_COUNT	TOT_7A_NUM

/* MADT: I/O APIC Structure */
struct acpi_madt_ioapic {
	u8 type;		/* Type (1) */
	u8 length;		/* Length in bytes (12) */
	u8 ioapic_id;		/* I/O APIC ID */
	u8 reserved;
	u32 ioapic_addr;	/* I/O APIC address */
	u32 gsi_base;		/* Global system interrupt base */
} __attribute__((packed));

/* PM1_CNT bit defines */
#define PM1_CNT_SCI_EN		BIT(0)

/* ACPI global NVS structure */
struct acpi_global_nvs;

/* SRAT (System Resource Affinity Table) */
struct acpi_srat {
	struct acpi_table_header header;
	u32 reserved1;			/* Must be set to 1 */
	u64 reserved2;
} __attribute__((packed));

/* SART Revision */
#define EFI_ACPI_3_0_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION		0x02
#define EFI_ACPI_4_0_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION		0x03
#define EFI_ACPI_5_0_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION		0x03
#define EFI_ACPI_5_1_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION		0x03
#define EFI_ACPI_6_0_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION		0x03
#define EFI_ACPI_6_1_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION		0x03
#define EFI_ACPI_6_2_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION		0x03


//
// ACPI table information used to initialize tables.
//

#define RESERVED8                 0x00
#define RESERVED16                0x0000
#define RESERVED32                0x00000000
#define RESERVED64                0x0000000000000000

#define PROC_TYPE                 EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY
#define PROC_LENGTH               16
#define PROC_FLAGS                EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_ENABLED
#define PROC_SAPIC_EID            0
#define PROC_PROX_DOMAIN_31TO8    0x00


#define MEM_TYPE                  EFI_ACPI_3_0_MEMORY_AFFINITY
#define MEM_LENGTH                40
#define MEM_FLAGS                 EFI_ACPI_3_0_MEMORY_ENABLED
#define MEM_PROX_DOMAIN0          0x00000000
#define MEM_PROX_DOMAIN1          0x00000001
#define MEM_PROX_DOMAIN2          0x00000002
#define MEM_PROX_DOMAIN3          0x00000003

//
// SRAT structure types.
// All other values between 0x02 an 0xFF are reserved and
// will be ignored by OSPM.
//
#define EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY  0x00
#define EFI_ACPI_3_0_MEMORY_AFFINITY                      0x01

///
/// Processor Local APIC/SAPIC Affinity Structure Definition
///
struct acpi_srat_lapic_lsapic_affinity {
	u8   type;
	u8   length;
	u8   proximitydomain7to0;
	u8   apicid;
	u32  flags;
	u8   localsapiceid;
	u8   proximitydomain31to8[3];
	u8   reserved[4];
} __attribute__((packed));

///
/// Local APIC/SAPIC Flags.  All other bits are reserved and must be 0.
///
#define EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC_SAPIC_ENABLED (1 << 0)

///
/// Memory Affinity Structure Definition
///
struct acpi_srat_memory_affinity {
  u8   type;
  u8   length;
  u32  proximitydomain;
  u16  reserved1;
  u32  addressbaselow;
  u32  addressbasehigh;
  u32  lengthlow;
  u32  lengthhigh;
  u32  reserved2;
  u32  flags;
  u64  reserved3;
} __attribute__((packed));

#define EFI_ACPI_OEM_MCFG_REVISION				0x00000001
#define EFI_ACPI_ALLOCATION_STRUCTURE_COUNT			TOT_7A_NUM
#define CONFIG_PCIE_EXT_CON_BASE				0x00000efe00000000

extern char ls7a_link_id_buf[];
#define EFI_ACPI_IOIPIC_ID1					((u64)ls7a_link_id_buf[1])
#define CONFIG_PCIE_EXT_CON_BASE1				CONFIG_PCIE_EXT_CON_BASE | (EFI_ACPI_IOIPIC_ID1 << 44)

/* MCFG (PCI Express MMIO config space BAR description table) */
struct acpi_mcfg {
    struct acpi_table_header header;
    u64 reserved;
} __attribute__((packed));

struct acpi_mcfg_mmconfig {
    u64 base_address;
    u16 pci_segment_group_number;
    u8 start_bus_number;
    u8 end_bus_number;
    u32 reserved;
} __attribute__((packed));
//
// Memory Flags.  All other bits are reserved and must be 0.
//
#define EFI_ACPI_3_0_MEMORY_ENABLED       (1 << 0)
#define EFI_ACPI_3_0_MEMORY_HOT_PLUGGABLE (1 << 1)
#define EFI_ACPI_3_0_MEMORY_NONVOLATILE   (1 << 2)

/* Tables defined/reserved by ACPI and generated by U-Boot */
enum acpi_tables {
	ACPITAB_BERT,
	ACPITAB_DBG2,
	ACPITAB_DMAR,
	ACPITAB_DSDT,
	ACPITAB_ECDT,
	ACPITAB_FACS,
	ACPITAB_FADT,
	ACPITAB_HEST,
	ACPITAB_HPET,
	ACPITAB_IVRS,
	ACPITAB_MADT,
	ACPITAB_MCFG,
	ACPITAB_NHLT,
	ACPITAB_RSDP,
	ACPITAB_RSDT,
	ACPITAB_SLIT,
	ACPITAB_SPCR,
	ACPITAB_SPMI,
	ACPITAB_SRAT,
	ACPITAB_SSDT,
	ACPITAB_TCPA,
	ACPITAB_TPM2,
	ACPITAB_VFCT,
	ACPITAB_XSDT,

	ACPITAB_COUNT,
};

/**
 * acpi_get_table_revision() - Get the revision number generated for a table
 *
 * This keeps the version-number information in one place
 *
 * @table: ACPI table to check
 * @return version number that U-Boot generates
 */
int acpi_get_table_revision(enum acpi_tables table);


/**
 * acpi_fill_header() - Set up a new table header
 *
 * This sets all fields except length, revision, checksum and aslc_revision
 *
 * @header: ACPI header to update
 * @signature: Table signature to use (4 characters)
 */
void acpi_fill_header(struct acpi_table_header *header, char *signature);

/**
 * acpi_align() - Align the ACPI output pointer to a 16-byte boundary
 *
 * @ctx: ACPI context
 */
void acpi_align(struct acpi_ctx *ctx);

/**
 * acpi_align64() - Align the ACPI output pointer to a 64-byte boundary
 *
 * @ctx: ACPI context
 */
void acpi_align64(struct acpi_ctx *ctx);

/**
 * acpi_inc() - Increment the ACPI output pointer by a bit
 *
 * The pointer is NOT aligned afterwards.
 *
 * @ctx: ACPI context
 * @amount: Amount to increment by
 */
void acpi_inc(struct acpi_ctx *ctx, uint amount);

/**
 * acpi_inc_align() - Increment the ACPI output pointer by a bit and align
 *
 * The pointer is aligned afterwards to a 16-byte boundary
 *
 * @ctx: ACPI context
 * @amount: Amount to increment by
 */
void acpi_inc_align(struct acpi_ctx *ctx, uint amount);

/**
 * acpi_add_table() - Add a new table to the RSDP and XSDT
 *
 * @ctx: ACPI context
 * @table: Table to add
 * @return 0 if OK, -E2BIG if too many tables
 */
int acpi_add_table(struct acpi_ctx *ctx, void *table);

/**
 * acpi_setup_base_tables() - Set up context along with RSDP, RSDT and XSDT
 *
 * Set up the context with the given start position. Some basic tables are
 * always needed, so set them up as well.
 *
 * @ctx: Context to set up
 */
void acpi_setup_base_tables(struct acpi_ctx *ctx, void *start);

void table_checksum(char *buff, int size);
#endif /* !__ACPI__*/

#endif /* __ACPI_TABLE_H__ */
