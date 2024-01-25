#include "target/ls7a_config.h"
#include "ht.h"
#include "gmemparam.h"

#define DDR_tRDDATA_ADD (readb(mc_reg_base + tRDDATA_OFFSET) = readb(mc_reg_base + tRDDATA_OFFSET)+1)
#define DDR_tRDDATA_SUB (readb(mc_reg_base + tRDDATA_OFFSET) = readb(mc_reg_base + tRDDATA_OFFSET)-1)

uint8_t uDimmOrder[9]={0,1,2,3,4,5,6,7,8};
uint8_t rDimmOrder[9]={8,3,2,1,0,4,5,6,7};

void wait_init_done(uint64_t mc_reg_base)
{
	uint8_t val;
	do{
		val = *(volatile uint8_t *)(mc_reg_base + DDR3_DRAM_INIT_OFFSET);
	} while (val == 0);
}

void dll_wr_data_set(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	val = readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20);
	if(val >= DLL_WRDQ_SUB) {
		val -= DLL_WRDQ_SUB;
	} else {
		val = val + 0x80 - DLL_WRDQ_SUB;
	}
	readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET + slice_num * 0x20) = val;
}

void wr_dq_half_set(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	val = readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET + slice_num * 0x20);
	//if(slice_num==0) pr_info(EFI_D_INFO,"DllWrdq=: %x\n", val);
	if(val >= WRDQ_LT_HALF_STD) {
		val = 0;
	} else {
		val = 1;
	}
	//if(slice_num==0) pr_info(EFI_D_INFO,"wrdq_lt_half=: %x\n", val);

	readb(mc_reg_base + WRDQ_LT_HALF_OFFSET + slice_num * 0x20) = val;
}

void wr_dqs_half_set(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	val = readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20);
	if(val >= WRDQS_LT_HALF_STD) {
		val = 0;
	} else {
		val = 1;
	}

	readb(mc_reg_base + WRDQS_LT_HALF_OFFSET + slice_num * 0x20) = val;
}

