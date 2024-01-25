#define SPI_VBIOS_OFFSET	0x1000
#define VBIOS_SIZE		0x1E000 //120KB
extern unsigned long long memorysize_high_n[];
unsigned char readspi_result[VBIOS_SIZE] = {0};
extern char  Vbios[];
extern unsigned char *ls7a_spi_read_vgabios(unsigned char *buf, unsigned int len);
#if defined(BOOT_PARAMS_BPI)
/***************************************************************
 * The following is the implementation of target bpi bootparam *
 ***************************************************************/
#include "../../../pmon/common/smbios/smbios.h"
#include "../../../pmon/cmds/bootparam.h"
#include "target/ls7a.h"
#ifdef VBIOS_TAB

u64 init_vbios(struct boot_params *bp, u64 offset)
{
	struct vbios_info *vbios_data = (struct vbios_info *)((u64)bp + offset);
	char data[8] = {'V', 'B', 'I', 'O', 'S'};

	memcpy(&vbios_data->header.sign, data, sizeof(data));
	vbios_data->header.rev = 0;
	vbios_data->header.len = sizeof (*vbios_data);

	ls7a_spi_read_vgabios(readspi_result, VBIOS_SIZE);
	if (!ls7a_vgabios_crc_check(readspi_result, VBIOS_SIZE)) {
		vbios_data->VbiosAddr = readspi_result;
	} else {
		vbios_data->VbiosAddr = Vbios;
		memcpy(HT1_MEM_BASE_ADDR | ((readl((HT1_CONF_BASE | 0x3000) + 0x18) & ~0xf) + 0x10000000 - 0x100000), Vbios, VBIOS_SIZE); // base-----vbios-base+256M
		printf("vbios addr = 0x%llx\n", HT1_MEM_BASE_ADDR | ((readl((HT1_CONF_BASE | 0x3000) + 0x18) & ~0xf) + 0x10000000 - 0x100000));
	}

	addlist(bp, &vbios_data->header);
	return sizeof(struct vbios_info);
}
#endif
#ifdef MEM_TAB
void map_entry_init(struct mem_info *mem_data, int type, u64 start, u64 size)
{
	mem_data->map[mem_data->map_num].type = type;
	mem_data->map[mem_data->map_num].start = start;
	mem_data->map[mem_data->map_num].size = size;
	mem_data->map_num++;
}

u64 init_mem(struct boot_params *bp, u64 offset)
{
	struct mem_info *mem_data = (struct mem_info *)((u64)bp + offset);
	char data[8] = {'M', 'E', 'M'};
	u64 j = 0;

	memcpy(&mem_data->header.sign, data, sizeof(data));
	mem_data->header.rev = 0;
	mem_data->header.len = sizeof (*mem_data);
	mem_data->map_num = 0;
	/*
	 * 1. The lowest 2M region cannot record in MemMap, cause Linux ram region should begin with usable ram.
	 *    map_entry_init(mem_data, MEM_RESERVED, 0x0, 0x200000);  // 0x0-0x200000
	 */

	/* 2. Available SYSTEM_RAM area. */
	map_entry_init(mem_data, SYSTEM_RAM, 0x200000, 0xf000000 - 0x200000);  // 0x200000~0xf000000

	/* 3. Reserved low memory highest 16M. */
	map_entry_init(mem_data, MEM_RESERVED, 0xf000000, 0x1000000);  // 0xf000000~0x10000000

	/* 4. Available SYSTEM_RAM area */
	map_entry_init(mem_data, SYSTEM_RAM, HIGH_MEM_WIN_BASE_ADDR + 0x10000000, memorysize_high_n[0] - 0x10000000); // (HIGH_MEM_WIN_BASE_ADDR + 0x10000000) ~ MAX

	for (j = 1; j < TOT_NODE_NUM; j++) {
		if (memorysize_high_n[j])
			map_entry_init(mem_data, SYSTEM_RAM, HIGH_MEM_WIN_BASE_ADDR | (j << 44), memorysize_high_n[j]);
	}
	addlist(bp, &mem_data->header);
#if 0
	for (j = 0; j < mem_data->map_num; j++) {
		printf("%d: type: %x, start: %llx, size %llx\n", j, mem_data->map[j].type, mem_data->map[j].start, mem_data->map[j].size);
	}

#endif
	return sizeof(struct mem_info);
}
#endif

