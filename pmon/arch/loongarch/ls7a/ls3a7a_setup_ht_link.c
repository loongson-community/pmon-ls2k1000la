#include <cpu.h>
#include "target/ls7a_config.h"
#include "ht.h"
#define CHIP_SAMPLE_OFFSET			(PHYS_TO_UNCACHED(0x1fe00000) + 0x190)
#define ERR_NOT_READY				-1
#define WAIT_HT_UP				0x1
#define WAIT_HT_DOWN				0x0

extern int ls7a_version(void);
extern void reset_ht(uint64_t node, uint64_t ht_num);
extern void wait_ht_down(uint64_t node, uint64_t ht_num);
extern void dereset_ht(uint64_t node, uint64_t ht_num);
extern int get_x_link_node_id(uint64_t node_id);
extern void ht_link_reset(uint64_t node, uint64_t ht, uint64_t dst_node, uint64_t dst_ht);
extern int check_ht_link(uint64_t node, uint64_t ht_num);
extern int  ht_wait_link_up(uint64_t node, uint64_t ht);
extern ls7a_resource_table_t ls7a_cfg_t;

void loop_delay(uint64_t loops)
{
	volatile uint64_t counts = loops;
	while (counts--);
}

void enable_fast_path(char num)
{
	uint64_t i;
	for(i = 0; i < num; i++)
		readq(PHYS_TO_UNCACHED(0x1fe00400) | (i << NODE_OFFSET)) |= (0x3ULL << 10);
}

void ht_reg_disable(char *buf, char size)
{
	uint64_t node_id;
	uint64_t val, val1;

	/*disable ht regs*/
	for (node_id = 0; node_id < LS3A_NODE_NUM; node_id++) {
		val = 0xf;
		if ((node_id == *buf) && size) {
			buf++;
			size--;
			val = 0xb;
		}
		val1 = (readq(PHYS_TO_UNCACHED(0x1fe00400) | (node_id << NODE_OFFSET)) & ~(0xfULL << 44)) | (val << 44);
		readq(PHYS_TO_UNCACHED(0x1fe00400) | (node_id << NODE_OFFSET)) = val1;
	}
}

void enable_xlink_c(char num)
{
	uint64_t i;
	for(i = 0; i < num; i++)
		readq(PHYS_TO_UNCACHED(0x1fe00400) | (i << NODE_OFFSET)) |= (0x1ULL << 8);
}

void set_buff_num(uint64_t ht_conf_addr)
{
	uint32_t val;

	val = readl(ht_conf_addr + LS3A_HT_RX_BUFFER_DEFAULT);
	if (((uint64_t)ht_conf_addr & 0xf000000) == 0xb000000) {
		val |= 0xfffffff;
	} else {
		val |= 0xfffffff;
	}
	readl(ht_conf_addr + LS3A_HT_RX_BUFFER_DEFAULT) = val;
	pr_info("Set buffer num 0x%x\n", readl(ht_conf_addr + LS3A_HT_RX_BUFFER_DEFAULT));
}

void beep_on(void)
{
	readb(LS7A_MISC_BASE_ADDR | (GPIO_BASE_ADDR_OFFSET + 0x900)) = 0x0;
}

void beep_off(void)
{
	readb(LS7A_MISC_BASE_ADDR | (GPIO_BASE_ADDR_OFFSET + 0x900)) = 0x1;
}

void ctrl_beep_pwm(uint64_t node_id)
{
	uint64_t base = (node_id << NODE_OFFSET) | LS7A_PWM0_REG_BASE;
	/*set PWM output 1*/
	readl(base + 4) = 0;
	readl(base + 8) = 0x100;
	readl(base + 0xc) = 1 << 0;
	readl(base + 0x104) = 0;
	readl(base + 0x108) = 0x100;
	readl(base + 0x10c) = 1 << 0;
	readl(base + 0x204) = 0;
	readl(base + 0x208) = 0x100;
	readl(base + 0x20c) = 1 << 0;
	readl(base + 0x304) = 0;
	readl(base + 0x308) = 0x100;
	readl(base + 0x30c) = 1 << 0;
}

int check_ht_pll_lock(uint64_t base)
{
	uint32_t val;

#ifdef CHECK_HT_PLL_LOCK
	val = readl(base + LS3A_HT_PLL_CONF);
	if (!(val & (1 << 3))) {
		pr_info("Error: CPU HT PLL not locked!!!\n");
		while(1);
	}
#ifdef CHECK_7A_HT_PLL_LOCK
	val = readl((base | (0xeUL << 24)) + LS3A_HT_PLL_CONF);
	if (!(val & (1 << 3))) {
		pr_info("Error: 7A HT PLL not locked!!!\n");
		while(1);
	}
#endif
	pr_info("PLL check success.\n");
#endif
	return 0;
}

