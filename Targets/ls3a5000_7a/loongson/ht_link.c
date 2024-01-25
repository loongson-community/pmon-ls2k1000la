#include "target/ls7a_config.h"

#define HT0_RECONNECT
//#define DISABLE_X_LINK
//#define INTERCONNECTION_HT1
#define HT0_16BIT //NO 16Bit mode when HT1.0 //hardware has some error need fixup.

//!!DO NOT CHANGE BELOW!!
#ifdef BONITO_100M
#if    (HT0_FREQ <= 400)
#define HT0_PHY_DIV		12
#elif (HT0_FREQ <= 800)
#define HT0_PHY_DIV		8
#elif (HT0_FREQ <= 1600)
#define HT0_PHY_DIV		4
#else
#define HT0_PHY_DIV		2
#endif

#define HT0_DIV_REFC		2
#elif  BONITO_25M
#if    (HT0_FREQ <= 600)
#define HT0_PHY_DIV		4
#elif (HT0_FREQ <= 1200)
#define HT0_PHY_DIV		2
#else
#define HT0_PHY_DIV		1
#endif

#define HT0_DIV_REFC		1
#else
#define HT0_PHY_DIV		"undefined"
#define HT0_DIV_REFC		"undefined"
#endif

#if    (HT0_FREQ == 1600)
#define HT_HARD_FREQ		((chip_ver() == 0x43) ? HT_HARD_1600M : HT_HARD_200M)
#elif (HT0_FREQ == 3200)
#define HT_HARD_FREQ		((chip_ver() == 0x43) ? HT_HARD_3200M : HT_HARD_400M)
#define HT_8B10B
#endif

#ifdef BONITO_100M
#define SYS_CLOCK		100     //MUST BE 100 or 25, depend on the osillator
#elif BONITO_25M
#define SYS_CLOCK		25      //MUST BE 100 or 25, depend on the osillator
/* SYS_CLOCK IS NOT GOOD FOR HIGH FREQUENCY */
#endif
#define HT0_PHY_LO_DIV		HT0_PHY_DIV
#define HT0_PHY_HI_DIV		HT0_PHY_DIV
#define HT0_DIV_LOOPC		(HT0_DIV_REFC*HT0_FREQ*HT0_PHY_DIV/SYS_CLOCK)
#define HT0_CORE_DIV		8

#ifdef  INTERCONNECTION_HT1
#define HT0_PLL_CONFIG		0x00464083
#else
//#define HT0_PLL_CONFIG		((HT0_PHY_LO_DIV << 4) | (HT0_PHY_HI_DIV << 8) | (HT0_DIV_REFC << 12) | (HT0_DIV_LOOPC << 16) | 0x3)
#define HT0_PLL_CONFIG		((HT0_PHY_LO_DIV << 4) | (HT0_PHY_HI_DIV << 8) | (HT0_DIV_REFC << 12) | (HT0_DIV_LOOPC << 16) | 0x2)
#endif

#define HT0_LO			0xaULL
#define HT0_HI			0xbULL
#define HT1_LO			0xeULL
#define HT1_HI			0xfULL

#define HT_HARD_3200M		0xf
#define HT_HARD_2400M		0xd
#define HT_HARD_2200M		0xc
#define HT_HARD_2000M		0xb
#define HT_HARD_1800M		0xa
#define HT_HARD_1600M		0x9
#define HT_HARD_1200M		0x9
#define HT_HARD_800M		0x5
#define HT_HARD_400M		0x2
#define HT_HARD_200M		0x0

#define HT_LINK_8B		0
#define HT_LINK_16B		1

extern void loop_delay(uint64_t loops);

void shutdown_cpu(void)
{
	uint64_t i;
	pr_info("Shut down CPU\n");
	for (i = 1; i < TOT_NODE_NUM; i++) {
		readl(PHYS_TO_UNCACHED(0x1fe001d0) | (i << NODE_OFFSET)) = 0;
	}
}