#elif defined(BOOT_PARAMS_EFI)
/***************************************************************
 * The following is the implementation of target efi bootparam *
 ***************************************************************/
struct loongson_params  g_lp = { 0 };
struct efi_memory_map_loongson g_map = { 0 };
struct efi_cpuinfo_loongson g_cpuinfo_loongson = { 0 };
struct system_loongson g_sysitem = { 0 };
struct irq_source_routing_table g_irq_source = { 0 };
struct interface_info g_interface = { 0 };
struct interface_info g_board = { 0 };
struct loongson_special_attribute g_special = { 0 };

extern struct pci_device *pcie_dev;

u64 init_vga_bios(void)
{
	if (!pcie_dev) {
		ls7a_spi_read_vgabios(readspi_result, VBIOS_SIZE);
		if (!ls7a_vgabios_crc_check(readspi_result, VBIOS_SIZE))
			return (u64)readspi_result;
		else
			return (u64)Vbios;
	} else {
		return 0ULL;
	}
}

extern struct efi_memory_map_loongson g_map;

#include "../../../pmon/common/smbios/smbios.h"
#include "../../../pmon/cmds/bootparam.h"

struct efi_memory_map_loongson * init_memory_map()
{
	struct efi_memory_map_loongson *emap = &g_map;
	int i = 0;
	int j = 0;

#define EMAP_ENTRY(entry, node, type, start, size) \
	emap->map[(entry)].node_id = (node), \
	emap->map[(entry)].mem_type = (type), \
	emap->map[(entry)].mem_start = (start), \
	emap->map[(entry)].mem_size = (size), \
	(entry)++

#if 1
	EMAP_ENTRY(i, 0, SYSTEM_RAM_LOW, 0x00200000, 0x0ee - DVFS_RAM_SIZE);
	/* for entry with mem_size < 1M, we set bit31 to 1 to indicate
	 * that the unit in mem_size is Byte not MBype */
	EMAP_ENTRY(i, 0, SMBIOS_TABLE, (SMBIOS_PHYSICAL_ADDRESS & 0x0fffffff),
			(SMBIOS_SIZE_LIMIT | 0x80000000));
	/* 0x20000000 size 512M */
	//EMAP_ENTRY(i, 0, VUMA_VIDEO_RAM, 0x20000000, 0x200);
	/* SYSTEM_RAM_HIGH high 512M  */
	//EMAP_ENTRY(i, 0, UMA_VIDEO_RAM, HIGH_MEM_WIN_BASE_ADDR + 0x10000000ULL + ((unsigned long long)(size - 0x20000000)), 0x200);

	EMAP_ENTRY(i, 0, SYSTEM_RAM_HIGH, HIGH_MEM_WIN_BASE_ADDR + 0x10000000, (memorysize_high_n[0] - 0x10000000 - 0x20000000) >> 20);
#endif

	for (j = 1; j < TOT_NODE_NUM; j++) {
		if (memorysize_high_n[j]) {
			EMAP_ENTRY(i, j, SYSTEM_RAM_HIGH, HIGH_MEM_WIN_BASE_ADDR, (memorysize_high_n[j] - 0x20000000) >> 20);
		}
	}

	emap->vers = 1;
	emap->nr_map = i;
	return emap;
#undef	EMAP_ENTRY
}

extern struct interface_info g_board;
struct board_devices *board_devices_info()
{
	struct board_devices *bd = &g_board;
#if (TOT_NODE_NUM >= 8)
	char total_nodes = '0' + (TOT_NODE_NUM / 4);
	strcpy(bd->name, "Loongson-3C5000L-7A-Dev-");
	strncat(bd->name, &total_nodes, 1);
	strcat(bd->name, "way-ATX_EVB");
	bd->num_resources = 10;
#else
	char total_nodes = '0' + TOT_NODE_NUM;
	strcpy(bd->name, "Loongson-3A5000-7A-Dev-");
	strncat(bd->name, &total_nodes, 1);
	strcat(bd->name, "way-ATX_EVB");
	bd->num_resources = 10;
#endif
	return bd;
}