void check_ls3a_ht_clk_sel(void)
{
	uint32_t val;

	/* checking CPU clock setting select */
	val = readl(CHIP_SAMPLE_OFFSET | 0x4);
	pr_info( "clksel= %lx\n", val);

	if (val & (0x1 << 15)) {
		pr_info("Warning: CPU HT in hard freq mode\n");
	} else
		pr_info("CPU HT in soft freq cfg mode...ok\n");
}

void check_ls7a_ht_clk_sel(void)
{
	uint64_t base;
	uint64_t node_base = 0;

	do {
		base = (LS7A_MISC_BASE_ADDR | GPIO_BASE_ADDR_OFFSET | (0xa00 + 53)) | node_base;
		if (readb(base)) {
			pr_info("Warning: 7A HT in hard freq mode\n");
		} else
			pr_info( "7A HT in soft freq cfg mode\n");
		if ((ls7a_cfg_t.ht.ls7a_node_num == 2) && (node_base == 0)) {
			node_base = (5ULL << NODE_OFFSET);
			continue;
		}
		break;
	} while (1);
}

void ls7a_fix_addr(void)
{
	uint64_t ht_conf_base;
	uint64_t node_id = 0;
	uint64_t addr;

	do {
		ht_conf_base = (HT1_CONF_BASE | (node_id << NODE_OFFSET));
		/*config fix address bar for Misc devices block*/
		addr = (ht_conf_base | (2 << 11) | (0 << 8));
		readl(addr + 0x10) = MISC_BASE_ADDR;
		readl(addr + 0x4) |= 0x2;

		/*change confbus base address*/
		addr = (ht_conf_base | (21 << 11) | (0 << 8));
		readl(addr + 0x10) = CONFBUS_BASE_ADDR;
		readl(addr + 0x4) |= 0x2;

		if (!(node_id)) {
			pr_info("set LS7A MISC and confbus base address done.\n");
			beep_on();
			ctrl_beep_pwm(node_id);
			beep_off();

			/*clear ACPI register*/
			addr = ((node_id << NODE_OFFSET) | LS7A_ACPI_REG_BASE);
			readl(addr + 0xc ) &= 0xffffffff;
			readl(addr + 0x28) &= 0xffffffff;
			readl(addr + 0x2c) &= 0xffffffff;
		}

		if (((ls7a_cfg_t.ht.ls7a_node_num == 2) || (ls7a_cfg_t.ht.ls7a_con_type == 1)) && (node_id == 0)) {
			/* another 7a linked node_id */
			node_id = ls7a_link_id_buf[1];
			continue;
		}
		break;
	} while (1);
}

void v_ctrl(void)
{
#ifdef VOLTAGE_CTRL
	int i = 0, vddn, skip = 1;
	if (CORE_FREQ > 2400)
		vddn = VOL_mV(1250);
	else if (CORE_FREQ > 1800)
		vddn = VOL_mV(1150);
	else if (CORE_FREQ > 1600)
		vddn = VOL_mV(1050);
	else if (CORE_FREQ > 1200)
		vddn = VOL_mV(950);
	else
		vddn = VOL_mV(950);

	if (LS3A_NODE_NUM >= 8) {
#if (TOT_NODE_NUM >= 8) && defined(BOARD_3C5000L_2W_V01)
		i = 1;
#endif
		skip = 4;
	}

	for (; i < LS3A_NODE_NUM; i += skip)
		v_n_ctrl(vddn, i);
#endif
}

int check_crc_err(void *ht_base)
{
	int i;
	uint32_t val;

	for (i = 256; i; i--) {
		pr_info(">");
		val = readl(ht_base + 0x44);
		if (val & 0x300) {
			pr_info("CRC error found.\n0x%lx\n", val);
			readl(ht_base + 0x44) &= ~0x300UL;
		} else
			break;
	}
	return  !(i);
}

