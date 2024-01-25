#include <cpu.h>
#include "target/ls7a_config.h"
#include "ht.h"
#include "gmemparam.h"

void ls7a2000_hw_init(void);
void ls7a2000_7a1_hw_init(void);
extern void prg_init(void);
extern void pcie_init(void);
extern void sata_phy_init(void);
extern void usb2_init(void);
extern void usb3_init(void);

extern void mm_feature_init_7a(void);
extern int ddr4_init (uint64_t node_num, ddr_ctrl *mm_ctrl_p);

extern uint64_t gl_node_id;

pcie_desc ls7a_pcie_ctrl[] = {
	{"F0", 0, LS7A_PCIE_F0_DISABLE, 8, LS7A_PCIE_GEN3_MODE, LS7A_PCIE_X4_MODE, LS7A_PCIE_EQ_MODE0},
	{"F1", 0, LS7A_PCIE_F1_DISABLE, 8, LS7A_PCIE_GEN3_MODE, LS7A_PCIE_X4_MODE, LS7A_PCIE_EQ_MODE0},
	{"H ", 0, LS7A_PCIE_H_DISABLE , 8, LS7A_PCIE_GEN3_MODE, LS7A_PCIE_X8_MODE, LS7A_PCIE_EQ_MODE0},
	{"G0", 0, LS7A_PCIE_G0_DISABLE, 8, LS7A_PCIE_GEN3_MODE, LS7A_PCIE_X16_MODE, LS7A_PCIE_EQ_MODE0},
#if (TOT_7A_NUM == 2) || (MULTI_7A_ON_PCIE == 1)
	{"F0", 0, LS7A_ANOTHER_PCIE_F0_DISABLE, 8, LS7A_PCIE_GEN3_MODE, LS7A_PCIE_X4_MODE, LS7A_PCIE_EQ_MODE0},
	{"F1", 0, LS7A_ANOTHER_PCIE_F1_DISABLE, 8, LS7A_PCIE_GEN3_MODE, LS7A_PCIE_X4_MODE, LS7A_PCIE_EQ_MODE0},
	{"H ", 0, LS7A_ANOTHER_PCIE_H_DISABLE,  8, LS7A_PCIE_GEN3_MODE, LS7A_PCIE_X8_MODE, LS7A_PCIE_EQ_MODE0},
	{"G0", 0, LS7A_ANOTHER_PCIE_G0_DISABLE, 8, LS7A_PCIE_GEN3_MODE, LS7A_PCIE_X16_MODE, LS7A_PCIE_EQ_MODE0},
#endif
};

sata_desc ls7a_sata_ctrl[] = {
	{LS7A_SATA_DISABLE},
	{LS7A_SATA0_DISABLE},
	{LS7A_SATA1_DISABLE},
	{LS7A_SATA2_DISABLE},
	{LS7A_SATA3_DISABLE},
};

usb_desc ls7a_usb_ctrl[] = {
	{LS7A_USB0_DISABLE, 0x1c0c8000, {{0, 3}}},
	{LS7A_USB1_DISABLE, 0x1c0c8000, {{0 ,3}}},
	/* XHCI ONLY support first param */
	{LS7A_XHCI_DISABLE, 0, {{0, 0}}},
};

gmac_desc ls7a_gmac_ctrl[] = {
	{LS7A_GMAC0_DISABLE},
	{LS7A_GMAC1_DISABLE},
};

