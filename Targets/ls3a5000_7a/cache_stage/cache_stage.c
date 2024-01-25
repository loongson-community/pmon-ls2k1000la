#include <sys/types.h>
#include "pmon.h"

#include "../loongson/loongson3_def.h"
#include "target/mem_ctrl.h"
#include "ddr4/ddr4_param_debug.c"

#include "../../../pmon/arch/loongarch/early_printf.c"
#include "../../../pmon/arch/loongarch/ls7a/ht.h"
#include "../loongson/ht_link.c"
#include "../../../pmon/arch/loongarch/ls7a/ls7a_init.c"

ddr_ctrl mm_ctrl_info;
extern int ddr4_init (uint64_t node_num, ddr_ctrl *mm_ctrl);

void mm_feature_init(void)
{
	mm_ctrl_info.mc_regs_base = DDR_CFG_BASE;
	mm_ctrl_info.cache_mem_base = 0xa000000000000000;
	mm_ctrl_info.dimm_info_in_flash_offset = DIMM_INFO_IN_FLASH_OFFS;
	mm_ctrl_info.ddr_freq = DDR_FREQ;
	mm_ctrl_info.ddr_freq_2slot = DDR_FREQ_2SLOT;
	mm_ctrl_info.node_offset = NODE_OFFSET;
	mm_ctrl_info.tot_node_num = TOT_NODE_NUM;
#ifdef BONITO_100M
	mm_ctrl_info.ref_clk = 100;
#elif BONITO_25M
	mm_ctrl_info.ref_clk = 25;
#endif
	mm_ctrl_info.channel_width = 64;
	//if you want change kernel high start address you should change the macro
	mm_ctrl_info.mem_base = HIGH_MEM_WIN_BASE_ADDR;
	/* mm_ctrl is global variable */
	mm_ctrl_info.table.enable_early_printf		= 1;
	mm_ctrl_info.table.ddr_param_store		= 1;
	mm_ctrl_info.table.ddr3_dimm			= 0;
	mm_ctrl_info.table.auto_ddr_config		= 1;
	mm_ctrl_info.table.enable_ddr_leveling		= 1;
	mm_ctrl_info.table.print_ddr_leveling		= 0;
	mm_ctrl_info.table.enable_mc_vref_training	= 1;
	mm_ctrl_info.table.vref_training_debug		= 0;
	mm_ctrl_info.table.enable_ddr_vref_training	= 1;
	mm_ctrl_info.table.enable_bit_training		= 1;
	mm_ctrl_info.table.bit_training_debug		= 1;
	mm_ctrl_info.table.enable_write_training	= 1;
	mm_ctrl_info.table.debug_write_training		= 0;
	mm_ctrl_info.table.print_dll_sample		= 0;
	mm_ctrl_info.table.disable_dq_odt_training	= 1;
	mm_ctrl_info.table.lvl_debug			= 0;
	mm_ctrl_info.table.disable_dram_crc		= 1;
	mm_ctrl_info.table.two_t_mode_enable		= 0;
	mm_ctrl_info.table.disable_dimm_ecc		= 0;
	mm_ctrl_info.table.disable_read_dbi		= 1;
	mm_ctrl_info.table.disable_write_dbi		= 1;
	mm_ctrl_info.table.disable_dm			= 0;
	mm_ctrl_info.table.preamble2			= 0;
	mm_ctrl_info.table.set_by_protocol		= 1;
	mm_ctrl_info.table.param_set_from_spd_debug	= 0;
	mm_ctrl_info.table.refresh_1x			= 1;
	mm_ctrl_info.table.spd_only			= 0;
	mm_ctrl_info.table.ddr_debug_param		= 0;
	mm_ctrl_info.table.ddr_soft_clksel		= 1;
#ifdef LS_STR
	mm_ctrl_info.table.str				= 1;
#else
	mm_ctrl_info.table.str				= 0;
#endif
	mm_ctrl_info.table.pda_mode			= 1;
	mm_ctrl_info.table.signal_test			= 0;
	mm_ctrl_info.data.vref				= 0x0a01;
	mm_ctrl_info.mc_vref_adjust			= 0x0;
	mm_ctrl_info.ddr_vref_adjust			= 0x0;
	mm_ctrl_info.data.vref_init			= 0x20;
	mm_ctrl_info.data.rl_manualy			= 0;
	mm_ctrl_info.data.bit_width			= 64;
	mm_ctrl_info.data.nc16_map			= 0;
	mm_ctrl_info.data.dll_ck_mc0			= 0x44;
	mm_ctrl_info.data.dll_ck_mc1			= 0x44;
	mm_ctrl_info.data.gate_mode				= 0;

	mm_ctrl_info.param.RCD					= 0;
	mm_ctrl_info.param.RP					= 0;
	mm_ctrl_info.param.RAS					= 0;
	mm_ctrl_info.param.REF					= 0;
	mm_ctrl_info.param.RFC					= 0;

	mm_ctrl_info.ocd.pad_clk_ocd			= PAD_CLK_OCD;
	mm_ctrl_info.ocd.pad_ctrl_ocd			= PAD_CTRL_OCD;
	mm_ctrl_info.ocd.pad_ds_split			= PAD_DS_SPLIT_ALL;

	mm_ctrl_info.odt.rtt_nom_1r_1slot		= RTT_NOM;
	mm_ctrl_info.odt.rtt_park_1r_1slot		= RTT_PARK;
	mm_ctrl_info.odt.mc_dqs_odt_1cs			= MC_DQS_ODT;
	mm_ctrl_info.odt.mc_dq_odt_1cs			= MC_DQ_ODT;

	mm_ctrl_info.odt.rtt_nom_2r_1slot		= RTT_NOM_2RANK;
	mm_ctrl_info.odt.rtt_park_2r_1slot		= RTT_PARK_2RANK;

	mm_ctrl_info.odt.rtt_nom_1r_2slot_cs0		= RTT_NOM_CS0;
	mm_ctrl_info.odt.rtt_park_1r_2slot_cs0		= RTT_PARK_CS0;
	mm_ctrl_info.odt.rtt_nom_1r_2slot_cs1		= RTT_NOM_CS1;
	mm_ctrl_info.odt.rtt_park_1r_2slot_cs1		= RTT_PARK_CS1;

	mm_ctrl_info.odt.rtt_nom_2r_2slot_cs0		= RTT_NOM_2R_CS0;
	mm_ctrl_info.odt.rtt_park_2r_2slot_cs0		= RTT_PARK_2R_CS0;
	mm_ctrl_info.odt.rtt_nom_2r_2slot_cs2		= RTT_NOM_2R_CS2;
	mm_ctrl_info.odt.rtt_park_2r_2slot_cs2		= RTT_PARK_2R_CS2;

	mm_ctrl_info.odt.mc_dqs_odt_2cs			= MC_DQS_ODT_2CS;
	mm_ctrl_info.odt.mc_dq_odt_2cs			= MC_DQ_ODT_2CS;

	mm_ctrl_info.sameba_adj				= MC_PHY_REG_DATA_070;
	mm_ctrl_info.samebg_adj				= MC_PHY_REG_DATA_078;
	mm_ctrl_info.samec_adj				= MC_PHY_REG_DATA_080;
	mm_ctrl_info.samecs_adj				= MC_PHY_REG_DATA_090;
	mm_ctrl_info.diffcs_adj				= MC_PHY_REG_DATA_098;

	/* paster parameter */
	mm_ctrl_info.paster.mc0_enable			= MC0_ENABLE;
	mm_ctrl_info.paster.mc1_enable			= MC1_ENABLE;

	mm_ctrl_info.paster.mc0_memsize			= MC0_MEMSIZE;
	mm_ctrl_info.paster.mc0_dram_type		= MC0_DRAM_TYPE;
	mm_ctrl_info.paster.mc0_dimm_type		= MC0_DIMM_TYPE;
	mm_ctrl_info.paster.mc0_cid_num			= MC0_CID_NUM;
	mm_ctrl_info.paster.mc0_ba_num			= MC0_BA_NUM;
	mm_ctrl_info.paster.mc0_bg_num			= MC0_BG_NUM;
	mm_ctrl_info.paster.mc0_csmap			= MC0_CSMAP;
	mm_ctrl_info.paster.mc0_dram_width		= MC0_DRAM_WIDTH;
	mm_ctrl_info.paster.mc0_module_width		= MC0_MODULE_WIDTH;
	mm_ctrl_info.paster.mc0_ecc			= MC0_ECC;
	mm_ctrl_info.paster.mc0_sdram_capacity		= MC0_SDRAM_CAPACITY;
	mm_ctrl_info.paster.mc0_col_num			= MC0_COL_NUM;
	mm_ctrl_info.paster.mc0_row_num			= MC0_ROW_NUM;
	mm_ctrl_info.paster.mc0_addr_mirror		= MC0_ADDR_MIRROR;
	mm_ctrl_info.paster.mc0_bg_mirror		= MC0_BG_MIRROR;

	mm_ctrl_info.paster.mc1_memsize			= MC1_MEMSIZE;
	mm_ctrl_info.paster.mc1_dram_type		= MC1_DRAM_TYPE;
	mm_ctrl_info.paster.mc1_dimm_type		= MC1_DIMM_TYPE;
	mm_ctrl_info.paster.mc1_cid_num			= MC1_CID_NUM;
	mm_ctrl_info.paster.mc1_ba_num			= MC1_BA_NUM;
	mm_ctrl_info.paster.mc1_bg_num			= MC1_BG_NUM;
	mm_ctrl_info.paster.mc1_csmap			= MC1_CSMAP;
	mm_ctrl_info.paster.mc1_dram_width		= MC1_DRAM_WIDTH;
	mm_ctrl_info.paster.mc1_module_width		= MC1_MODULE_WIDTH;
	mm_ctrl_info.paster.mc1_ecc			= MC1_ECC;
	mm_ctrl_info.paster.mc1_sdram_capacity		= MC1_SDRAM_CAPACITY;
	mm_ctrl_info.paster.mc1_col_num			= MC1_COL_NUM;
	mm_ctrl_info.paster.mc1_row_num			= MC1_ROW_NUM;
	mm_ctrl_info.paster.mc1_addr_mirror		= MC1_ADDR_MIRROR;
	mm_ctrl_info.paster.mc1_bg_mirror		= MC1_BG_MIRROR;
	mm_ctrl_info.param_reg_array			= &param_info;
}

