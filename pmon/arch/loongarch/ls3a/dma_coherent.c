#include <sys/types.h>
#include <pmon.h>
#include <stdio.h>
#include <string.h>
#include <machine/frame.h>
#include "cpu.h"
#define CONFIG_PAGE_SIZE_4KB
#include "../../../arch/loongarch/loongarchregs.h"
#include "../../../cmds/bootparam.h"

#define PRID_REV_LOONGSON1B     0x0020
#define PRID_REV_LOONGSON2E     0x0002
#define PRID_REV_LOONGSON2F     0x0003
#define PRID_REV_LOONGSON3A_R1  0x0005
#define PRID_REV_LOONGSON3B_R1  0x0006
#define PRID_REV_LOONGSON3B_R2  0x0007
#define PRID_REV_LOONGSON3A_R2  0x0008
#define PRID_REV_LOONGSON3A_R3_0        0x0009
#define PRID_REV_LOONGSON3A_R3_1        0x000D
#define PRID_REV_LOONGSON2K_R1  0x0001
#define PRID_REV_LOONGSON2K_R2  0x0003

#define HT_uncache_enable_reg0	PHYS_TO_UNCACHED(0xefdfb0000f0)
#define HT_uncache_base_reg0	PHYS_TO_UNCACHED(0xefdfb0000f4)
#define HT_uncache_enable_reg1	PHYS_TO_UNCACHED(0xefdfb0000f8)
#define HT_uncache_base_reg1	PHYS_TO_UNCACHED(0xefdfb0000fc)
#define HT_uncache_enable_reg2	PHYS_TO_UNCACHED(0xefdfb000168)
#define HT_uncache_base_reg2	PHYS_TO_UNCACHED(0xefdfb00016c)
#define HT_uncache_enable_reg3	PHYS_TO_UNCACHED(0xefdfb000170)
#define HT_uncache_base_reg3	PHYS_TO_UNCACHED(0xefdfb000174)
#define HT_cache_enable_reg1	PHYS_TO_UNCACHED(0xefdfb000068)
#define HT_cache_base_reg1	PHYS_TO_UNCACHED(0xefdfb00006c)


void cfg_coherent(int ac, char *av[])
{
	int i, prid;
	uint16_t nocoherent = 0;
#ifdef BOOT_PARAMS_EFI
	struct irq_source_routing_table * irq_tb;
	struct boot_params *bp;

	bp = (struct boot_params *)(cpuinfotab[whatcpu]->a2);

#endif
#ifndef LS7A_2WAY_CONNECT

	prid = read_prid() & 0xf;

	if (prid == PRID_REV_LOONGSON3A_R2 || prid == PRID_REV_LOONGSON3A_R3_0)
		nocoherent = 1;
	for (i = 0; i < ac; i++) {
		if (strstr(av[i], "cached"))
			nocoherent = 0;
		if (strstr(av[i], "uncached"))
			nocoherent = 1;
	}

	printf("set %scoherent\n", nocoherent ? "no":"");
	if (nocoherent) {
		/*	uncache		*/
#ifdef LS7A
		writel(readl(HT_cache_enable_reg1), HT_uncache_enable_reg0); //for 7a gpu
		writel(readl(HT_cache_base_reg1), HT_uncache_base_reg0);
#else
		writel(0xc0000000, HT_uncache_enable_reg0); //Low 256M
		writel(0x0080fff0, HT_uncache_base_reg0);
#endif

		writel(0xc0000000, HT_uncache_enable_reg1); //Node 0
		writel(0x0000e000, HT_uncache_base_reg1);
		writel(0xc0100000, HT_uncache_enable_reg2); //Node 1
		writel(0x2000e000, HT_uncache_base_reg2);
		writel(0xc0200000, HT_uncache_enable_reg3); //Node 2/3
		writel(0x4000c000, HT_uncache_base_reg3);
		writeq(0x0000202000000000, PHYS_TO_UNCACHED(0x1fe02708));
		writeq(0xffffffe000000000, PHYS_TO_UNCACHED(0x1fe02748));
		writeq(0x0000300000000086, PHYS_TO_UNCACHED(0x1fe02788));
	} else
#endif
	{
		/*	cache		*/
		writel(0x0, HT_uncache_enable_reg0);
		writel(0x0, HT_uncache_enable_reg1);
		writel(0x0, HT_uncache_enable_reg2);
		writel(0x0, HT_uncache_enable_reg3);
	}
#ifdef BOOT_PARAMS_EFI
	irq_tb = (struct irq_source_routing_table *)((uint64_t)&(bp->efi.smbios.lp) + bp->efi.smbios.lp.irq_offset);
	irq_tb->dma_noncoherent = nocoherent;
#endif
}