void dereset_ht_x_link(void)
{
	int node_id, dst_node_id;
	for (node_id = 0; node_id < TOT_NODE_NUM; node_id++) {
		if ((node_id % 4) >= 2)
			continue;
		dst_node_id = get_x_link_node_id(node_id);
		dereset_ht(node_id, HT1_HI);
		pr_info("Waiting node%d %x HyperTransport bus to be up.\n", node_id, HT1_HI);
		ht_wait_link_up(node_id, HT1_HI);
		pr_info("Waiting node%d %x HyperTransport bus to be up.\n", dst_node_id, HT1_HI);
		ht_wait_link_up(dst_node_id, HT1_HI);
	}
}
int reset_ht_link_one(uint64_t node_id)
{
	void* base_7a = (PHYS_TO_UNCACHED(0x0efdfe000000) | node_id << 44);
	uint64_t base0_ht1_lo = (PHYS_TO_UNCACHED(0x0efdfb000000) | node_id << 44);
	char	ret;
#if	HT1_RECONNECT
	do {
		/* reset_link 7a node_id ht1 lo_bus */
		pr_info("Reset NODE%d HT1-lo bus\n", node_id);
		reset_ht(node_id, HT1_LO);
		wait_ht_down(node_id, HT1_LO);

#ifndef DISABLE_X_LINK
		if (LS3A_NODE_NUM == 4) {
			uint64_t dst_node_id = get_x_link_node_id(node_id);

			/* x reset by CPU-CPU link code */
			pr_info("HT x %d -> %d linking\n", node_id, dst_node_id);
			if (node_id < dst_node_id) {
				ht_link_reset(node_id, HT1_HI, dst_node_id, HT1_HI);
			} else {
				ht_link_reset(dst_node_id, HT1_HI, node_id, HT1_HI);
			}
			pr_info(" complete!\n");
		}
#endif
		pr_info("Dereset NODE%d HT1-LO BUS\n", node_id);
		dereset_ht(node_id, HT1_LO);
		pr_info("after 0x%x\n",readl(base0_ht1_lo + 0x3c));

		check_ht_link(node_id, HT1_LO);
		ret = ht_wait_link_up(node_id, HT1_LO);
		if (ret < 0) {
			continue;
		}
		pr_info("Checking NODE%d HT1 LO CRC error done!\n", node_id);
		check_ht_pll_lock(base0_ht1_lo);

    		pr_info("Checking Bridge HT CRC error bit.\n");
		ret = check_crc_err(base_7a);
		if (ret)
			continue;
	} while(ret);
#else
	pr_info("no reconnet.\n");
#endif
	return 0;
}

int reset_ht_link(char *buf, char size)
{
	int i;
	/* first reset all 7A linked ht */
	for(i = 0; i < size;i++)
		reset_ht_link_one(buf[i]);
#ifndef DISABLE_X_LINK
	if(TOT_NODE_NUM >= 8) {
		dereset_ht_x_link();
	}
#endif
	return 0;
}

void ht_link_cfg(uint32_t ht1_cfg, uint64_t ht_base)
{
	uint8_t val;

	readl(ht_base + 0x1e0) = 0x80000080;
	readl(ht_base + 0x1e4) = 0xff;
	readl(ht_base + 0x1e8) = 0xffffffff;
	readl(ht_base + 0x1ec) = 0xffffffff;

	if ((ht1_cfg & 0x2) == 0)
		val = HT_WIDTH_CTRL_8;
	else
		val = HT_WIDTH_CTRL_16;
	readb(ht_base + 0x47) = val;
	pr_info( "Set width 0x%x\n", readl(ht_base + 0x44));

	val = readb(ht_base + 0x4c + 1);
	if (((uint64_t)ht_base & 0xf000000) == 0xb000000) {
		val = (val & ~0xf) | ((ht1_cfg >> 8) & 0xf);
	} else {
		val = (val & ~0xf) | ((ht1_cfg >> 12) & 0xf);
	}

	readb(ht_base + 0x4c + 1) = val;
	pr_info("Set freq 0x%x\n", readl(ht_base + LS3A_HT_FREQ));

	if (((uint64_t)ht_base & 0xf000000) == 0xb000000) {
		//readl(ht_base + 0x1f4) = LS3A_HT1_SOFT_FREQ_CFG;
		//readl(ht_base + 0x1f4) = (LS3A_HT1_SOFT_FREQ_CFG | (0x1 << 25)); //pll_lock_en
		readl(ht_base + 0x1f4) = (LS3A_HT1_SOFT_FREQ_CFG | (0x3 << 27) | (0x1 << 26)); //counter_soft_cfg
	} else {
		readl(ht_base + 0x1f4) = ls7a_version() ? LS7A_HT1_SOFT_FREQ_CFG_C : LS7A_HT1_SOFT_FREQ_CFG;
	}
	pr_info("Set soft config 0x%x\n", readl(ht_base + LS3A_HT_PLL_CONF));

	if (((ht1_cfg >> 4) & 0xf) == 3) {
		readl(ht_base + 0x6c) = 0x88600000;
		pr_info( "Set GEN3 mode 0x%x\n", readl(ht_base + LS3A_HT_REVISION));
		readb(ht_base + 0x64) = 0x81;
		pr_info( "Set retry mode 0x%x\n", readl(ht_base + LS3A_HT_RETRY_CONTROL));
		readb(ht_base + 0xd0) = 0x78;
		pr_info( "Enable scrambling 0x%x\n", readl(ht_base + LS3A_HT_LINK_TRAIN));
	}

#ifdef LS7A2000
	/*set 8b10b*/
	readb(ht_base + LS3A_HT_LINK_TRAIN) |= (1 << 2);

	/*set scrambling*/
	readb(ht_base + LS3A_HT_LINK_TRAIN) |= (1 << 3);

	/*set preenmp*/
	if (((uint64_t)ht_base & 0xf000000) == 0xb000000) {
		readl(ht_base + 0x1fc) = ((readl(ht_base + 0x1fc) & 0xf0fe0fff) | 0x08004000);
	} else {
		readl(ht_base + 0x1fc) = ((readl(ht_base + 0x1fc) & 0xf0fe0fff) | 0x0c00c000);
	}

	/*set ldo*/
	if (((uint64_t)ht_base & 0xf000000) == 0xb000000) {
		readl(ht_base + 0x450) = 0x9ba78783;
	} else {
	//readb(PHYS_TO_UNCACHED(0xefdfe000000) + 0x450) = (1 << 3);
	}

	/*set scanin*/
	if (((uint64_t)ht_base & 0xf000000) == 0xb000000) {
		readl(ht_base + 0x1f8) = 0xe5af0000;
	} else {
		readl(ht_base + 0x1f8) = 0xe87f0000;
	}
#endif

	set_buff_num(ht_base);
}