void mm_feature_init_7a(void)
{
	mm_ctrl_info.mc_regs_base = LS7A_DDR4_CFG_BASE;
	mm_ctrl_info.cache_mem_base = 0xa0000e0000000000 | TEMP_GMEM_ADDR;
	mm_ctrl_info.dimm_info_in_flash_offset = LS7A_DIMM_INFO_IN_FLASH_OFFS;
	mm_ctrl_info.node_offset = NODE_OFFSET;
	mm_ctrl_info.ddr_freq = DDR_FREQ;
	mm_ctrl_info.tot_node_num = 0;
#ifdef BONITO_100M
	mm_ctrl_info.ref_clk = 100;
#elif BONITO_25M
	mm_ctrl_info.ref_clk = 25;
#endif
	mm_ctrl_info.channel_width = 32;
	//if you want change kernel high start address you should change the macro
	mm_ctrl_info.mem_base = HIGH_MEM_WIN_BASE_ADDR;
	/* mm_ctrl is global variable */
	mm_ctrl_info.table.enable_early_printf		= 1;
	mm_ctrl_info.table.ddr_param_store		= 1;
	mm_ctrl_info.table.ddr3_dimm			= 0;
	mm_ctrl_info.table.auto_ddr_config		= 0;
	mm_ctrl_info.table.enable_ddr_leveling		= 1;
	mm_ctrl_info.table.print_ddr_leveling		= 0;
	mm_ctrl_info.table.enable_mc_vref_training	= 1;
	mm_ctrl_info.table.vref_training_debug		= 0;
	mm_ctrl_info.table.enable_ddr_vref_training	= 1;
	mm_ctrl_info.table.enable_bit_training		= 1;
	mm_ctrl_info.table.bit_training_debug		= 1;
	mm_ctrl_info.table.enable_write_training	= 1;
	mm_ctrl_info.table.debug_write_training		= 1;
	mm_ctrl_info.table.print_dll_sample		= 0;
	mm_ctrl_info.table.disable_dq_odt_training	= 1;
	mm_ctrl_info.table.lvl_debug			= 0;
	mm_ctrl_info.table.disable_dram_crc		= 1;
	mm_ctrl_info.table.two_t_mode_enable		= 0;
	mm_ctrl_info.table.disable_dimm_ecc		= 0;
	mm_ctrl_info.table.disable_read_dbi		= 1;
	mm_ctrl_info.table.disable_write_dbi		= 1;
	mm_ctrl_info.table.disable_dm			= 0;
	mm_ctrl_info.table.set_by_protocol		= 1;
	mm_ctrl_info.table.param_set_from_spd_debug	= 0;
	mm_ctrl_info.table.refresh_1x			= 1;
	mm_ctrl_info.table.spd_only			= 0;
	mm_ctrl_info.table.ddr_debug_param		= 0;
	mm_ctrl_info.table.ddr_soft_clksel		= 1;
	mm_ctrl_info.table.str				= 0;
	mm_ctrl_info.table.pda_mode			= 1;
	mm_ctrl_info.table.signal_test			= 0;
	mm_ctrl_info.data.vref				= 0x0a01;
	mm_ctrl_info.data.vref_init			= 0x20;
	mm_ctrl_info.data.rl_manualy			= 0;
	mm_ctrl_info.data.bit_width			= 32;
	mm_ctrl_info.data.nc16_map			= 0;
	mm_ctrl_info.data.dll_ck_mc0			= 0x44;
	mm_ctrl_info.data.dll_ck_mc1			= 0x44;

	mm_ctrl_info.ocd.pad_clk_ocd			= PAD_CLK_OCD;
	mm_ctrl_info.ocd.pad_ctrl_ocd			= PAD_CTRL_OCD;
	mm_ctrl_info.ocd.pad_ds_split			= PAD_DS_SPLIT_ALL;

	mm_ctrl_info.odt.rtt_nom_1r_1slot		= RTT_NOM_7A;
	mm_ctrl_info.odt.rtt_park_1r_1slot		= RTT_PARK_7A;
	mm_ctrl_info.odt.mc_dqs_odt_1cs			= MC_DQS_ODT;
	mm_ctrl_info.odt.mc_dq_odt_1cs			= MC_DQ_ODT;

	mm_ctrl_info.odt.rtt_nom_2r_1slot		= RTT_NOM_2RANK;
	mm_ctrl_info.odt.rtt_park_2r_1slot		= RTT_PARK_2RANK;

	mm_ctrl_info.odt.rtt_nom_1r_2slot_cs0		= RTT_NOM_CS0;
	mm_ctrl_info.odt.rtt_park_1r_2slot_cs0		= RTT_PARK_CS0;
	mm_ctrl_info.odt.rtt_nom_1r_2slot_cs1		= RTT_NOM_CS1;
	mm_ctrl_info.odt.rtt_park_1r_2slot_cs1		= RTT_PARK_CS1;

	mm_ctrl_info.odt.rtt_nom_2r_2slot_cs0		= RTT_NOM_2R_CS0;
	mm_ctrl_info.odt.rtt_park_2r_2slot_cs0		= RTT_PARK_2R_CS0;
	mm_ctrl_info.odt.rtt_nom_2r_2slot_cs2		= RTT_NOM_2R_CS2;
	mm_ctrl_info.odt.rtt_park_2r_2slot_cs2		= RTT_PARK_2R_CS2;

	mm_ctrl_info.odt.mc_dqs_odt_2cs			= MC_DQS_ODT_2CS;
	mm_ctrl_info.odt.mc_dq_odt_2cs			= MC_DQ_ODT_2CS;

	mm_ctrl_info.sameba_adj				= MC_PHY_REG_DATA_070;
	mm_ctrl_info.samebg_adj				= MC_PHY_REG_DATA_078;
	mm_ctrl_info.samec_adj				= MC_PHY_REG_DATA_080;
	mm_ctrl_info.samecs_adj				= MC_PHY_REG_DATA_090;
	mm_ctrl_info.diffcs_adj				= MC_PHY_REG_DATA_098;

	/* paster parameter */
	mm_ctrl_info.paster.mc0_enable			= 1;
	mm_ctrl_info.paster.mc1_enable			= 0;

	mm_ctrl_info.paster.mc0_memsize			= GMEM_MEMSIZE;
	mm_ctrl_info.paster.mc0_dram_type		= GMEM_DRAM_TYPE;
	mm_ctrl_info.paster.mc0_dimm_type		= GMEM_DIMM_TYPE;
	mm_ctrl_info.paster.mc0_cid_num			= GMEM_CID_NUM;
	mm_ctrl_info.paster.mc0_ba_num			= GMEM_BA_NUM;
	mm_ctrl_info.paster.mc0_bg_num			= GMEM_BG_NUM;
	mm_ctrl_info.paster.mc0_csmap			= GMEM_CSMAP;
	mm_ctrl_info.paster.mc0_dram_width		= GMEM_DRAM_WIDTH;
	mm_ctrl_info.paster.mc0_module_width		= GMEM_MODULE_WIDTH;
	mm_ctrl_info.paster.mc0_ecc			= GMEM_ECC;
	mm_ctrl_info.paster.mc0_sdram_capacity		= GMEM_SDRAM_CAPACITY;
	mm_ctrl_info.paster.mc0_col_num			= GMEM_COL_NUM;
	mm_ctrl_info.paster.mc0_row_num			= GMEM_ROW_NUM;
	mm_ctrl_info.paster.mc0_addr_mirror		= GMEM_ADDR_MIRROR;
	mm_ctrl_info.paster.mc0_bg_mirror		= 0;
}

void cache_stage()
{
	pr_info("run in cache!\n");
	cpu_ht_link();
	ls7a_feature_init();
	ls3a7a_ht_linkup();

	mm_feature_init();
	ddr4_init(TOT_NODE_NUM, &mm_ctrl_info);
	pr_info("mem init done!\n");
	startup_cpu();
	ls7a_init();
	pr_info("ls7a init done!\n");
	if (((readl(LS7A_ACPI_PM1_CNT_REG) >> 10) & 0x7) == SLEEP_TYPE_S3) {
		realinit_loongarch();
	} else {
		init_loongarch();
	}
}