void startup_cpu(void)
{
	uint64_t i;
	pr_info("start up other CPU\n");
	for (i = 0; i < TOT_NODE_NUM; i++) {
		readl(PHYS_TO_UNCACHED(0x1fe00420) | (i << NODE_OFFSET)) |= (1 << 23);
		readl(PHYS_TO_UNCACHED(0x1fe001d0) | (i << NODE_OFFSET)) = 0xffffffff;
	}
}

int ht_freq_flag(void)
{
	return !(readl(PHYS_TO_UNCACHED(0x1fe00194)) & 0x8000);
}

int check_ht_crc(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	if (readl(base + 0x44) & 0x300)
		return -1;
	return 0;
}

void set_ch_interleave(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1d8) |= 0xc0000000;
}

void train_time(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1e4) = 0x1ff;
}

void set_soft_freq(uint64_t node, uint64_t ht_num, int pll_config)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + LS3A_HT_PLL_CONF) = pll_config;
}

void set_hard_freq(uint64_t node, uint64_t ht_num, int pll_config)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readb(base + LS3A_HT_FREQ + 0x1) = pll_config;
}

void set_gen3_mode(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + LS3A_HT_REVISION) = 0x88600000;
}

void set_retry_mode(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readb(base + LS3A_HT_RETRY_CONTROL) = 0x81;
}

void set_scrambling(uint64_t node, uint64_t ht_num)
{
	if(chip_ver() == 0x43) {
		//pr_info("Enable HyperTransport Controller scrambling\r\n");
		uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
		readb(base + LS3A_HT_LINK_TRAIN) |= (1 << 3);
	}
}

void set_8b10b(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readb(base + LS3A_HT_LINK_TRAIN) |= (1 << 2);
}

void set_coherent_mode(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1c0) |= (1 << 22);
}

void set_ht_link_8b_16b(uint64_t node, uint64_t ht_num, uint32_t mode)
{
	uint64_t val;
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	val = readl(base + 0x44);
	val = (val & 0x88ffffff) | (mode << 24) | (mode << 28);
	readl(base + 0x44) = val;
}

void pll_lock_en(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1f4) |= (0x1 << 25);
}

void counter_soft_cfg(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1f4) |= ((0x3 << 27) | (0x1 << 26));
	//readl(base + 0x1f4) |= ((0x1f << 27) | (0x1 << 26));
}

void set_tx_preenmp(uint64_t node, uint64_t ht_num, uint32_t val)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1fc) = ((readl(base + 0x1fc) & 0xf0fe0fff) | val);
}

void set_scanin(uint64_t node, uint64_t ht_num, uint32_t val)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1f8) = val;
}

void set_ldo(uint64_t node, uint64_t ht_num, uint32_t val)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x450) = val;
}

int get_x_link_node_id(uint64_t node_id)
{
	uint64_t ret;

	ret = (node_id % 4);

	switch (ret) {
	case 0:
		ret = node_id + 3;
		break;
	case 1:
		ret = node_id + 1;
		break;
	case 2:
		ret = node_id - 1 ;
		break;
	case 3:
		ret = node_id - 3;
		break;
	}

	return ret;
}

#if 0
void check_node_acc(void)
{
	int i;
	uint32_t val;
	for (i = 0;i < TOT_NODE_NUM; i++) {
		val = readl(PHYS_TO_UNCACHED(0x1fe00000) | ((uint64_t)i << 44));
		//pr_info("node%d ok!\n",i);
	}
}
#endif

void reset_ht(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readb(base + 0x3e) |= 0x40;
	//pr_info("Reset HT bus 0x%x\n", readl(base + 0x3c));
}

void dereset_ht(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readb(base + 0x3e) &= ~0x40;
	//pr_info("Reset HT bus 0x%x\n", readl(base + 0x3c));
}

void ldt_reset_ht(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1c0) &= ~(1 << 29);
	readl(base + 0x1c0) |= (1 << 29);
}

void ldt_dereset_ht(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x1c0) &= ~(1 << 28);
	readl(base + 0x1c0) |= (1 << 28);
}