void wlvl_get0(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	uint8_t i = 0;
	/*first set to 0x1*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
	/*then set to 0x0*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
	do {
	  val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
	} while (val == 0);

	while((readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num)) & WLVL_CHECK_BIT) {
		val = readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20);
		val = (val + 1) & 0x7f;
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = val;
#ifdef DQ_CHANGE_WITH_DQS
		dll_wr_data_set(mc_reg_base, slice_num);
		wr_dq_half_set(mc_reg_base, slice_num);
		wr_dqs_half_set(mc_reg_base, slice_num);
#endif
		/*first set to 0x1*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
		/*then set to 0x0*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

		//pr_info(EFI_D_INFO,"write leveling: slice %d searching 0\r\n", slice_num);

		do {
			val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
		} while (val == 0);
	}

	/*filter the 0 to 1 glitch*/
	while(i < 0x38) {
		/*first set to 0x1*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
		/*then set to 0x0*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
		do {
			val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
		} while (val == 0);

		while((readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num)) & WLVL_CHECK_BIT) {
			val = readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20);
			val = (val + 1) & 0x7f;
			readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = val;
#ifdef DQ_CHANGE_WITH_DQS
			dll_wr_data_set(mc_reg_base, slice_num);
			wr_dq_half_set(mc_reg_base, slice_num);
			wr_dqs_half_set(mc_reg_base, slice_num);
#endif
			/*first set to 0x1*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
			/*then set to 0x0*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
			do {
				val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
			} while (val == 0);

			i = 0;
		}
		i++;
	}
}

void wlvl_get1(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	uint8_t i = 0;
	/*first set to 0x1*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
	/*then set to 0x0*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
	do {
		val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
	} while (val == 0);

	while(!((readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num)) & WLVL_CHECK_BIT)) {
		val = readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20);
		val = (val + 1) & 0x7f;
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = val;
#ifdef DQ_CHANGE_WITH_DQS
		dll_wr_data_set(mc_reg_base, slice_num);
		wr_dq_half_set(mc_reg_base, slice_num);
		wr_dqs_half_set(mc_reg_base, slice_num);
#endif
		/*first set to 0x1*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
		/*then set to 0x0*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

		//pr_info(EFI_D_INFO,"write leveling: slice %d searching 1\r\n", slice_num);
		do {
			val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
		} while (val == 0);
	}

	/*filter the 1 to 0 glitch*/
	while(i < WLVL_FILTER_LEN) {
		/*first set to 0x1*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
		/*then set to 0x0*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

		do {
			val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
		} while (val == 0);

		while(!((readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num)) & WLVL_CHECK_BIT)) {
			val = readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num*0x20);
			val = (val + 1) & 0x7f;
			readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = val;
#ifdef DQ_CHANGE_WITH_DQS
			dll_wr_data_set(mc_reg_base, slice_num);
			wr_dq_half_set(mc_reg_base, slice_num);
			wr_dqs_half_set(mc_reg_base, slice_num);
#endif
			/*first set to 0x1*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
			/*then set to 0x0*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
			do {
			  val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
			} while (val == 0);

			i = 0;
		}
		i++;
	}

	/*set back*/
	val = readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20);
	val = ((val > WLVL_FILTER_LEN) ? val - WLVL_FILTER_LEN : val + 0x80 - WLVL_FILTER_LEN) & 0x7f;
	readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = val;
	dll_wr_data_set(mc_reg_base, slice_num);
	wr_dq_half_set(mc_reg_base, slice_num);
	wr_dqs_half_set(mc_reg_base, slice_num);
}

void dll_adj(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t DllWrdqs;
	uint8_t DllWrdq;
	DllWrdqs = readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20);

	if(DllWrdqs >= DLL_WRDQ_SUB) {
		DllWrdq = DllWrdqs - DLL_WRDQ_SUB;
	} else {
		DllWrdq = DllWrdqs + 0x80 - DLL_WRDQ_SUB;
	}

	if(DllWrdqs >= 0x0 && DllWrdqs < DLL_ADJ_RANGE) {
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = 0x8;
		readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET  + slice_num * 0x20) = (0x08 - DLL_WRDQ_SUB) & 0x7f;
	} else if (DllWrdqs >= 0x40 - DLL_ADJ_RANGE && DllWrdqs < 0x40) {
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = 0x38;
		readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET  + slice_num * 0x20) = (0x38 - DLL_WRDQ_SUB) & 0x7f;
	} else if (DllWrdqs >= 0x40 && DllWrdqs < 0x40 + DLL_ADJ_RANGE) {
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = 0x48;
		readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET  + slice_num * 0x20) = (0x48 - DLL_WRDQ_SUB) & 0x7f;
	} else if (DllWrdqs >= 0x80 - DLL_ADJ_RANGE && DllWrdqs <= 0x7f) {
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = 0x78;
		readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET  + slice_num * 0x20) = (0x78 - DLL_WRDQ_SUB) & 0x7f;
	}

	if(DllWrdq >= 0x0 && DllWrdq < DLL_ADJ_RANGE) {
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = (0x08 + DLL_WRDQ_SUB) & 0x7f;
		readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET  + slice_num * 0x20) = 0x08;
	} else if (DllWrdq >= 0x40-DLL_ADJ_RANGE && DllWrdq < 0x40) {
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = (0x38 + DLL_WRDQ_SUB) & 0x7f;
		readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET  + slice_num * 0x20) = 0x38;
	} else if (DllWrdq >= 0x40 && DllWrdq < 0x40 + DLL_ADJ_RANGE) {
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = (0x48 + DLL_WRDQ_SUB) & 0x7f;
		readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET  + slice_num * 0x20) = 0x48;
	} else if (DllWrdq >= 0x80 - DLL_ADJ_RANGE && DllWrdq <= 0x7f) {
		readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET + slice_num * 0x20) = (0x78 + DLL_WRDQ_SUB) & 0x7f;
		readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET  + slice_num * 0x20) = 0x78;
	}
}