void ls7a_pll_adapt(ls7a_pll_table* pll_ctrl)
{
	/*pcie, gmac, sata/usb*/
	pll_ctrl[LS7A_PLL0].pll_val = LS7A_PLL_VALUE(170, 17, 34, 14); //250 125 303
	pll_ctrl[LS7A_PLL0].div = 4;
	/*gpu, gmem, dc*/
	pll_ctrl[LS7A_PLL1].pll_val = LS7A_PLL_VALUE(48, 12, 4, 12); //400 1200 400
	pll_ctrl[LS7A_PLL1].div = (0 << LS7A_GMEM_DIV_RESETn_OFFSET) | (1 << LS7A_GMEM_RESETn_OFFSET) | (LS7A_GMEM_DIV_MODE << LS7A_GMEM_DIV_MODE_OFFSET) | 1;
	/*flex, node, hda bitclk*/
	pll_ctrl[LS7A_PLL2].pll_val = LS7A_PLL_VALUE(24, 24, 3, 100); //node 800
	pll_ctrl[LS7A_PLL2].div = 1;
	/*PIX0, default 38.2MHz for x800x600*/
	pll_ctrl[LS7A_PLL3].pll_val = LS7A_PLL_VALUE(40, 10, 10, 10);
	pll_ctrl[LS7A_PLL3].div = 1;
	/*PIX1, default 38.2MHz for x800x600*/
	pll_ctrl[LS7A_PLL4].pll_val = LS7A_PLL_VALUE(40, 10, 10, 10);
	pll_ctrl[LS7A_PLL4].div = 1;
#if (TOT_7A_NUM == 2) || (MULTI_7A_ON_PCIE == 1)
	//7a 2w
	/*pcie, gmac, sata/usb*/
	pll_ctrl[LS7A1_PLL0].pll_val = LS7A_PLL_VALUE(170, 17, 34, 14); //250 125 303
	pll_ctrl[LS7A1_PLL0].div = 4;
	/*flex, node, hda bitclk*/
	pll_ctrl[LS7A1_PLL2].pll_val = LS7A_PLL_VALUE(24, 24, 3, 100);
	pll_ctrl[LS7A1_PLL2].div = 1;
#endif
}

void ls7a_feature_init(void)
{
#ifdef LS7A_2WAY_CONNECT
	ls7a_cfg_t.ht.ls7a_2way_connect = 1;
#else
	ls7a_cfg_t.ht.ls7a_2way_connect = 0;
#endif
	ls7a_cfg_t.ht.ls7a_node_num = TOT_7A_NUM;
	ls7a_cfg_t.ht.ls3a_node_num = TOT_NODE_NUM;
	ls7a_cfg_t.ht.ls7a_con_type = 0;
#ifdef MULTI_7A_ON_PCIE
	ls7a_cfg_t.ht.ls7a_con_type = MULTI_7A_ON_PCIE;
#endif

	/* pll */
	ls7a_pll_adapt(ls7a_cfg_t.pll);

	/* pcie */
	ls7a_cfg_t.pcie.controller = ls7a_pcie_ctrl;
	/* sata */
	ls7a_cfg_t.sata.controller = ls7a_sata_ctrl;
	/* usb */
	ls7a_cfg_t.usb.controller = ls7a_usb_ctrl;
	/* gmac */
	ls7a_cfg_t.gmac.controller = ls7a_gmac_ctrl;
	/* iommu */
	ls7a_cfg_t.iommu.disable = LS7A_IOMMU_DISABLE;

	/* display */
	ls7a_cfg_t.dc.graphics_disable = LS7A_GRAPHICS_DISABLE;
	ls7a_cfg_t.dc.gmem_disable = LS7A_GMEM_DISABLE;
	ls7a_cfg_t.dc.gpu_disable = LS7A_GPU_DISABLE;
	ls7a_cfg_t.dc.vpu_disable = LS7A_VPU_DISABLE;

	/* misc devices */
	ls7a_cfg_t.misc.lpc_disable = LS7A_LPC_DISABLE;
	ls7a_cfg_t.misc.fan.min_rpm = 5000;
	ls7a_cfg_t.misc.fan.min_rpm = 10000;
}