void wait_ht_down(uint64_t node, uint64_t ht_num)
{
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	while (readl(base + 0x44) & 0x20);
	//pr_info("link down!\n");
}

#if 0
void check_ht_nocrc(uint64_t node, uint64_t ht_num)
{
	int i;
	uint64_t val;
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	for (i = 0; i < 0x1000; i++) {
		val = readl(base + 0x44);
		if (val & 0x20) {
			pr_info("%x\n",readl(base + 0x44));
			readl(base + 0x44) = 0;
			reset_ht(node, ht_num);
			dereset_ht(node, ht_num);
			pr_info("%x\n",readl(base + 0x44));
			return;
		}
		if (i == (0x1000 - 1)) {
			reset_ht(node, ht_num);
			readl(base + 0x44) = 0;
			dereset_ht(node, ht_num);
			i = 0;
		}
	}
}
#endif

int check_ht_link(uint64_t node, uint64_t ht_num)
{
	int i;
	uint64_t val;
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	readl(base + 0x44) &= ~(3 << 8);
	for (i = 0; i < 0x1000; i++) {
		val = readl(base + 0x44);
		if (val & 0x300) {
			reset_ht(node, ht_num);
			/* this print as a delay function */
			pr_info("CRC ERROR\n");
			dereset_ht(node, ht_num);
			i = 0;
			continue;
		}
		if (val & 0x20) {
			pr_info("%x\n",readl(base + 0x44));
			return 0;
		}
	}
	return 1;
}

int ht_wait_link_status(uint64_t node, uint64_t ht_num, int bit)
{
	uint32_t val;
	uint64_t base = (PHYS_TO_UNCACHED(0x00fdfb000000) | node << NODE_OFFSET | ht_num << 40);
	int i;
	for (i = 0; i < 0x200; i++) {
		pr_info(">");
		val = readl(base + 0x44);
		if ((val & 0x20) ^ bit) {
			/* check HT crc error */
			val = readl(base + 0x68);
			loop_delay(0x20000);
			/* don't optimize the register reading */
			if ((readl(base + 0x68) & 0xffff) || (val != readl(base + 0x68))) {
				pr_info("%x\n", readl(base + 0x68));
				readl(base + 0x68) = 0;
				return -1;
			}
			pr_info("%x\n", readl(base + 0x44));
				return 0;
		}
		if (!((i + 1) % 32))
			pr_info("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b=");
	}
	return -1;
}

int  ht_wait_link_up(uint64_t node, uint64_t ht)
{
	return ht_wait_link_status(node, ht, 0);
}

int  ht_wait_link_down(uint64_t node, uint64_t ht)
{
	return ht_wait_link_status(node, ht, 0x20);
}

void ht_link_reset(uint64_t node, uint64_t ht, uint64_t dst_node, uint64_t dst_ht)
{
	do {
		reset_ht(node, ht);
		dereset_ht(node, ht);
		check_ht_link(node, ht);
		pr_info("Waiting node%d %x HyperTransport bus to be up.\n", node, ht);
		if (ht_wait_link_up(node, ht) < 0)
			continue;
		pr_info("Waiting node%d %x HyperTransport bus to be up.\n", dst_node, dst_ht);
		if (ht_wait_link_up(dst_node, dst_ht) < 0)
			continue;
		break;
	} while (1);
}

int reset_ht_x_link(const char *buf, const int size)
{
	const char *ptr = buf;
	int count = size;
	uint64_t node_id, dst_node_id;

	for (node_id = 0; node_id < TOT_NODE_NUM; node_id++) {
		if ((node_id % 4) >= 2)
			continue;
		dst_node_id = get_x_link_node_id(node_id);
		if (((node_id == *ptr) || (dst_node_id == *ptr)) && count) {
			//pr_info("-- x link -- %d -> %d\n", node_id, *ptr);
			ptr++;
			count--;
			if (node_id < dst_node_id)
				dst_node_id = node_id;
			pr_info("Reset node%d HT1-HI bus\n", dst_node_id);
			reset_ht(dst_node_id, HT1_HI);
			pr_info("Wait HT bus down...\n");
			wait_ht_down(dst_node_id, HT1_HI);
		} else {
			ht_link_reset(node_id, HT1_HI, dst_node_id, HT1_HI);
		}
	}
	return 0;
}

