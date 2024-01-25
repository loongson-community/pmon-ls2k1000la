#include "target/ls7a_config.h"
#include "target/ls7a.h"

#include "ls3a7a_setup_ht_link.c"
#include "ls7a.c"
#include "ls3a7a_win.c"


void ls7a_init(void)
{
	int status;
	//Initialize LS7A here
	pr_info("Ls7A init begain\n");
	ls3a7a_ht_config();

#ifdef DEBUG_ENABLE_LOONGSON
	pr_info("------------------Test Ls7a access-----------------\n");
	status = ls7a_dbg();
	if (status) {
		pr_info("Test Ls7a access fail!\n");
		while(1);
	}
#endif

	ls7a_resource_cfg(&ls7a_cfg_t);
	loongson_ht_trans_init();

	if (ls7a_version()) {
		/*Debug New 7A Func,Drop Rtc voltage*/
		do {
			readl(LS7A_ACPI_PMCON_RTC_REG) |= (0x3 << 9);
		} while (((readl(LS7A_ACPI_PMCON_RTC_REG) >> 9) & 0x3) != 0x3);
	}
	ls7a_clear_pcie_portirq();
	pr_info("Ls7A1000 Chipset initialize done.\n");
}