void wr_dq_clkdelay_set(uint64_t mc_reg_base, uint32_t ModuleType, uint8_t  EccEnable)
{
	int32_t i, j;
	uint8_t val, last_value, value;
	uint8_t already_sub = 0;

	if(ModuleType == UDIMM || ModuleType == SODIMM) {
		/*first set all wrdq_clkdelay to 0x0*/
		for(i = 0;i < 8 + EccEnable; i++) {
			readb(mc_reg_base + WRDQ_CLKDELAY_OFFSET+uDimmOrder[i] * 0x20) = 0x0;
		}

		for(i = 0;i <7 + EccEnable; i++) {
			last_value = readb(mc_reg_base + WRDQ_LT_HALF_OFFSET + uDimmOrder[i] * 0x20);
			value = readb(mc_reg_base + WRDQ_LT_HALF_OFFSET + uDimmOrder[i+1] * 0x20);

			//pr_info(EFI_D_INFO,"slice %d: last_value = %x, value = %x\r\n", i, last_value, value);

			if(i == 0 && last_value == 1) {
				//pr_info(EFI_D_INFO,"slice %d: SUB", i);
				val = readb(mc_reg_base + tRDDATA_OFFSET);
				val -= 1;
				readb(mc_reg_base + tRDDATA_OFFSET) = val;

				val = readb(mc_reg_base + tPHY_WRLAT_OFFSET);
				val -= 1;
				readb(mc_reg_base + tPHY_WRLAT_OFFSET) = val;
			}

			if(last_value == 1 && value == 0) {
				for(j = i + 1;j < 8 + EccEnable; j++) {
					readb(mc_reg_base + WRDQ_CLKDELAY_OFFSET+uDimmOrder[j] * 0x20) = 0x1;
				}
				break;
			}
		}
	} else if (ModuleType == RDIMM) {
		/*first set all wrdq_clkdelay to 0x0*/
		for(i = 0;i < 8 + EccEnable; i++) {
			readb(mc_reg_base + WRDQ_CLKDELAY_OFFSET+rDimmOrder[i] * 0x20) = 0x0;
		}

		for(i = 1 - EccEnable;i < 5; i++) {
			last_value = readb(mc_reg_base + WRDQ_LT_HALF_OFFSET + rDimmOrder[i] * 0x20);
			value = readb(mc_reg_base + WRDQ_LT_HALF_OFFSET + rDimmOrder[i+1] * 0x20);
			//pr_info(EFI_D_INFO,"last_value = %x, value = %x\n", last_value, value);
			if(i == 1 - EccEnable && last_value == 1) {
				val = readb(mc_reg_base + tRDDATA_OFFSET);
				val -= 1;
				readb(mc_reg_base + tRDDATA_OFFSET) = val;

				val = readb(mc_reg_base + tPHY_WRLAT_OFFSET);
				val -= 1;
				readb(mc_reg_base + tPHY_WRLAT_OFFSET) = val;
				already_sub = 1;
			}

			if(last_value == 1 && value == 0) {
				for(j = i + 1;j < 5; j++) {
					readb(mc_reg_base + WRDQ_CLKDELAY_OFFSET+rDimmOrder[j] * 0x20) = 0x1;
				}
				break;
			}
		}

		for(i = 5; i < 8; i++) {
			last_value = readb(mc_reg_base + WRDQ_LT_HALF_OFFSET + rDimmOrder[i] * 0x20);
			value = readb(mc_reg_base + WRDQ_LT_HALF_OFFSET + rDimmOrder[i+1] * 0x20);
			//pr_info(EFI_D_INFO,"last_value = %x, value = %x\n", last_value, value);

			if(i == 5 && last_value == 1) {
				if(!already_sub) {
				val = readb(mc_reg_base + tRDDATA_OFFSET);
				val -= 1;
				readb(mc_reg_base + tRDDATA_OFFSET) = val;

				val = readb(mc_reg_base + tPHY_WRLAT_OFFSET);
				val -= 1;
				readb(mc_reg_base + tPHY_WRLAT_OFFSET) = val;
				}
			}

			if(last_value == 1 && value == 0) {
				for(j = i + 1; j < 9; j++) {
					readb(mc_reg_base + WRDQ_CLKDELAY_OFFSET+rDimmOrder[j] * 0x20) = 0x1;
				}
				break;
			}
		}
	}
}