void reset_useless_ht1_lo(const char *buf, const int size, char link_offset)
{
	const char *ptr = buf;
	int count = size;
	uint64_t node_id;

	for (node_id = 0; node_id < TOT_NODE_NUM; node_id += link_offset) {
		if ((node_id == *ptr) && count) {
			ptr++;
			count--;
			continue;
		}
		pr_info("Reset node%d HT1-LO bus\n", node_id);
		reset_ht(node_id, HT1_LO);
	}
}

void chip_link_conf(const char *buf, int link_num)
{
	uint64_t i;
	uint32_t  val;
	char conf_flag;
	char freq_flag = ht_freq_flag();
	const char *ptr = buf;

	pr_info("HT configuration!\r\n");
	for (i = 0; i < TOT_NODE_NUM; i++) {
		conf_flag = 1;
		if ((i == *ptr) && link_num) {
			ptr++;
			link_num--;
			conf_flag = 0;
			pr_info("- conf flag - %d\n", i);
		}
		set_ch_interleave(i, HT0_LO);
		set_ch_interleave(i, HT0_HI);
		if (conf_flag)
			set_ch_interleave(i, HT1_LO);
#ifndef DISABLE_X_LINK
		set_ch_interleave(i, HT1_HI);
#endif

#ifdef HT0_16BIT
		set_ht_link_8b_16b(i, HT0_LO, HT_LINK_16B);
		set_ht_link_8b_16b(i, HT0_HI, HT_LINK_16B);
#else
		set_ht_link_8b_16b(i, HT0_LO, HT_LINK_8B);
		set_ht_link_8b_16b(i, HT0_HI, HT_LINK_8B);
#endif
		if (conf_flag)
			set_ht_link_8b_16b(i, HT1_LO, HT_LINK_8B);
#ifndef DISABLE_X_LINK
		set_ht_link_8b_16b(i, HT1_HI, HT_LINK_8B);
#endif

		set_coherent_mode(i, HT0_LO);
		set_coherent_mode(i, HT0_HI);
		if (conf_flag)
			set_coherent_mode(i, HT1_LO);
#ifndef DISABLE_X_LINK
		set_coherent_mode(i, HT1_HI);
#endif

#ifdef HT0_RECONNECT
		if (freq_flag) {
			set_soft_freq(i, HT0_LO, HT0_PLL_CONFIG);
			set_soft_freq(i, HT1_LO, HT0_PLL_CONFIG);
		} else {
			set_hard_freq(i, HT0_LO, HT_HARD_FREQ);
			set_hard_freq(i, HT0_HI, HT_HARD_FREQ);
			set_hard_freq(i, HT1_LO, HT_HARD_FREQ);
			set_hard_freq(i, HT1_HI, HT_HARD_FREQ);
		}

		set_gen3_mode(i, HT0_LO);
		set_gen3_mode(i, HT0_HI);
		if (conf_flag)
			set_gen3_mode(i, HT1_LO);
#ifndef DISABLE_X_LINK
		set_gen3_mode(i, HT1_HI);
#endif

		set_retry_mode(i, HT0_LO);
		set_retry_mode(i, HT0_HI);
		if (conf_flag)
			set_retry_mode(i, HT1_LO);
#ifndef DISABLE_X_LINK
		set_retry_mode(i, HT1_HI);
#endif
#ifdef HT_8B10B
		set_8b10b(i, HT0_LO);
		set_8b10b(i, HT0_HI);
		if (conf_flag)
			set_8b10b(i, HT1_LO);
#ifndef DISABLE_X_LINK
		set_8b10b(i, HT1_HI);
#endif
#endif
		set_scrambling(i, HT0_LO);
		set_scrambling(i, HT0_HI);
		set_scrambling(i, HT1_LO);
#ifndef DISABLE_X_LINK
		set_scrambling(i, HT1_HI);
#endif
		train_time(i, HT0_LO);
		train_time(i, HT0_HI);
		train_time(i, HT1_LO);
#ifndef DISABLE_X_LINK
		train_time(i, HT1_HI);
#endif

		val = 0x08004000;
		set_tx_preenmp(i, HT0_LO, val);
		set_tx_preenmp(i, HT0_HI, val);
		val = 0x0c002000;
		set_tx_preenmp(i, HT1_LO, val);
		if(((i % 4) == 0) || ((i % 4) == 3)) {
			val = 0x0d004000;
		} else {
			val = 0x08004000;
		}
		set_tx_preenmp(i, HT1_HI, val);

		val = 0x9ba78783;
		set_ldo(i, HT0_LO, val);
		set_ldo(i, HT0_HI, val);
		set_ldo(i, HT1_LO, val);
		set_ldo(i, HT1_HI, val);

		val = 0xe5af0000;
		set_scanin(i, HT0_LO, val);
		set_scanin(i, HT0_HI, val);
		set_scanin(i, HT1_LO, val);
		set_scanin(i, HT1_HI, val);
#if 1
		/*pll lock en*/
		pll_lock_en(i, HT0_LO);
		pll_lock_en(i, HT0_HI);
		pll_lock_en(i, HT1_LO);
		pll_lock_en(i, HT1_HI);
#else
		/*counter soft cfg*/
		counter_soft_cfg(i, HT0_LO);
		counter_soft_cfg(i, HT0_HI);
		counter_soft_cfg(i, HT1_LO);
		counter_soft_cfg(i, HT1_HI);
#endif

#endif
	}
	pr_info("HT configure done!\r\n");
}

