#include <cpu.h>
#include "target/ls7a_config.h"
#include "ht.h"
#include "gmemparam.h"

#ifdef LS7A2000
#include "ls7a2000.c"
#else
#include "ls7a1000.c"
#endif

#define LS7A_PCIE_CFG_READ(data,reg)                                    \
        readl(0x80000e0000000000 + data + reg)

#define LS7A_PCIE_CLEAR_PORT_IRQ(addr)                                  \
        do {                                                            \
                readl(addr) = 0x60000000;                               \
                if (LS7A_PCIE_CFG_READ(0x60000000,0x18)) {              \
                        LS7A_PCIE_CFG_READ(0x60000000, 0x1c) =          \
                                LS7A_PCIE_CFG_READ(0x60000000, 0x18);   \
                        break;                                          \
                }                                                       \
                readl(addr) = 0x0;                                      \
	} while(0)
ls7a_resource_table_t ls7a_cfg_t;

extern void loop_delay(uint64_t loop);
extern int ls7a_version(void);

void ls7a_dis_ht_clk(uint64_t base)
{
	if (!ls7a_cfg_t.ht.ls7a_2way_connect) {
		/*shut down LS7A HT HI*/
		readl(base + CONF_HT_CLKEN_OFFSET) &= ~(0x1 << 1);
		pr_info("7A HT Hi clock disabled.\r\n");
	}
}

void ls7a_dma_cfg(uint64_t base)
{
	readl(base + CONF_HT_ROUTE_OFFSET) &= ~(0xf | (0xf << 16));
	readl(base + CONF_HT_ROUTE_OFFSET) |= DMA_DEST_HT | (LS3A_NODE_NUM - 0x1);

	if (LS3A_NODE_NUM >= 2) {
		/*open 40 bit DMA device otherwise it lost node id*/
		readl(base + CONF_HT_ROUTE_OFFSET) &= ~(0xff << 8);
		readl(base + CONF_HT_ROUTE_OFFSET) |= (((DMA_NODE_ID_OFFSET - 36) << 8) | ((DMA_NODE_ID_OFFSET - 32) << 13));
	}
	pr_info("config 7A dma route done.\r\n");
}

void ls7a_resource_cfg(ls7a_resource_table_t* cfg_table)
{
#ifndef PMON
	memcpy(&ls7a_cfg_t, cfg_table, sizeof(ls7a_resource_table_t));
#endif
	ls7a0_resource_cfg();
	if ((ls7a_cfg_t.ht.ls7a_node_num == 2) || (ls7a_cfg_t.ht.ls7a_con_type == 1))
		ls7a1_resource_cfg();
}

int ls7a_dbg(void)
{

	uint64_t store_val, val = 0x5a5aa5a516161616;
	uint32_t i, j = 0;

	/*Store the memory old val*/
	/*Cache addr 0x98xx*/
	store_val = readq(PHYS_TO_CACHED(0x0f000000));

	/*Test begain*/
	readq(PHYS_TO_UNCACHED(0x0f000000)) = val;
	readq(PHYS_TO_CACHED(0x0f000000)) = val;

	pr_info("Read local memory .. \n");
	if (readq(PHYS_TO_CACHED(0x0f000000)) != val) {
		pr_info("local cache compare fail!\n");
		j++;
	}

	pr_info("Read node0 7A ..\n");
	for(i = 0; i < 0x10000; i++) {
		if (readq(PHYS_TO_UNCACHED(0x0e000f000000)) != val) {
			pr_info("LS7A > node0 compare fail!\n");
			j++;
			break;
		}
	}

	if (ls7a_cfg_t.ht.ls7a_2way_connect) {
		switch (LS3A_NODE_NUM) {
		case 2:
		case 4:
			pr_info("Read node1 7A ..\n");
			if (readq(PHYS_TO_UNCACHED(0x1e000f000000)) != val) {
				pr_info("node1 > LS7A > node0 compare fail!\n");
			}
			break;
		case 8:
			pr_info("Read node5 7A ..\n");
			if (readq(PHYS_TO_UNCACHED(0x5e000f000000)) != val) {
				pr_info("node5 > LS7A > node0 compare fail!\n");
			}
			break;
		case 16:
			pr_info("Read node10 7A ..\n");
			if (readq(PHYS_TO_UNCACHED(0xae000f000000)) != val) {
				pr_info("node10 > LS7A > node0 compare fail!\n");
			}
			break;
		default:
			pr_info("ls3a node num error:%d\r\n", LS3A_NODE_NUM);
			break;
		}
	}
	/*Recover the memory old val*/
	/*Cache addr 0x98xx*/
	readq(PHYS_TO_CACHED(0x0f000000)) = store_val;
	return j ? -1 : 0;
}

void loongson_ht_trans_init(void)
{
	uint64_t i;
	uint64_t base;
	for (i = 0; i < LS3A_NODE_NUM; i++) {
		base = ((i << 44) | LOONGSON_HT1_INT_TRANS_ADDR);
		writeq(0x400000001fe01140ULL, base);
	}
}

/* 
 * Clean the Ls7a internal PCIe prot useless IRQ, to workaround the bug
 * that some PCIe device will creat pseudo interrupt in kernel.
 * This code has some risk that if the PCIe PHY connect device time are
 * not enough, this funciton will lose efficacy.
 */
int ls7a_clear_pcie_portirq(void)
{
	int i;

	for (i = 0; i < (ls7a_cfg_t.pcie.num); i++) {
		/* This debug print can not delete,if improve the cpu freq the code*/
		pr_info("clear pcie 0x%lx\n", ls7a_cfg_t.pcie.pcie_cfg_buffer[i]);
		/* Clear the PCIe internal port useless irq. Otherwise it maybe cause*/
		/* the linked device irq error in kernel.*/
		LS7A_PCIE_CLEAR_PORT_IRQ(ls7a_cfg_t.pcie.pcie_cfg_buffer[i]);
	}
	return 1;
}