void rddqs_cnt_check (uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	uint8_t rddqs_cnt1 = 0;
	uint8_t rddqs_cnt2 = 0;

	/*first read;*/
	/*first set to 0x1*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
	/*then set to 0x0*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
	do {
		val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
	} while (val == 0);

	rddqs_cnt1 = (readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num) >> 2) & 0x7;

	/*second read;*/
	/*first set to 0x1*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
	/*then set to 0x0*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
	do {
		val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
	} while (val == 0);

	rddqs_cnt2 = (readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num) >> 2) & 0x7;

	if(((rddqs_cnt2 - rddqs_cnt1) & 0x7) != BURST_LENGTH/2) {
		pr_info("Slice %d: rddqs counter check error, counter value is: %d\n", slice_num, ((rddqs_cnt2 - rddqs_cnt1) & 0x7));
	}else{
		pr_info("Slice %d: rddqs counter check pass\n", slice_num);
	}
}

void dd_gate_add(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	uint16_t k;

	val = readb(mc_reg_base + DDR3_DLL_GATE_OFFSET + slice_num * 0x20);
	val = (val + 1) & 0x7f;
	readb(mc_reg_base + DDR3_DLL_GATE_OFFSET + slice_num * 0x20) = val;
	if(val == 0x00) {
		DDR_tRDDATA_ADD;
		/*sync code*/
		for(k = 0; k < 512; k++) { 
			/*first set to 0x1*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
			/*then set to 0x0*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
		}
	}
}

void glvl_get0(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	uint8_t i = 0;
	/*first set to 0x1*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
	/*then set to 0x0*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

	do {
		val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
	} while (val == 0);

	while((readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num)) & GLVL_CHECK_BIT) {
		dd_gate_add(mc_reg_base, slice_num);
		/*first set to 0x1*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
		/*then set to 0x0*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

		do {
			val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
		} while (val == 0);
	}

	/*filter the 0 to 1 glitch*/
	while(i < 0x10) {
		dd_gate_add(mc_reg_base, slice_num);
		/*first set to 0x1*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
		/*then set to 0x0*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

		do {
			val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
		} while (val == 0);

		while((readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num)) & GLVL_CHECK_BIT) {
			dd_gate_add(mc_reg_base, slice_num);
			/*first set to 0x1*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
			/*then set to 0x0*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

			do {
				val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
			} while (val == 0);

			i = 0;
		}
		i++;
	}
	val = readb(mc_reg_base + DDR3_DLL_GATE_OFFSET + slice_num * 0x20);
	pr_info("gate leveling: slice %d found 0, dll_gate = 0x%02lx\n", slice_num, val);
}

void glvl_get1(uint64_t mc_reg_base,uint32_t slice_num)
{
	uint8_t val;
	uint8_t i = 0;
	/*first set to 0x1*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
	/*then set to 0x0*/
	readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

	do {
		val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
	} while (val == 0);

	while(!((readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num)) & GLVL_CHECK_BIT)) {
		dd_gate_add(mc_reg_base, slice_num);
		/*first set to 0x1*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
		/*then set to 0x0*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

#ifdef DDR_DEBUG
		pr_info("gate leveling: slice %d searching 1\r\n", slice_num);
#endif

		do {
			val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
		} while (val == 0);
	}

	/*filter the 1 to 0 glitch*/
	while(i < GLVL_FILTER_LEN) {
		dd_gate_add(mc_reg_base, slice_num);
		/*first set to 0x1*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
		/*then set to 0x0*/
		readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;

		do {
			val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
		} while (val == 0);

		while(!((readb(mc_reg_base + DDR3_LVL_RESP_OFFSET + slice_num)) & GLVL_CHECK_BIT)) {
			dd_gate_add(mc_reg_base, slice_num);

#ifdef DDR_DEBUG
			pr_info("gate leveling: slice %d filter 1 to 0\r\n", slice_num);
#endif
			/*first set to 0x1*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
			/*then set to 0x0*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
			do {
				val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
			} while (val == 0);

			i = 0;
		}
		i++;
	}

	/*set back*/
	val = readb(mc_reg_base + DDR3_DLL_GATE_OFFSET + slice_num * 0x20);
	if(val >= GLVL_FILTER_LEN) {
		val = (val - GLVL_FILTER_LEN) & 0x7f;
	} else {
		val = (val + 0x80 - GLVL_FILTER_LEN) & 0x7f;
		DDR_tRDDATA_SUB;
	}
	readb(mc_reg_base + DDR3_DLL_GATE_OFFSET + slice_num * 0x20) = val;

#ifdef DEBUG_ENABLE_LOONGSON
/*def DDR_DEBUG*/
	uint8_t tRDDATA = readb(mc_reg_base + tRDDATA_OFFSET);
	pr_info("gate leveling: slice %d found 1, dll_gate = 0x%02lx, tRDDATA = 0x%02lx\r\n", slice_num, val, tRDDATA);
#endif
}

void gmem_dll_gate_sub(uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	uint16_t k;

	val = readb(mc_reg_base + DDR3_DLL_GATE_OFFSET + slice_num * 0x20);
	if(val >= DLL_GATE_SUB) {
		val -= DLL_GATE_SUB;
	} else {
		val = val + 0x80 - DLL_GATE_SUB;

		DDR_tRDDATA_SUB;
	       	/*sync code*/
		for(k = 0; k < 512; k++) {
			/*first set to 0x1*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
			/*then set to 0x0*/
			readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
		}
	}
	readb(mc_reg_base + DDR3_DLL_GATE_OFFSET + slice_num * 0x20) = val;
}

void rddqs_half_set(uint64_t mc_reg_base,uint32_t slice_num)
{
	/*TODO, add edge control*/
	uint8_t val_0;
	uint8_t val_1;
	uint8_t val;

	val_0 = readb(mc_reg_base + DDR3_DLL_WRDQ_OFFSET + slice_num * 0x20);
	val_1 = readb(mc_reg_base + DDR3_DLL_GATE_OFFSET + slice_num * 0x20);
	val   = (val_0 + val_1) & 0x7f;
	if((val >= RDDQS_LT_HALF_STD1) || (val < RDDQS_LT_HALF_STD2)) {
		val = 1;
	} else {
		val = 0;
	}

	readb(mc_reg_base + RDDQS_LT_HALF_OFFSET + slice_num * 0x20) = val;
}

void rddqs_preamble_check (uint64_t mc_reg_base, uint32_t slice_num)
{
	uint8_t val;
	uint8_t j;
	uint16_t k;
	uint8_t dll_gate_ori;
	uint8_t preamble_found;
	uint8_t has_sub;
	uint8_t sample = 0;

	dll_gate_ori = readb(mc_reg_base + DDR3_DLL_GATE_OFFSET+slice_num*0x20);

	do {
		preamble_found = 1;
		has_sub  = 0;
		for(j = 0;j < PREAMBLE_LEN - 0x10; j++) {
			val = dll_gate_ori-0x10-j;
			val = val & 0x7f;
			readb(mc_reg_base + DDR3_DLL_GATE_OFFSET+slice_num*0x20) = val;
#ifdef DDR_DEBUG
			pr_info("slice %d dll_gate = 0x%x\n", slice_num, val);
#endif
			if(val == 0x7f) {
				DDR_tRDDATA_SUB;
				has_sub = 1;
				/*sync code*/
				for(k = 0; k < 512; k++) {
					/*first set to 0x1*/ 
					readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
					/*then set to 0x0*/
					readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
				}
			}

			for(k = 0; k < 5; k++) {
				/*first set to 0x1*/
				readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x1;
				/*then set to 0x0*/
				readb(mc_reg_base + DDR3_LVL_REQ_OFFSET) = 0x0;
				do {
					val = readb(mc_reg_base + DDR3_LVL_DONE_OFFSET);
				} while (val == 0);

				sample = (sample << 1) | (readb(mc_reg_base + DDR3_LVL_RESP_OFFSET+slice_num) & GLVL_CHECK_BIT & 0x1);
			}

#ifdef DDR_DEBUG
			pr_info("slice %d glvl_resp = 0x%x\n", slice_num, sample);
#endif
			if(sample & 0x1f) {
				pr_info("slice %d preamble check failed @ 0x%x\n", slice_num, j);
				preamble_found = 0;
				if(!has_sub) {
					DDR_tRDDATA_SUB;
				}
				break;
			}
			if(j == PREAMBLE_LEN - 0x10 - 1) {
				pr_info("slice %d preamble check pass\n", slice_num);
			}
		}
	} while (!preamble_found);

	glvl_get1(mc_reg_base, slice_num);
	rddqs_half_set(mc_reg_base, slice_num);
	gmem_dll_gate_sub(mc_reg_base, slice_num);
}

void  ddr3_leveling(uint64_t regs_base, uint64_t node_id, uint8_t module_type, uint8_t slice_num, uint8_t raw_card_version)
{
	uint64_t i = 0;
	uint8_t  val;
	uint8_t  trddataori;
	uint8_t  trddata[slice_num];
	uint64_t mc_reg_base = regs_base;
	mc_reg_base = mc_reg_base | (node_id << NODE_OFFSET);
	uint8_t  max=0;
	uint8_t  min=255;
	uint8_t  ecc_enable = 0;
	/*If enable Ecc, the slice_num will be 9, otherwise it is 8;*/
	ecc_enable = slice_num & 0x1; 

	if(raw_card_version == RAW_CARD_VERSION_F && module_type == SODIMM) {
		uDimmOrder[0]= 3;
		uDimmOrder[1]= 1;
		uDimmOrder[2]= 0;
		uDimmOrder[3]= 2;
		uDimmOrder[4]= 5;
		uDimmOrder[5]= 6;
		uDimmOrder[6]= 7;
		uDimmOrder[7]= 4;
		uDimmOrder[8]= 8;
	}

	wait_init_done(mc_reg_base);

	pr_info("write leveling begin\n");
	/*set all DllWrdqs to 0x0*/
	for(i = 0; i < slice_num; i++) {
	    readb(mc_reg_base + DDR3_DLL_WRDQS_OFFSET+i*0x20) = 0x0;
	}
	/*set leveling_mode*/
	pr_info("set leveling mode to be WRITE LEVELING\n");
	readb(mc_reg_base + DDR3_LVL_MODE_OFFSET) = 0x1;

	do {
		val = readb(mc_reg_base + DDR3_LVL_READY_OFFSET);
	} while (val == 0);

	pr_info("write leveling ready\n");

	for(i = 0; i < slice_num; i++) {
		wlvl_get0(mc_reg_base, i);
	}

	for(i = 0; i < slice_num; i++) {
		wlvl_get1(mc_reg_base, i);
	}

	for(i = 0; i < slice_num; i++) {
		dll_adj(mc_reg_base, i);
	}
	/*UDIMM:00, RDIMM:01*/
	wr_dq_clkdelay_set(mc_reg_base, module_type, ecc_enable);

	readb(mc_reg_base + DDR3_LVL_MODE_OFFSET) = 0x0;

	pr_info("the parameter after write leveling is:\n");
#ifdef DEBUG_ENABLE_LOONGSON
/*def DDR_DEBUG*/
	uint32_t reg_addr;
	uint64_t val64;
	for(i = 0;i < MC_REGS_COUNT; i++) {
		reg_addr = 0x8 * i;
		val64 = readq(mc_reg_base + reg_addr);
		pr_info("%03x: %016lx\n", reg_addr, val64);
	}
	if(i % 4 == 3) {
		pr_info( "\n");
	}
#endif

	readb(mc_reg_base + DDR3_LVL_MODE_OFFSET) = 0x0;

	/*reset init_start*/
	readb(mc_reg_base + DDR3_INIT_START_OFFSET) = 0x0;
	readb(mc_reg_base + DDR3_INIT_START_OFFSET) = 0x1;

	wait_init_done(mc_reg_base);

	/*set all dll_gate to 0x0*/
	for(i = 0; i < slice_num; i++) {
		readb(mc_reg_base + DDR3_DLL_GATE_OFFSET+i*0x20) = 0x0;
	}

	/*set leveling_mode*/
	pr_info("set leveling mode to be Gate LEVELING\r\n");

	readb(mc_reg_base + DDR3_LVL_MODE_OFFSET) = 0x2;

	do {
		val = readb(mc_reg_base + DDR3_LVL_READY_OFFSET);
	} while (val == 0);


	for(i = 0; i < slice_num; i++) {
		trddataori = readb(mc_reg_base + tRDDATA_OFFSET);
		readb(mc_reg_base + DDR3_DLL_GATE_OFFSET+i*0x20) = 0x0;
		glvl_get0(mc_reg_base, i);

		/*reset init_start*/
		readb(mc_reg_base + DDR3_INIT_START_OFFSET) = 0x0;
		readb(mc_reg_base + DDR3_INIT_START_OFFSET) = 0x1;
		wait_init_done(mc_reg_base);

		glvl_get1(mc_reg_base, i);
		rddqs_preamble_check(mc_reg_base, i);
		rddqs_cnt_check(mc_reg_base, i);
		trddata[i] = readb(mc_reg_base + tRDDATA_OFFSET);
		readb(mc_reg_base + tRDDATA_OFFSET) = trddataori;
	}

	/*find the max rd_oe_begin/end*/
	for(i = 0; i < slice_num; i++) {
		if(max < trddata[i]) {
			max = trddata[i];
		}
		if(min > trddata[i]) {
			min = trddata[i];
		}
	}

#ifdef DDR_DEBUG
	for(i = 0; i < slice_num; i++) {
		pr_info("trddata[%01d] = 0x%02lx\n", i, tRDDATA[i]);
	}
		pr_info("max = %01d, min = %01d\n", max, min);
#endif

	if(max-min > 3) {
		pr_info("ERROR: read gate window difference is too large\n");
	} else {
	       	/*here assume all slice has same rd_oe_begin/end*/
		readb(mc_reg_base + tRDDATA_OFFSET)  = min + readb(mc_reg_base + RDOE_BEGIN_OFFSET) - 1;
		for(i = 0; i < slice_num; i++) {
			val = trddata[i] - min + 1;
			readb(mc_reg_base + RDOE_BEGIN_OFFSET + i * 0x20) = val;
			readb(mc_reg_base + RDOE_END_OFFSET + i * 0x20) = val;

			val = readb(mc_reg_base + RDOE_START_OFFSET + i * 0x20);
			if(val > 0x1) {
				val -= 0x2;
				readb(mc_reg_base + RDODT_START_OFFSET + i * 0x20) = val;
				val = readb(mc_reg_base + RDOE_BEGIN_OFFSET + i * 0x20);
				readb(mc_reg_base + RDODT_BEGIN_OFFSET + i * 0x20) = val;
			} else {
				val += 0x2;
				readb(mc_reg_base + RDODT_START_OFFSET + i * 0x20) = val;
				val = readb(mc_reg_base + RDOE_BEGIN_OFFSET + i * 0x20);
				val -= 0x1;
				readb(mc_reg_base + RDODT_BEGIN_OFFSET + i * 0x20) = val;
			}

			val = readb(mc_reg_base + RDOE_STOP_OFFSET + i * 0x20);
			if(val < 0x2) {
				val += 0x2;
				readb(mc_reg_base + RDODT_STOP_OFFSET + i * 0x20) = val;
				val = readb(mc_reg_base + RDOE_END_OFFSET + i * 0x20);
				readb(mc_reg_base + RDODT_END_OFFSET + i * 0x20) = val;
			} else {
				val -= 0x2;
				readb(mc_reg_base + RDODT_STOP_OFFSET + i * 0x20) = val;
				val = readb(mc_reg_base + RDOE_END_OFFSET + i * 0x20);
				val += 0x1;
				readb(mc_reg_base + RDODT_END_OFFSET + i * 0x20) = val;
			}
		}
	}


	readb(mc_reg_base + DDR3_LVL_MODE_OFFSET) = 0x0;

	/*reset init_start*/
	readb(mc_reg_base + DDR3_INIT_START_OFFSET) = 0x0;
	readb(mc_reg_base + DDR3_INIT_START_OFFSET) = 0x1;
	wait_init_done(mc_reg_base);

	readb(mc_reg_base + 0x19) = 0x1;
	readb(mc_reg_base + 0x7) = 0x0;
}

void ls7a_gmem_init(uint64_t s1_gmem)
{
	//volatile uint64_t *mc_reg_base = 0;
	uint64_t val, bank, row, col;
	uint32_t i = 0;
	uint8_t gmem_module_type;

	pr_info("enable gmem config space\n");
	readl(LS7A_CONFBUS_BASE_ADDR + CONF_NB_OFFSET + 0x4) &= ~(0x1 << 24);

	for (i = 0; i < MC_REGS_COUNT; i++) {
		//mc_reg_base = (uint64_t *)(LS7A_GMEM_TEMP_ADDR + (i << 3));
		//*mc_reg_base = gmem_param[i];
		readq(LS7A_GMEM_TEMP_ADDR + (i << 3)) = gmem_param[i];
	}

	/*rewrite eight_bank_mode*/
	/*rewrite pm_bank_diff_0 and pm_bank*/
	val = readq(LS7A_GMEM_TEMP_ADDR + EIGHT_BANK_MODE_ADDR);
	val &= ~(0x3ULL << EIGHT_BANK_MODE_OFFSET);
	bank = ((s1_gmem >> EIGHT_BANK_OFFSET) & 0x1);
	bank = ((~bank) & 0x1);
	val |= (bank << EIGHT_BANK_MODE_OFFSET);
	readq(LS7A_GMEM_TEMP_ADDR + EIGHT_BANK_MODE_ADDR) = val;

	/*rewrite pm_addr_win*/
	val = readq(LS7A_GMEM_TEMP_ADDR + ADDR_WIN_BANK_NUM_ADDR);
	val &= ~(0x3ULL << ADDR_WIN_BANK_NUM_OFFSET);
	bank = ((s1_gmem >> ADDR_WIN_BANK_NUM_OFFSET) & 0x1);
	bank |= 0x2ULL;
	val |= (bank << ADDR_WIN_BANK_NUM_OFFSET);
	readq(LS7A_GMEM_TEMP_ADDR + ADDR_WIN_BANK_NUM_ADDR) = val;

	/*rewrite row_diff and column_diff*/
	val = readq(LS7A_GMEM_TEMP_ADDR + ROW_DIFF_ADDR);
	val &= ~(0x7ULL << ROW_DIFF_OFFSET);
	row = ((s1_gmem >> ROW_SIZE_OFFSET) & 0x7);
	val |= (row << ROW_SIZE_OFFSET);
	readq(LS7A_GMEM_TEMP_ADDR + ROW_DIFF_ADDR) = val;


	val = readq(LS7A_GMEM_TEMP_ADDR + COLUMN_DIFF_ADDR);
	val &= ~(0x7ULL << COLUMN_DIFF_OFFSET);
	col = ((s1_gmem >> COL_SIZE_OFFSET) & 0x3);
	col += 0x4;
	val |= (col << COLUMN_DIFF_OFFSET);
	readq(LS7A_GMEM_TEMP_ADDR + COLUMN_DIFF_ADDR) = val;

	/*reconfig address_mirroring*/
	val = readq(LS7A_GMEM_TEMP_ADDR + ADDRESS_MIRROR_ADDR);
	val &= ~(0xfULL << ADDRESS_MIRROR_OFFSET);
	val |= (0ULL << ADDRESS_MIRROR_OFFSET);
	readq(LS7A_GMEM_TEMP_ADDR + ADDRESS_MIRROR_ADDR) = val;

#ifndef MANAUL_ODT_MAP
	/*reconfig ODT map*/
	/*set default first*/
	/*clear map first*/
	readq(LS7A_GMEM_TEMP_ADDR + ODT_MAP_CS_ADDR) &= 0x0000ffff0000ffffULL;
	if(((s1_gmem >> SDRAM_TYPE_OFFSET) & 0x3) == 0x3) {
		/*ddr3*/
		readq(LS7A_GMEM_TEMP_ADDR + ODT_MAP_CS_ADDR) |= 0x8421000000000000ULL;
	} else {
		/*ddr2*/
		readq(LS7A_GMEM_TEMP_ADDR + ODT_MAP_CS_ADDR) |= 0x8421000084210000ULL;
	}
#endif

	/***** set start to 1,start to initialize SDRAM *****/
	readb(LS7A_GMEM_TEMP_ADDR + 0x18) |= 0x1;

	pr_info("wait start.....\n");
	while (!readb(LS7A_GMEM_TEMP_ADDR + 0x163));
	pr_info("start .....\n");
	/*RDIMM is 0x1; UDIMM is 0x2, here send 0x0 to leveling; otherwise, the 7A's GPU will worked unnormal.*/
	gmem_module_type = 0; 

	ddr3_leveling(LS7A_GMEM_TEMP_ADDR, 0x0, gmem_module_type, 2, 0x0);

	pr_info("disable gmem config space\n");
	readl(LS7A_CONFBUS_BASE_ADDR + CONF_NB_OFFSET + 0x4) |= (0x1 << 24);
}