void ht_2w_link(const char *buf,const int size)
{
	uint32_t  val;

	pr_info("Setting HT b interleave and nop interleave\r\n");
	set_ch_interleave(0, HT0_LO);
	set_ch_interleave(1, HT0_LO);
#ifdef HT0_16BIT
	pr_info("setting ht link to 16 bit mode\r\n");
	set_ht_link_8b_16b(0, HT0_LO, HT_LINK_16B);
	set_ht_link_8b_16b(1, HT0_LO, HT_LINK_16B);
#else
	pr_info("setting ht link to 8 bit mode\r\n");
	set_ht_link_8b_16b(0, HT0_LO, HT_LINK_8B);
	set_ht_link_8b_16b(1, HT0_LO, HT_LINK_8B);
#endif

	set_coherent_mode(0, HT0_LO);
	set_coherent_mode(1, HT0_LO);

#ifdef HT0_RECONNECT
	if (ht_freq_flag()) {
		pr_info("Setting HyperTransport bus frequency by SOFT config\r\n");
		set_soft_freq(0, HT0_LO, HT0_PLL_CONFIG);
		set_soft_freq(1, HT0_LO, HT0_PLL_CONFIG);
	} else {
		pr_info("Setting HyperTransport bus frequency by HARD config\r\n");
		set_hard_freq(0, HT0_LO, HT_HARD_FREQ);
		set_hard_freq(1, HT0_LO, HT_HARD_FREQ);
	}
#ifdef INTERCONNECTION_HT1
	pr_info("Enable HyperTransport Controller GEN1 mode\r\n");

#else
	pr_info("Enable HyperTransport Controller GEN3 mode\r\n");
	set_gen3_mode(0, HT0_LO);
	set_gen3_mode(1, HT0_LO);

	pr_info("Setting HyperTransport Controller retry mode\r\n");
	set_retry_mode(0, HT0_LO);
	set_retry_mode(1, HT0_LO);

#ifdef HT_8B10B
	pr_info("Enable HyperTransport Controller 8B/10B\r\n");
	set_8b10b(0, HT0_LO);
	set_8b10b(1, HT0_LO);
#endif
	set_scrambling(0, HT0_LO);
	set_scrambling(1, HT0_LO);

	val = 0x08004000;
	set_tx_preenmp(0, HT0_LO, val);
	set_tx_preenmp(1, HT0_LO, val);

	val = 0x9ba78783;
	set_ldo(0, HT0_LO, val);
	set_ldo(1, HT0_HI, val);

	val = 0xe5af0000;
	set_scanin(0, HT0_LO, val);
	set_scanin(1, HT0_HI, val);

	pll_lock_en(0, HT0_LO);
	pll_lock_en(1, HT0_LO);

#endif //HT 3.0
	/* CPU to CPU connect */
	ht_link_reset(0, HT0_LO, 1, HT0_LO);

	/* reset HT1_LO */
	reset_useless_ht1_lo(buf, size, 1);
#endif
}