int ls3a7a_ht_link_cfg(uint64_t node_id)
{
	uint64_t ls3a_ht_conf_base;
	uint64_t ls7a_ht_conf_base;
	uint32_t ht1_cfg;

	ls3a_ht_conf_base = (HT1_CTRL_CONF_BASE | (node_id <<  NODE_OFFSET));
	ls7a_ht_conf_base = (HT1_CONF_BASE | (node_id <<  NODE_OFFSET));

	ht1_cfg = ((LS7A_HT1_HARD_FREQ_CFG << 12) | (LS3A_HT1_HARD_FREQ_CFG << 8) | (HT1_GEN_CFG << 4) | (HT1_WIDTH_CFG << 1) | (HT1_RECONNECT << 0));

#if	HT1_RECONNECT
	pr_info("Set 7A side HT:\n");
	/* if HT_WIDTH_CTRL_8BIT close HIGH HT clock */
	if (!HT1_WIDTH_CFG)
		readl(((node_id <<  NODE_OFFSET) | LS7A_CONFBUS_BASE_ADDR) + CONF_HT_CLKEN_OFFSET) &= ~(1 << 1);
	ht_link_cfg(ht1_cfg, ls7a_ht_conf_base);
	pr_info("Set CPU side HT:\n");
	ht_link_cfg(ht1_cfg, ls3a_ht_conf_base);
	pr_info("config ht link done.\n");
#else
	pr_info("no reconnet.\n");
#endif
	return 0;
}

void ls3a7a_ht_cfg(char *buf, char size)
{
	uint64_t node_id;
	int i;

	for (i = 0; i < size; i++) {
		node_id = buf[i];
		if (ls3a7a_ht_link_cfg(node_id)) {
			pr_info("NODE %d error!\n", node_id);
			while(1);
		}
	}
}

/*
 * Ls7A1000 chipset Front side bus initialize entry point.
*/
int ls3a7a_ht_linkup (void)
{
	int i;
	int size = sizeof(ls7a_link_id_buf);

	/* ensure 7A HT link ok */
	for (i = 0; i < size; i++)
		while (check_ht_link(ls7a_link_id_buf[i], HT1_LO));

	pr_info( "Ls7A Hyper Transport Bridge Initialize.\n");
	ls7a_fix_addr();
	check_ls3a_ht_clk_sel();
	check_ls7a_ht_clk_sel();
	ls3a7a_ht_cfg(ls7a_link_id_buf, size);
	reset_ht_link(ls7a_link_id_buf, size);

	//watchdog_close();
	if (LS3A_NODE_NUM == 8) {
		/*enable fast path 36 and 27*/
		enable_fast_path(LS3A_NODE_NUM);
	}
	/*disable ht regs*/
	ht_reg_disable(ls7a_link_id_buf, size);
#ifndef DISABLE_X_LINK
	if (LS3A_NODE_NUM >= 4)
		enable_xlink_c(LS3A_NODE_NUM);
#endif
	v_ctrl();

	pr_info("LS3A-7A linkup.\n");

	return 0;
}