void config_one_pll(uint64_t conf_base, int pll_num)
{
	uint32_t i, val32;
	uint64_t pll_base = conf_base + CONF_PLL0_OFFSET + (pll_num % 5) * 0x10;
	/*switch to backup clk*/
	readl(pll_base + 0x4) &= ~(0x7 << LS7A_PLL_SEL0_OFFSET);
	/*power down pll*/
	readl(pll_base + 0x4) |= (1 << LS7A_PLL_PD_OFFSET);
	/*disable pll configure*/
	readl(pll_base + 0x4) &= ~(1 << LS7A_PLL_SET_OFFSET);

	/*configure pll parameters*/
	readl(pll_base) = ls7a_cfg_t.pll[pll_num].pll_val;
	val32 = readl(pll_base + 0x4);
	if (pll_num == 1)
		readl(pll_base + 0x4) = (val32 & ~(0x3f << LS7A_PLL_DIV_REFC_OFFSET) & ~(0xf << LS7A_GMEM_DIV_RESETn_OFFSET)) | ls7a_cfg_t.pll[pll_num].div;
	else
		readl(pll_base + 0x4) = (val32 & ~(0x3f << LS7A_PLL_DIV_REFC_OFFSET)) | ls7a_cfg_t.pll[pll_num].div;

	/*enable pll configure*/
	readl(pll_base + 0x4) |= (1 << LS7A_PLL_SET_OFFSET);
	/*not bypass pll*/
	readl(pll_base + 0x4) &= ~(0x1 << LS7A_PLL_BYPASS_OFFSET);
	/*power up pll*/
	readl(pll_base + 0x4) &= ~(0x1 << LS7A_PLL_PD_OFFSET);

	/*poll lock signal*/
	i = 0x1000;
	do {
		val32 = readl(pll_base + 0x4) & (0x1 << LS7A_PLL_LOCK_OFFSET);
		i--;
	} while ((!val32) && i);

	if (i > 0) {
		/* select pll out */
		readl(pll_base + 0x4) |= (0x7 << LS7A_PLL_SEL0_OFFSET);
		return;
	}
	pr_info("!!!LS7A PLL%d soft configure fail.\r\n", pll_num % 5);
	while(1);
}