void ht_4w_link_reset(const char *buf,const int size)
{
	do {
		/*4 way board HT0_LO hardware connected so only reset node 0 ht0_lo */
		reset_ht(0, HT0_LO);
		dereset_ht(0, HT0_LO);
		check_ht_link(0, HT0_LO);
		pr_info("Waiting node0 %x HyperTransport bus to be up.\n", HT0_LO);
		if (ht_wait_link_up(0, HT0_LO) < 0)
			continue;
		pr_info("Waiting node1 %x HyperTransport bus to be up.\n", HT0_LO);
		if (ht_wait_link_up(1, HT0_LO) < 0)
			continue;
		pr_info("Waiting node0 %x HyperTransport bus to be up.\n", HT0_HI);
		if (ht_wait_link_up(0, HT0_HI) < 0)
			continue;
		pr_info("Waiting node1 %x HyperTransport bus to be up.\n", HT0_HI);
		if (ht_wait_link_up(1, HT0_HI) < 0)
			continue;
		pr_info("Waiting node2 %x HyperTransport bus to be up.\n", HT0_LO);
		if (ht_wait_link_up(2, HT0_LO) < 0)
			continue;
		pr_info("Waiting node2 %x HyperTransport bus to be up.\n", HT0_HI);
		if (ht_wait_link_up(2, HT0_HI) < 0)
			continue;
		pr_info("Waiting node3 %x HyperTransport bus to be up.\n", HT0_LO);
		if (ht_wait_link_up(3, HT0_LO) < 0)
			continue;
		pr_info("Waiting node3 %x HyperTransport bus to be up.\n", HT0_HI);
		if (ht_wait_link_up(3, HT0_HI) < 0)
			continue;
		break;
	} while (1);

#ifndef DISABLE_X_LINK
	reset_ht_x_link(buf, size);
#endif
	/* reset 2 3 HT1_LO */
	reset_useless_ht1_lo(buf, size, 1);
}

