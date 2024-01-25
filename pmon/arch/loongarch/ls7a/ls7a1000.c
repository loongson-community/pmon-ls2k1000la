#include <cpu.h>
#include "target/ls7a_config.h"
#include "ht.h"
#include "gmemparam.h"

#include "ls7a_gmem_config.c"

#define PCIE_TRY_GEN2			1
#define PCIE_TX_FULL_SWING		0
#define PCIE_STAT_CHECK_TIMES		20000	/*if some device link fail, maybe you can increase this value for try*/
#define PCIE_PD_LOOP			20
#define POWER_UP			1
#define POWER_DOWN			0

#define ABS_SUB(a1, a2)	((a1) > (a2) ? ((a1) - (a2)) : ((a2) - (a1)))


typedef struct current_sc {
	uint64_t clk1;
	uint64_t clk0;
	uint64_t current_score;
} current_score;

pcie_phy_param_t def_phy_cfg = {0xc2492331, 0x73e70b0, 0x20000};
pcie_desc ls7a_pcie_ctrl[] = {
	{0x0efe08004800, 0,  "F0", 0xf, LS7A_PCIE_F0_DISABLE, &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
	{0x0efe08006800, 4,  "F1", 0x3, LS7A_PCIE_F1_DISABLE, &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
	{0x0efe08009800, 6,  "H ", 0x3, LS7A_PCIE_H_DISABLE , &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
	{0x0efe08007800, 8,  "G0", 0x3, LS7A_PCIE_G0_DISABLE, &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
	{0x0efe08008800, 10, "G1", 0x3, LS7A_PCIE_G1_DISABLE, &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
#if (TOT_7A_NUM == 2)
	/* now the bus_base don,t contain node id */
	{0x0efe08004800, 0,  "F0", 0xf, LS7A_ANOTHER_PCIE_F0_DISABLE, &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
	{0x0efe08006800, 4,  "F1", 0x3, LS7A_ANOTHER_PCIE_F1_DISABLE, &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
	{0x0efe08009800, 6,  "H ", 0x3, LS7A_ANOTHER_PCIE_H_DISABLE , &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
	{0x0efe08007800, 8,  "G0", 0x3, LS7A_ANOTHER_PCIE_G0_DISABLE, &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
	{0x0efe08008800, 10, "G1", 0x3, LS7A_ANOTHER_PCIE_G1_DISABLE, &def_phy_cfg, LS7A_PCIE_ADAPT_MODE},
#endif
};

sata_desc ls7a_sata_ctrl[] = {
	{LS7A_SATA0_DISABLE, 0, 0x403f1002},
	{LS7A_SATA1_DISABLE, 0, 0x403f1002},
	{LS7A_SATA2_DISABLE, 0, 0x403f1002},
};

usb_desc ls7a_usb_ctrl[] = {
	{LS7A_USB0_DISABLE, 0, 0x02f80dc9, 0, 0x02f80dc9, 0, 0x02f80dc9},
	{LS7A_USB1_DISABLE, 1, 0x02f80dc9, 1, 0x02f80dc9, 1, 0x02f80dc9},
};

gmac_desc ls7a_gmac_ctrl[] = {
	{LS7A_GMAC0_DISABLE},
	{LS7A_GMAC1_DISABLE},
};

void ls7a_pll_adapt(ls7a_pll_table* pll_ctrl)
{
	/*pcie, gmac, sata/usb*/
	pll_ctrl[LS7A_PLL0].pll_val = LS7A_PLL_VALUE(80, 8, 16, 12);
	pll_ctrl[LS7A_PLL0].div = 4;
	/*gpu, gmem, dc*/
	pll_ctrl[LS7A_PLL1].pll_val = (ls7a_version() ? LS7A_PLL_VALUE(120, 5, 5, 12) : LS7A_PLL_VALUE(127, 8, 6, 12));
	pll_ctrl[LS7A_PLL1].div = 4;
	/*flex, node, hda bitclk*/
	pll_ctrl[LS7A_PLL2].pll_val = LS7A_PLL_VALUE(96, 72, (ls7a_version() ? 4 : 6), 100);
	pll_ctrl[LS7A_PLL2].div = 4;
	/*PIX0, default 38.2MHz for x800x600*/
	pll_ctrl[LS7A_PLL3].pll_val = LS7A_PLL_VALUE(104, 68, 68, 68);
	pll_ctrl[LS7A_PLL3].div = 4;
	/*PIX1, default 38.2MHz for x800x600*/
	pll_ctrl[LS7A_PLL4].pll_val = LS7A_PLL_VALUE(104, 68, 68, 68);
	pll_ctrl[LS7A_PLL4].div = 4;
#if	(TOT_NODE_NUM == 16)
	//7a 2w
	/*pcie, gmac, sata/usb*/
	pll_ctrl[LS7A1_PLL0].pll_val = LS7A_PLL_VALUE(80, 8, 16, 12);
	pll_ctrl[LS7A1_PLL0].div = 4;
	/*flex, node, hda bitclk*/
	pll_ctrl[LS7A1_PLL2].pll_val = LS7A_PLL_VALUE(96, 72, (ls7a_version() ? 4 : 6), 100);
	pll_ctrl[LS7A1_PLL2].div = 4;
#endif
}

void ls7a_feature_init(void)
{
#ifdef LS7A_2WAY_CONNECT
	ls7a_cfg_t.ht.ls7a_2way_connect = 1;
#else
	ls7a_cfg_t.ht.ls7a_2way_connect = 0;
#endif
	/* 7a1000 do'nt support pcie connection */
	ls7a_cfg_t.ht.ls7a_con_type = 0;
	ls7a_cfg_t.ht.ls7a_node_num = TOT_7A_NUM;
	ls7a_cfg_t.ht.ls3a_node_num = TOT_NODE_NUM;

	//pll
	ls7a_pll_adapt(ls7a_cfg_t.pll);

	//pcie
	ls7a_cfg_t.pcie.ref_clk = USE_INSIDE_REFCLK;
	ls7a_cfg_t.pcie.x8_cal_en = ENABLE_PCIEX8_CAL;
	ls7a_cfg_t.pcie.controller = ls7a_pcie_ctrl;

	//sata
	ls7a_cfg_t.sata.ref_clk = USE_INSIDE_REFCLK;
	ls7a_cfg_t.sata.controller = ls7a_sata_ctrl;

	//usb
	ls7a_cfg_t.usb.ref_clk = USE_INSIDE_REFCLK;
	ls7a_cfg_t.usb.controller = ls7a_usb_ctrl;

	//gmac
	ls7a_cfg_t.gmac.ref_clk = USE_INSIDE_REFCLK;
	ls7a_cfg_t.gmac.controller = ls7a_gmac_ctrl;

	//display
	ls7a_cfg_t.dc.graphics_disable = LS7A_GRAPHICS_DISABLE;
	ls7a_cfg_t.dc.gmem_cfg = LS7A_GMEM_CFG;

	//misc devices
	ls7a_cfg_t.misc.lpc_disable = LS7A_LPC_DISABLE;
	ls7a_cfg_t.misc.fan.min_rpm = 5000;
	ls7a_cfg_t.misc.fan.min_rpm = 10000;
}

void ls7a_get_pcie_dll_score(uint64_t confreg_addr, current_score *result)
{
	uint64_t control, compare, read_confreg, loop;
	result->clk1 = 0;
	result->clk0 = 0;
	result->current_score = PCIE_PD_LOOP * 2;
	loop = PCIE_PD_LOOP;
	do {
		read_confreg = readl(confreg_addr);
		compare = ((read_confreg >> 24 ) & 0xf);
		control = 0;
		do {
			switch (compare) {
			case 0x3:
				result->clk1 += (1 << 0);
				result->current_score -= 1;
				break;
			case 0xc:
				result->clk0 += (1 << 0);
				result->current_score -= 1;
				break;
			case 0x1:
			case 0x4:
			case 0x5:
				result->current_score += 1;
				break;
			default:
				break;
			}
			if (control != 0) {
				break;
			}
			control++;
			compare = ((read_confreg >> 28 ) & 0xf);
		} while (1);
		loop--;
	} while (loop);
}

void pcie_x8_conf(uint64_t node_id)
{
	uint64_t score;
	uint64_t reg_base, right_shift, adjust_value;
	current_score cs;
	reg_base = (node_id << NODE_OFFSET) | (LS7A_CONFBUS_BASE_ADDR + 0x5cc);
	/*for PCIE_H/G0/G1*/
	do {
		ls7a_get_pcie_dll_score(reg_base,&cs);
		/*find max of value 3 and c and determine the calibration direction*/
		/*when the two number has remarkable difference, start dll calibration*/
		if (ABS_SUB(cs.clk1, cs.clk0) >= (PCIE_PD_LOOP / 2)) {
			cs.clk1 > cs.clk0 ? (right_shift = 8) : (right_shift = 0);
			do {
				score = cs.current_score;
				adjust_value = ((readl(reg_base) >> right_shift) & 0xff);
				if (adjust_value == 0xff)
					break;
				adjust_value = (((adjust_value << 1) | 0x1 ) & 0xff) << right_shift;
				readl(reg_base) = adjust_value;
				ls7a_get_pcie_dll_score(reg_base, &cs);

				/*if this calibration has not make it remarkable worse, continue*/
			} while ((cs.current_score - score) > -2);
			/*make it worse a lot, scroll back*/
			readl(reg_base) = ((((readl(reg_base) >> right_shift) & 0xff) >> 1) << right_shift);
		}
		reg_base += 0x20;
	} while (reg_base <= ((node_id << NODE_OFFSET) | (LS7A_CONFBUS_BASE_ADDR + 0x60c)));
}

void phy_power_set(uint64_t addr, uint8_t opt)
{
	uint32_t val;
	int i;

	for (i = 0; i < 0xa0; i += 0x20) {
		val = readl(addr + 0x588 + i);
		val &= ~(1UL << 24);
		val |= ((opt ^ 1) << 24);
		readl(addr + 0x588 + i) = val;
	}
}

void sata_config(uint8_t sata_num)
{
	uint64_t ls7a_base = LS7A_CONFBUS_BASE_ADDR;
	uint32_t sata_offset = sata_num * 0x10;
	if (ls7a_cfg_t.sata.ref_clk == USE_INSIDE_REFCLK) {
		/*power down phy */
		readl(ls7a_base + 0x744 + sata_offset) |= (1 << 31);
		/*assert phy reset*/
		readl(ls7a_base + 0x740 + sata_offset) |= (1 << 2);
		/*switch refclk*/
		readl(ls7a_base + 0x740 + sata_offset) &= ~(1 << 1);
		loop_delay(0x1000);
		/*power up phy*/
		readl(ls7a_base + 0x744 + sata_offset) &= ~(1 << 31);
		/*deassert phy reset*/
		readl(ls7a_base + 0x740 + sata_offset) &= ~(1 << 2);
		loop_delay(0x10000);
	}
	/*deassert cntl reset*/
	readl(ls7a_base + CONF_SB_OFFSET + 4) &= ~(1 << (8 + sata_num * 0x4));
	/*sata en*/
	readl(ls7a_base + CONF_SB_OFFSET + 4) |= (1 << (10 + sata_num * 0x4));

	if (ls7a_cfg_t.sata.controller[sata_num].ovrd_en)
		ls7a_phy_cfg_write(ls7a_base + 0x748 + sata_offset, ls7a_cfg_t.sata.controller[sata_num].ovrd_val);
	pr_info("SATA%d enabled\n", sata_num);
}

void disable_sata(uint8_t sata)
{
	/* power down sata phy */
	readl(LS7A_CONFBUS_BASE_ADDR + 0x744 + sata * 0x10) |= (1 << 31);
	/* disable clock */
	readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET + 4) &= ~(1 << (11 + sata * 4));
	pr_info("SATA%d disabled\n", sata);
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

int ls7a_phy_cfg_write(uint64_t cfg_addr, uint32_t data)
{
	while(!(readl(cfg_addr + 0x4) & 0x4));

	readl(cfg_addr) = data;
	readl(cfg_addr + 0x4) = 0x1;

	return 0;
}

uint32_t ls7a_phy_cfg_read(uint64_t cfg_addr, uint32_t data)
{
	while(!(readl(cfg_addr + 0x4) & 0x4));

	readl(cfg_addr) = data;
	readl(cfg_addr + 0x4) = 0x0;

	while(!(readl(cfg_addr + 0x4) & 0x4));

	return readl(cfg_addr);
}

int ls7a_pcie_port_set(uint64_t node_id, int func_num, int port_num)
{
	int i, val, set_bits = 0;
	pcie_desc* ls7a_pcie_bus = &ls7a_cfg_t.pcie.controller[func_num + (node_id > 0 ? 5 : 0)];
	uint64_t base = PHYS_TO_UNCACHED(ls7a_pcie_bus->bus_base);
	base |= node_id << NODE_OFFSET;

	val = (readl(base + port_num * 0x800 + 0xc) & 0xfff9ffff) | ((PCIE_TRY_GEN2 << 17) | (PCIE_TX_FULL_SWING << 18));
	readl(base + port_num * 0x800 + 0xc) = val;
	readl(base - 0x1000000 + port_num * 0x800 + 0x1c) |= (1 << 26);
	readl(base - 0x8000000 + port_num * 0x800 + 0x10) = 0x60000000 + port_num * 0x100000;

	val = ~((0x7 << 18) | (0x7 << 2));
	readl((node_id << NODE_OFFSET) | (PHYS_TO_UNCACHED(0x0e0060000054) + port_num * 0x100000)) &= val;
	readl((node_id << NODE_OFFSET) | (PHYS_TO_UNCACHED(0x0e0060000058) + port_num * 0x100000)) &= val;
	readl((node_id << NODE_OFFSET) | (PHYS_TO_UNCACHED(0x0e0060000024) + port_num * 0x100000)) = 0;
	if (ls7a_version() && (func_num > 1))
		readl((node_id << NODE_OFFSET) | (PHYS_TO_UNCACHED(0x0e0060000028) + port_num * 0x100000)) |= 1;

	val = 0xff204c;
	if (ls7a_pcie_bus->mode & LS7A_PCIE_FLIP_EN)
		val |= 3;
	readl(node_id << NODE_OFFSET | (PHYS_TO_UNCACHED(0x0e0060000000) + port_num * 0x100000)) = val;

	/*read link state*/
	i = PCIE_STAT_CHECK_TIMES;
	while (!(readl((node_id << NODE_OFFSET) | (PHYS_TO_UNCACHED(0x0e006000000c) + port_num * 0x100000)) & 0x3f) && --i);
	if (!i) {
		set_bits |= (1 << port_num);
	}

	/*Port 0*/
	/*read x4_mode to decide whether P1 / P1~P3 is valid*/
	if (port_num == 0) {
		/* for H / G0 / G1, the bit26 is x8 mode */
		if ((func_num > 1) ^ ((readl(node_id << NODE_OFFSET | PHYS_TO_UNCACHED(0x0e0060000028)) >> 26) & 1)) {
			set_bits |= func_num == 0 ? 0xe : 0x2;
		}
	}
	pr_info("ls7a_pcie_port_set port[%d],set_bits[0x%x],[0x%x]\n", port_num, set_bits, readl(node_id << NODE_OFFSET | PHYS_TO_UNCACHED(0x0e0060000028)));
	base = base - 0x8000000 + port_num * 0x800 + 0x10;
	readl(base) = 0;
	ls7a_cfg_t.pcie.pcie_cfg_buffer[(ls7a_cfg_t.pcie.num)++] = base;
	return set_bits;
}

void ls7a_pcie_disable(uint64_t node_id)
{
	int func_num, i, set_bits;
	volatile void *addr;
	pcie_desc *ls7a_pcie_bus;

	ls7a_pcie_bus = &ls7a_cfg_t.pcie.controller[node_id > 0 ? 5 : 0];

	addr = ((node_id << NODE_OFFSET) | LS7A_CONFBUS_BASE_ADDR);

	for (func_num = 0; func_num < 5; func_num++) {
		if (!ls7a_pcie_bus[func_num].disable) {
			/*enable access; F0:1 << 9; F1: 1 << 17; H: 1 << 21; G0: 1 << 25; G1: 1 << 29*/
			readl(addr + CONF_NB_OFFSET) |= 1 << (func_num ? ((func_num - 1) * 4 + 17) : 9);
			pr_info("PCIE %s enabled, [0x%x]\n", ls7a_pcie_bus[func_num].name, readl(addr + CONF_NB_OFFSET));

			/*re-configure PCIE PHYs*/
			ls7a_phy_cfg_write(addr + 0x590 + func_num * 0x20, 0x4fff1002);
			ls7a_phy_cfg_write(addr + 0x590 + func_num * 0x20, 0x4fff1102);
			ls7a_phy_cfg_write(addr + 0x590 + func_num * 0x20, 0x4fff1202);
			ls7a_phy_cfg_write(addr + 0x590 + func_num * 0x20, 0x4fff1302);
			/*PCIE H / G0 / G1*/
			if (func_num > 1) {
				ls7a_phy_cfg_write(addr + 0x598 + func_num * 0x20, 0x4fff1002);
				ls7a_phy_cfg_write(addr + 0x598 + func_num * 0x20, 0x4fff1102);
				ls7a_phy_cfg_write(addr + 0x598 + func_num * 0x20, 0x4fff1202);
				ls7a_phy_cfg_write(addr + 0x590 + func_num * 0x20, 0x4fff1302);
			}

			set_bits = ls7a_pcie_port_set(node_id, func_num, 0);
			if (set_bits <= 1) {
				/*more ports*/
				for (i = 1; ls7a_pcie_bus[func_num].port_bits >> i; i++)
					set_bits |= ls7a_pcie_port_set(node_id, func_num, i);
			}

			if (ls7a_pcie_bus[func_num].mode & LS7A_PCIE_NO_POWERDOWN)
				set_bits = 0;
		} else {	/*disable this PCIE*/
			set_bits = ls7a_pcie_bus[func_num].port_bits;
		}
		if (set_bits == ls7a_pcie_bus[func_num].port_bits) {
			/*powerdown phy*/
			readl(addr + 0x588 + func_num * 0x20) |= ((func_num > 1 ? 3 : 1) << 24);
			readl(addr + CONF_NB_OFFSET) &= ~(1 << (func_num ? ((func_num - 1) * 4 + 17) : 9));
			pr_info("Powerdown PCIE %s PHY and disable all Ports.\n", ls7a_pcie_bus[func_num].name);
		} else {
			pr_info("not Powerdown PCIE %s.set_bits[0x%x]\n", ls7a_pcie_bus[func_num].name, set_bits);
		}

		/*disable clock of unused PCIE ports*/
		readl(addr + CONF_NB_OFFSET) &= ~(set_bits << (func_num ? ((func_num - 1) * 4 + 18) : 10));
		pr_info("unused PCIE %s ports clock disabled.\n", ls7a_pcie_bus[func_num].name);
	}
}

void pcie_cfg(uint64_t node_id)
{
	uint64_t i = 0;
	void *addr;
	pcie_desc *ls7a_pcie_bus;

	addr = (node_id << NODE_OFFSET) | LS7A_CONFBUS_BASE_ADDR;
	ls7a_pcie_bus = &ls7a_cfg_t.pcie.controller[node_id > 0 ? 5 : 0];

	/*init PCIE -- v0.3*/
	/*configure phy parameter*/
	for (i = 0; i < 5; i++) {
		readl(addr + 0x580 + i * 0x20) = ls7a_pcie_bus[i].phy_param->val_0;
		readl(addr + 0x584 + i * 0x20) = ls7a_pcie_bus[i].phy_param->val_4;
		readl(addr + 0x588 + i * 0x20) = ls7a_pcie_bus[i].phy_param->val_8;
	}
	/*assert reset*/
	readl(addr + CONF_NB_OFFSET) |= ((1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8));

	/*enable all ports*/
	readl(addr + 0x588) |= (1 << 27);
	readl(addr + 0x5a8) |= (1 << 27);
	readl(addr + 0x5c8) |= (3 << 26);
	readl(addr + 0x5e8) |= (3 << 26);
	readl(addr + 0x608) |= (3 << 26);

	/*power down phy*/
	phy_power_set(addr, POWER_DOWN);

	/*switch to inside ref clk*/
	readl(addr + CONF_NB_OFFSET) |= 1 << 2;

	/*delay 10ms*/
	loop_delay(0x100000);

	/*power up phy*/
	phy_power_set(addr, POWER_UP);

	/*deassert reset*/
	readl(addr + CONF_NB_OFFSET) &= ~((1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8));
	/*make sure all ports clock are ready*/
	while (((readl(addr + 0x424) >> 8) & 0xfff) != 0xfff);

	/*
	 * note: this delay not enough will caused pcie controller status not stable
	 * and some device con't be founded such as 82580 network in 8632 bridge
	 l*/

	/*delay at least 100ms*/
	loop_delay(0x1000000);

	/*unless specified, recover to use prsnt pin to decide device existence.*/
	for (i = 0; i < 5; i++) {
		switch (ls7a_cfg_t.pcie.controller[i].mode & 0xf) {
		case LS7A_PCIE_X1_MODE:
		case LS7A_PCIE_X8_MODE:
			readl(addr + 0x588 + i * 0x20) |= (1 << 27);
			readl(addr + 0x588 + i * 0x20) &= ~(1 << 26);
			break;
		case LS7A_PCIE_X4_MODE:
			readl(addr + 0x588 + i * 0x20) |= (1 << 27);
			readl(addr + 0x588 + i * 0x20) |= (1 << 26);
			break;
		case LS7A_PCIE_ADAPT_MODE:
			readl(addr + 0x588 + i * 0x20) &= ~(1 << 27);
			break;
		}
	}

	/*assert reset*/
	readl(addr + CONF_NB_OFFSET) |= (1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8);

	if (ls7a_cfg_t.pcie.ref_clk == USE_INSIDE_REFCLK) {
		loop_delay(0x1000000);
	} else {
		/*power down phy*/
		phy_power_set(addr, POWER_DOWN);

		/*switch to use outside ref clk*/
		readl(addr + CONF_NB_OFFSET) &= ~(1 << 2);

		loop_delay(0x1000000);

		/*power up phy*/
		phy_power_set(addr, POWER_UP);
	}
	/*deassert reset*/
	readl(addr + CONF_NB_OFFSET) &= ~((1 << 28) | (1 << 24) | (1 << 20) | (1 << 16) | (1 << 8));

	if (ls7a_cfg_t.pcie.x8_cal_en == ENABLE_PCIEX8_CAL)
		pcie_x8_conf(node_id);
}

int device_cfg (void)
{
	uint32_t s1_gmem, gmem_size;
	uint64_t i = 0;

	pcie_cfg (0);

	/*init SATA*/
	readl(LS7A_CONFBUS_BASE_ADDR + 0x744) = 0x30c31cf9;
	readl(LS7A_CONFBUS_BASE_ADDR + 0x754) = 0x30c31cf9;
	readl(LS7A_CONFBUS_BASE_ADDR + 0x764) = 0x30c31cf9;
	readl(LS7A_CONFBUS_BASE_ADDR + 0x740) = 0xf3000403;
	readl(LS7A_CONFBUS_BASE_ADDR + 0x750) = 0xf3000403;
	readl(LS7A_CONFBUS_BASE_ADDR + 0x760) = 0xf3000403;

	for (i = 0; i < 3; i++) {
		if (ls7a_cfg_t.sata.controller[i].disable)
			disable_sata(i);
		else
			sata_config(i);
	}

	/*init USB*/
	if (ls7a_cfg_t.usb.ref_clk == USE_INSIDE_REFCLK) {
		/*switch refclk*/
		readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) |= (2 << 2);
		loop_delay(0x1000);
	}
	for (i = 0; i < 2; i++) {
		if (ls7a_cfg_t.usb.controller[i].disable) {
			/*disable clock*/
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) &= ~(1 << (11 + i * 4));
			pr_info("USB%d disabled\r\n", i);
		} else {
			/*deassert phy reset*/
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) &= ~(1 << (9 + i * 4));
			loop_delay(0x10000);
			/*deassert cntl reset*/
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) &= ~(1 << (8 + i * 4));
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) |= (1 << (10 + i * 4));
			/* increase usb driving strength */
			if (ls7a_cfg_t.usb.controller[i].port0_ovrd_en)
				readl(LS7A_CONFBUS_BASE_ADDR + 0x700 + i * 0x10) = ls7a_cfg_t.usb.controller[i].port0_ovrd_val;
			if (ls7a_cfg_t.usb.controller[i].port1_ovrd_en)
				readl(LS7A_CONFBUS_BASE_ADDR + 0x704 + i * 0x10) = ls7a_cfg_t.usb.controller[i].port1_ovrd_val;
			if (ls7a_cfg_t.usb.controller[i].port2_ovrd_en)
				readl(LS7A_CONFBUS_BASE_ADDR + 0x708 + i * 0x10) = ls7a_cfg_t.usb.controller[i].port2_ovrd_val;
			pr_info("USB%d enabled\r\n", i);
		}
	}

	/*init GMAC*/
	for (i = 0; i < 2; i++) {
		if (ls7a_cfg_t.gmac.controller[i].disable) {
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET) &= ~(1 << (5 + i * 2));
			pr_info("GMAC%d disabled\r\n", i);
		}
	}

	/*init DC*/
	if (!ls7a_cfg_t.misc.lpc_disable) {
		readl(LS7A_CONFBUS_BASE_ADDR + CONF_SB_OFFSET + 0x4) |= (0x1 << 0);
		pr_info("LPC enabled\r\n");
	}
	if (ls7a_cfg_t.dc.graphics_disable) {
		readl(LS7A_CONFBUS_BASE_ADDR + 0x420) &= ~(1 << 5);
		pr_info("Graphics disabled\r\n");
	}

	/*put PCIE device detect later, else you need to add more delay*/
	/*delay at least 200ms*/
	loop_delay(0x1000000);

	ls7a_pcie_disable(0);

	if (ls7a_cfg_t.dc.gmem_cfg) {
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

			s1_gmem = 0xc3a10404; /*memsize: unit 32MB*/
			ls7a_gmem_init(s1_gmem);

			/*set gmem space bar mask*/
			gmem_size = ((s1_gmem >> 0x8) & 0x7f);
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_GMEM_BAR_MASK_OFFSET) = ((gmem_size << 25) - 1);
			readl(LS7A_CONFBUS_BASE_ADDR + CONF_GMEM_BAR_MASK_OFFSET + 0x4) = 0x0;

			uint64_t gmem_addr = LS7A_GMEM_TEMP_ADDR;

			readq(gmem_addr+0x00) = 0x5555555555555555;
			readq(gmem_addr+0x08) = 0xaaaaaaaaaaaaaaaa;
			readq(gmem_addr+0x10) = 0x3333333333333333;
			readq(gmem_addr+0x18) = 0xcccccccccccccccc;
			readq(gmem_addr+0x20) = 0x7777777777777777;
			readq(gmem_addr+0x28) = 0x8888888888888888;
			readq(gmem_addr+0x30) = 0x1111111111111111;
			readq(gmem_addr+0x38) = 0xeeeeeeeeeeeeeeee;


			pr_info("The gmem data is:\n");
			for (i = 0; i < 8; i++) {
				pr_info("0x%03x : 0x%llx\n", i*8, readq(gmem_addr + i*8));
			}

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
	uint64_t base = (0UL << NODE_OFFSET) | LS7A_CONFBUS_BASE_ADDR;

	ls7a_dis_ht_clk(base);

	ls7a_dma_cfg(base);

	/*configure 7A pll*/
	for (i = 0; i < 5; i++) {
		config_one_pll(base, i);
	}
	pr_info("LS7A pll configure done.\r\n");

	/*init 7a hardware*/
	/*configure to obey strict HT order*/
	readl(base + 0x414) |= (0x7ffff << 0);
	/*rewrite pci header default value*/
	for (i = 0; i <= 0x480; i += 0x40) {
		readb(base + 0x300c + i) = 0x0;
		readl(base + 0x303c + i) = 0x100;
	}
	/*rewrite HEADER TYPE to Multi-function*/
	/*gmac0*/
	readb(base + 0x304e) = 0x80;
	/*gmac1*/
	readb(base + 0x308e) = 0x80;
	/*sata0*/
	readb(base + 0x32ce) = 0x80;
	/*sata1*/
	readb(base + 0x330e) = 0x80;
	/*sata2*/
	readb(base + 0x334e) = 0x80;
	/*fix GPU/DC header*/
	readh(base + 0x31ca) = 0x0400;
	readh(base + 0x320a) = 0x0380;

	/*fix LPC header*/
	readh(base + 0x344a) = 0x0601;
	readl(base + 0x3460) = 0xfc000000;
	readl(base + 0x3464) = 0xfd;
	readl(base + 0x3450) = LPC_CNTL_BASE_ADDR;
	readl(base + 0x3458) = LPC_MEM_BASE_ADDR;
	readl(base + 0x3444) |= 0x3;

	/*disable pci scan of MISC and confbus*/
	readl(base + 0x3800) |= 0xf;
	readl(base + 0x3878) |= 0xf;
	readl(base + 0x3000) |= 0xffffffff;
	readl(base + 0x33c0) |= 0xffffffff;

	/*Disable write to RO for IRQ Line bits of device header ---- start*/
	for (i = 0; i <= 0x90; i += 0x8) {
		readh(base + 0x3800 + i) = 0x10;
		/* Bit 60, PCI header offset 3Ch. Interrupt Line */
		readl(base + 0x3804 + i) = 0x10000000;
	}

	/*Special take care of GMEM BAR, clear to 4KB*/
	/*if defined CFG_GMEM, this Register will be reconfigured.*/
	readl(base + 0x3838) = 0xfff;
	readl(base + 0x383c) = 0x0;
	/*Special take care of GPU-BAR3(64)*/
	readl(base + 0x3844) |= 0xfc;
	/*Special take care of LPC*/
	readl(base + 0x3888) = 0x0;
	/*Disable write to part of header ---- end*/

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
	uint64_t node_id;
	uint64_t i;

	node_id = ls7a_link_id_buf[1];
	base = (node_id << NODE_OFFSET) | LS7A_CONFBUS_BASE_ADDR;

	ls7a_dma_cfg(base);

	/*configure 7A pll*/
	config_one_pll(base, LS7A1_PLL0);
	config_one_pll(base, LS7A1_PLL2);
	pr_info("LS7A pll configure done.\r\n");

	/*init 7a hardware*/
	/*configure to obey strict HT order*/
	readl(base + 0x414) |= (0x7ffff << 0);
	/*rewrite pci header default value*/
	for (i = 0; i <= 0x480; i += 0x40) {
		readb(base + 0x300c + i) = 0x0;
		readl(base + 0x303c + i) = 0x100;
	}

	/*disable all device except pcie F/G/H */
	for (i = 0; i <= 0x90; i += 0x8) {
		readl(base + 0x3800 + i) |= 0xff;
	}
	for (i = 0; i <= 0x480; i += 0x40) {
		readl(base + 0x3000 + i) |= 0xffffffff;
		readl(base + 0x3004 + i) |= 0xffffffff;
	}

	/*Disable write to RO for IRQ Line bits of device header ---- start*/
	for (i = 0; i <= 0x90; i += 0x8) {
		readh(base + 0x3800 + i) = 0x10;
		/* Bit 60, PCI header offset 3Ch. Interrupt Line */
		readl(base + 0x3804 + i) = 0x10000000;
	}

	/*change INT and HPET fix baseess*/
	readl(base + 0x460) = (INT_BASE_ADDR | 0x4);
	readl(base + 0x464) = (HPET_BASE_ADDR | 0x4);
	pr_info("LS7A hardware init done.\r\n");
	/*Disable write to part of header ---- end*/

	pcie_cfg(node_id);
	pr_info("\r\nLS7A pcie init done.\r\n");

	/*put PCIE device detect later, else you need to add more delay*/
	/*delay at least 200ms*/
	loop_delay(0x10000000);

	ls7a_pcie_disable(node_id);
}