int device_cfg (void)
{
	uint64_t gmem_size;
	uint64_t i = 0;

	usb2_init();

	if (ls7a_cfg_t.usb.controller[2].disable)
		readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) &= ~(0x1 << 19);
	else
		usb3_init();

	prg_init();
	pcie_init();

	if (ls7a_cfg_t.sata.controller[0].disable) {
		readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET + 0x4) &= ~(0x1 << 11);
	} else {
		sata_phy_init();
		for(i = 0; i < 4; i++) {
			if (ls7a_cfg_t.sata.controller[i + 1].disable) {
				readl(LS7A_CONFBUS_BASE_ADDR + 0x740) |=  (0x1 << (12 + i));
				readl(LS7A_CONFBUS_BASE_ADDR + 0x740) &= ~(0x1 << (16 + i));
			}
		}
	}

	if (!ls7a_cfg_t.misc.lpc_disable) {
		readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET + 0x4) |= (0x1 << 0);
		pr_info("LPC enabled\r\n");
	}

	/*GMAC*/
	if (ls7a_cfg_t.gmac.controller[i].disable) {
		readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) &= ~(1 << 5);
		readl(LS7A_CONFBUS_BASE_ADDR + 0x770) &= ~(1 << 24);
		pr_info("GMAC0 disabled\r\n");
	}

	if (ls7a_cfg_t.gmac.controller[i].disable) {
		readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) &= ~(1 << 7);
		pr_info("GMAC1 disabled\r\n");
	}

	/*IOMMU*/
	if (ls7a_cfg_t.iommu.disable) {
		readl(PHYS_TO_UNCACHED(0xefe00000000 | 0x10 | (26 << 11))) = 0x60000000;
		readl(PHYS_TO_UNCACHED(0xefe00000000 | 0x04 | (26 << 11))) |= 0x147;
		readl(PHYS_TO_UNCACHED(0xe0060000000)) |= 1 << 30;
		/* release pcie mem */
		readl(PHYS_TO_UNCACHED(0xefe00000000 | 0x10 | (26 << 11))) = 0x0;
	}

	/*DC*/
	if (ls7a_cfg_t.dc.graphics_disable) {
		unsigned long dc_base_addr;
		dc_base_addr = PHYS_TO_UNCACHED(0xe0060000000);
		readl(PHYS_TO_UNCACHED(0xefe00000000 | 0x10 | (6 << 11) | (1 << 8))) = 0x60000000;
		readl(PHYS_TO_UNCACHED(0xefe00000000 | 0x04 | (6 << 11) | (1 << 8))) |= 0x147;
		//disable dc dma out put
		readl(dc_base_addr + 0x1240 + 0x0) &= ~(1 << 8);
		readl(dc_base_addr + 0x1250 + 0x0) &= ~(1 << 8);
		//disable dc clk
		readl(LS7A_CONFBUS_BASE_ADDR + 0x420) &= ~(1 << 7);
		//disable ClockEn
		readl(dc_base_addr + 0x1240 + 0x180) &= ~(1 << 8);
		readl(dc_base_addr + 0x1250 + 0x180) &= ~(1 << 8);
		//pix0 pd
		readl(LS7A_CONFBUS_BASE_ADDR + 0x4b4) |= (1 << 23);
		//pix1 pd
		readl(LS7A_CONFBUS_BASE_ADDR + 0x4c4) |= (1 << 23);
		//PhyResetn and PhyEn
		readl(dc_base_addr + 0x1240 + 0x5c0) &= ~(0x3 << 1);
		readl(dc_base_addr + 0x1250 + 0x5c0) &= ~(0x3 << 1);
		//Cfg_vdac_pd[4] VGA_off_det_en[1] VGA_on_det_en[0]
		readl(dc_base_addr + 0x1bb0) |= ((0x1 << 4) | (0x3 << 0));
		/* release pcie mem */
		readl(PHYS_TO_UNCACHED(0xefe00000000 | 0x10 | (6 << 11) | (1 << 8))) = 0x0;
	} else {
		readl(LS7A_CONFBUS_BASE_ADDR + 0x420) |= (1 << 7);
		pr_info("Graphics clk enabled\r\n");
	}

	/*GPU*/
	if (ls7a_cfg_t.dc.gpu_disable) {
		readl(LS7A_CONFBUS_BASE_ADDR + 0x420) &= ~(1 << 6);
		pr_info("GPU clk disabled\r\n");
	} else {
		readl(LS7A_CONFBUS_BASE_ADDR + 0x420) |= (1 << 6);
		pr_info("GPU clk enabled\r\n");
	}

	/*VPU*/
	if (ls7a_cfg_t.dc.vpu_disable) {
		readl(LS7A_CONFBUS_BASE_ADDR + 0x424) &= ~(1 << 26);
		pr_info("VPU clk disabled\r\n");
	} else {
		readl(LS7A_CONFBUS_BASE_ADDR + 0x424) |= (1 << 26);
		pr_info("VPU clk enabled\r\n");
	}

	/*put PCIE device detect later, else you need to add more delay*/
	/*delay at least 200ms*/
	loop_delay(0x1000000);

	if (ls7a_cfg_t.dc.gmem_disable) {
		//disable gmem_lpconf_en[29] gmem_lpmc_en[28]
		readl(LS7A_CONFBUS_BASE_ADDR + 0x424) |= (0x3 << 28);
		//disable gmem clk
		readl(LS7A_CONFBUS_BASE_ADDR + 0x420) &= ~(1 << 5);
		pr_info("Gmem clk disabled\r\n");
	} else {
		readl(LS7A_CONFBUS_BASE_ADDR + 0x420) |= (1 << 5);
		pr_info("Gmem clk enabled\r\n");
#if DEBUG_GMEM
		pr_info("\r\nInitial GMEM?(0xf: skip): ");
		if((inputaddress() & 0xf) <= 1)
#endif
		{
			pr_info("Gmem config begin\r\n");
			/*set gmem bar for init gmem use*/
			readq(GPU_HEADER_ADDR + 0x18) = TEMP_GMEM_ADDR;
			readl(GPU_HEADER_ADDR + 0x1c) = 0x0;
			/*mem space en*/
			readl(GPU_HEADER_ADDR + 0x4) = 0x2;

			//uint32_t s1_gmem;
			//s1_gmem = 0xc3a10404; /*memsize: unit 32MB*/
			//ls7a_gmem_init(s1_gmem);
			/***********
			config gmem window: //TODO
			1. conf_nb[0] = 1, 0xe0040000000 gmem
			2. conf_nb[0] = 0, gpu base and mask
			3. conf_nb[0] = 0, gpu base0/mask0 and base1/mask1
			***********/
			//readl(LS7A_CONFBUS_BASE_ADDR + CONF_NB_OFFSET) |= 0x1;
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_GMEM_BAR_MASK_OFFSET) = 0xfffffff;
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_GMEM_BAR_MASK_OFFSET + 0x4) = 0x0;
			mm_feature_init_7a();
			ddr4_init(0, &mm_ctrl_info);

			/*set gmem space bar mask*/
			gmem_size = mm_ctrl_info.paster.mc0_memsize;	//memsize: unit 1GB
			pr_info("Gmem size 0x%x\r\n", gmem_size);

			//test_gmem();
			/*recover gpu bar*/
			readl(GPU_HEADER_ADDR + 0x4 ) = 0;
			readl(GPU_HEADER_ADDR + 0x18) = 0;
			readl(GPU_HEADER_ADDR + 0x1c) = 0;
		}
	}