void ht_3c5000l_link_reset(const char *buf,const int size)
{
#ifdef HT0_RECONNECT
	uint64_t node_id;
	for (node_id = 0; node_id < TOT_NODE_NUM; node_id += 4) {
		/*reset and dereset x between 0 and 3*/
		pr_info("reset and dereset x between %d and %d\n", node_id, node_id + 3);
		reset_ht(node_id, HT1_HI);
		wait_ht_down(node_id, HT1_HI);
		dereset_ht(node_id, HT1_HI);
		ht_wait_link_up(node_id, HT1_HI);

		/*enable x between 0 and 3*/
		pr_info("enable x between %d and %d\n", node_id, node_id + 3);
		readq(PHYS_TO_UNCACHED(0x1fe00400) | ((node_id + 0)  << NODE_OFFSET)) |= (0x1ULL << 8);
		readq(PHYS_TO_UNCACHED(0x1fe00400) | ((node_id + 3)  << NODE_OFFSET)) |= (0x1ULL << 8);

		pr_info("reset node %d ring ht hi\n", node_id + 3);
		reset_ht(node_id + 3, HT0_HI);
		pr_info("reset node %d ring ht lo\n", node_id + 3);
		reset_ht(node_id + 3, HT0_LO);
		pr_info("reset node %d ring ht hi\n", node_id + 0);
		reset_ht(node_id, HT0_HI);
		pr_info("reset node %d ring ht lo\n", node_id + 0);
		reset_ht(node_id, HT0_LO);

		pr_info("wait ring ht down\n");
		ht_wait_link_down(node_id + 3, HT0_LO);
		ht_wait_link_down(node_id + 3, HT0_HI);
		ht_wait_link_down(node_id, HT0_LO);
		ht_wait_link_down(node_id, HT0_HI);

		pr_info("dereset ring ht\n");
		dereset_ht(node_id + 3, HT0_LO);
		dereset_ht(node_id + 3, HT0_HI);
		dereset_ht(node_id, HT0_LO);
		dereset_ht(node_id, HT0_HI);

		pr_info("Waiting node%d HT0_LO bus to be up.\n", node_id);
		ht_wait_link_up(node_id, HT0_LO);
		pr_info("Waiting node%d HT0_HI bus to be up.\n", node_id);
		ht_wait_link_up(node_id, HT0_HI);
		pr_info("Waiting node%d HT0_LO bus to be up.\n", node_id + 1);
		ht_wait_link_up(node_id + 1, HT0_LO);
		pr_info("Waiting node%d HT0_HI bus to be up.\n", node_id + 1);
		ht_wait_link_up(node_id + 1, HT0_HI);
		pr_info("Waiting node%d HT0_LO bus to be up.\n", node_id + 2);
		ht_wait_link_up(node_id + 2, HT0_LO);
		pr_info("Waiting node%d HT0_HI bus to be up.\n", node_id + 2);
		ht_wait_link_up(node_id + 2, HT0_HI);
		pr_info("Waiting node%d HT0_LO bus to be up.\n", node_id + 3);
		ht_wait_link_up(node_id + 3, HT0_LO);
		pr_info("Waiting node%d HT0_HI bus to be up.\n", node_id + 3);
		ht_wait_link_up(node_id + 3, HT0_HI);

		/*disable x between 0 and 3*/
		readq(PHYS_TO_UNCACHED(0x1fe00400) | (node_id << NODE_OFFSET)) &= ~(0x1ULL << 8);
		readq(PHYS_TO_UNCACHED(0x1fe00400) | ((node_id + 3) << NODE_OFFSET)) &= ~(0x1ULL << 8);

		/*reset x link*/
		reset_ht(node_id, HT1_HI);
		wait_ht_down(node_id, HT1_HI);
		reset_ht(node_id + 1, HT1_HI);
		wait_ht_down(node_id + 1, HT1_HI);
	}

#if (TOT_NODE_NUM == 8)
	/*between chip 0 and chip 1*/
	ht_link_reset(1, HT1_LO, 4, HT1_LO);
	ht_link_reset(2, HT1_LO, 7, HT1_LO);
	ht_link_reset(3, HT1_LO, 6, HT1_LO);

#elif	(TOT_NODE_NUM == 16)
	/* each CHIP connect */
	ht_link_reset( 6, HT1_LO,  9, HT1_LO);
	ht_link_reset( 7, HT1_LO, 13, HT1_LO);
	ht_link_reset(11, HT1_LO, 14, HT1_LO);
	ht_link_reset( 1, HT1_LO,  4, HT1_LO);
	ht_link_reset( 2, HT1_LO,  8, HT1_LO);
	ht_link_reset( 3, HT1_LO, 12, HT1_LO);
#endif

	reset_useless_ht1_lo(buf, size, 5);
#endif
}

void cpu_ht_link()
{
#if (TOT_NODE_NUM >= 2)
	int size = sizeof(ls7a_link_id_buf);
#if (TOT_NODE_NUM == 2)
	ht_2w_link(ls7a_link_id_buf, size);
#elif	(TOT_NODE_NUM == 4)
	/* 7A linked on chip 0 and chip 5 HT configuration without it */
	chip_link_conf(ls7a_link_id_buf, size);
	ht_4w_link_reset(ls7a_link_id_buf, size);
#elif	(TOT_NODE_NUM >= 8)
	chip_link_conf(ls7a_link_id_buf, size);
	ht_3c5000l_link_reset(ls7a_link_id_buf, size);
#endif
#endif
}