int probe_cputype(unsigned int processor_id)
{
	switch (processor_id & 0xff00) {
	case 0x4200:
		return CPU_LOONGSON32;
	case 0x6100:
	case 0x6400:
	case 0xc000:
	default: /* Default to 64 bit */
		return CPU_LOONGSON64;
	}
}

struct efi_cpuinfo_loongson *init_cpu_info()
{
	struct efi_cpuinfo_loongson *c = &g_cpuinfo_loongson;
	unsigned int available_core_mask = 0;
	unsigned int available = 0;

	c->vers = 2;
	sprintf(c->cpuname, "%s", md_cpuname()); /*cpu name*/
	c->processor_id =  md_cputype();
	c->cputype = probe_cputype(c->processor_id);

	c->cpu_clock_freq = atoi(getenv("cpuclock"));

	c->total_node = TOT_NODE_NUM;
	c->nr_cpus = 4 * TOT_NODE_NUM;

#ifdef	BOOTCORE_ID
	c->cpu_startup_core_id = BOOTCORE_ID;
	c->reserved_cores_mask = RESERVED_COREMASK;
#else
	c->cpu_startup_core_id = 0;
	c->reserved_cores_mask = 0;
#endif
	return c;
}

struct system_loongson *init_system_loongson()
{
	struct system_loongson *s = &g_sysitem;
	s->ccnuma_smp = 1;
	s->vers = 2;
	s->ccnuma_smp = TOT_NODE_NUM / 4 - 1;
	s->sing_double_channel = TOT_NODE_NUM / 4;

	s->nr_uarts = 1;
	s->uarts[0].iotype = 2; //UPIO_MEM
#ifdef BONITO_100M
	s->uarts[0].uartclk = 100000000; //100M clk
#else
	s->uarts[0].uartclk = 25000000; //25M clk
#endif
	s->uarts[0].int_offset = 2; //56 + 2
	s->uarts[0].uart_base = 0x1fe001e0; //uart 0

	return s;
}

enum loongson_irq_source_enum
{
	HT, I8259, UNKNOWN
};

struct irq_source_routing_table *init_irq_source()
{

	struct irq_source_routing_table *irq_info = &g_irq_source ;


	irq_info->PIC_type = HT;


	irq_info->ht_int_bit = 1 << 24;
	irq_info->ht_enable = 0x0000d17b;

	irq_info->node_id = 0;

	irq_info->pci_io_start_addr = 0x00000efdfc000000;

	irq_info->pci_mem_start_addr = 0x44000000ul;
	irq_info->pci_mem_end_addr = 0x7ffffffful;
	irq_info->dma_mask_bits= 64;
	return irq_info;
}

struct interface_info *init_interface_info()
{
	struct interface_info *inter = &g_interface;
	int flashsize;

	tgt_flashinfo((void *)PHYS_TO_UNCACHED(0x1c000000), &flashsize);

	inter->vers = SPEC_VERS;
	inter->size = flashsize / 0x400;
	inter->flag = 1;

	strcpy(inter->description, "Loongson-PMON-V4.0");

	return inter;
}

struct loongson_special_attribute *init_special_info()
{
	struct loongson_special_attribute *special = &g_special;
	char update[11];

	get_update(update);

	strcpy(special->special_name, update);
#ifdef CONFIG_GFXUMA
	special->resource[0].flags = 1;
	special->resource[0].start = 0;
	special->resource[0].end = VRAM_SIZE;
	strcpy(special->resource[0].name, "UMAMODULE");
#else
	special->resource[0].flags = 0;
	special->resource[0].start = 0;
	special->resource[0].end = VRAM_SIZE;
	strcpy(special->resource[0].name, "SPMODULE");
#endif
	special->resource[0].flags |= DMA64_SUPPORT;
	return special;
}

u64 init_vbios(struct boot_params *bp, u64 offset)
{
	if (!pcie_dev) {
		ls7a_spi_read_vgabios(readspi_result, VBIOS_SIZE);
		if (!ls7a_vgabios_crc_check(readspi_result, VBIOS_SIZE))
			smbios->vga_bios = readspi_result;
		else
			smbios->vga_bios = Vbios;
	} else {
		smbios->vga_bios = 0;
	}
}
#endif