#ifdef LS7A_UC_ACC
	readl(LS7A_CONFBUS_BASE_ADDR + CONF_NB_OFFSET + 4) |= (0x3f << 0);
	readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET + 0) |= (0xef << 24);
	pr_info("LS7A uncache accellerator enabled\r\n");
#endif

	return 0;
}

void ls7a0_resource_cfg(void)
{
	uint64_t i;

	gl_node_id = 0;
	uint64_t base = (gl_node_id << NODE_OFFSET) | LS7A_CONFBUS_BASE_ADDR;

	ls7a_dis_ht_clk(base);

	ls7a_dma_cfg(base);

	/*configure 7A pll*/
	for (i = 0; i < 5; i++) {
		config_one_pll(base, i);
	}
	readl(base + CONF_PLL0_OFFSET + 0x10 + 0x4) |= (0x1 << LS7A_GMEM_DIV_RESETn_OFFSET);
	pr_info("LS7A pll configure done.\r\n");

	ls7a2000_hw_init();
	/*configure to obey strict HT order*/
	readl(base + 0x414) |= (0x7ffff << 0);

	/* gmac0 phy configure */
	readl(base + 0x770) &= ~0xf00;
	readl(base + 0x770) |= 0x4e00;

	/* [12:11]:  11b => HDA1, 01b => HDA, 10b => AC97 */
//	readl(base + 0x440) |= (3 << 11);
	/* enable HDA0 */
	readl(base + CONF_SB_OFFSET) |= (1 << 20);
	/* enable HDA1 */
	readl(base + CONF_SB_OFFSET) |= (1 << 21);
	readl(base + CONF_SB_OFFSET + 4) &= ~(1 << 31);

#if 1
	/* rewrite iommu header,don't scan it dev 26 fun 0 */
	readl(base + 0xa18) = 0xffffffff;
	readl(PHYS_TO_UNCACHED(0xefe0000d000)) = 0xffffffff;
	readl(base + 0xa18) = 0x0;
#else
	readl(base + 0xa18) = 0xfefefff0;
#endif

	/*change INT and HPET fix address*/
	readl(base + 0x460) = (INT_BASE_ADDR | 0x4);
	readl(base + 0x464) = (HPET_BASE_ADDR | 0x4);
	pr_info("LS7A hardware init done.\r\n");
	/*3. device configure*/
	device_cfg();
	pr_info("\r\nLS7A init done.\r\n");
}

void ls7a1_resource_cfg(void)
{
	uint64_t base;

	/* now use the ls7a_link_id_buf[1] get another 7A init node ID */
	gl_node_id = ls7a_link_id_buf[1];
	base = (gl_node_id << NODE_OFFSET) | LS7A_CONFBUS_BASE_ADDR;

	/* disable some useless device */
	readl(base + 0x420) &= (~(0x7 << 5));
	readl(base + 0x430) &= (~((1 << 5) | (1 << 7) | (1<< 10) | (1 << 14) | (1 << 18) | (1 << 21)));
	readl(base + 0x434) &= ~((1 << 0) | (1 << 10));

	ls7a_dma_cfg(base);

	/* configure 7A pll */
	config_one_pll(base, LS7A1_PLL0);
	config_one_pll(base, LS7A1_PLL2);
	pr_info("LS7A pll configure done.\r\n");

	/* configure to obey strict HT order */
	readl(base + 0x414) |= (0x7ffff << 0);

	/* PCIE bridge linked by F0 */
	if (ls7a_cfg_t.ht.ls7a_con_type == 1)
		readl(LS7A_CONFBUS_BASE_ADDR + 0x638) = 1;

	/* init 7a hardware */
	ls7a2000_7a1_hw_init();

	/* change INT and HPET fix baseess */
	readl(base + 0x460) = (INT_BASE_ADDR | 0x4);
	readl(base + 0x464) = (HPET_BASE_ADDR | 0x4);
	pr_info("LS7A hardware init done.\r\n");

	/* another 7A linked by HT */
	if (!ls7a_cfg_t.ht.ls7a_con_type)
		prg_init();
	pcie_init();
	pr_info("\r\nLS7A pcie init done.\r\n");

	/* put PCIE device detect later, else you need to add more delay */
	/* delay at least 200ms */
	loop_delay(0x10000000);
}
