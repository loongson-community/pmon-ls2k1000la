#include <include/stdio.h>
#include <sys/types.h>
#include "cpu.h"
#include "target/ls7a.h"
#ifdef LS_STR
extern void asm_wait_for_kernel(void);

void check_str()
{
	uint32_t system_s;
	struct acpi_fadt *fadt;

	system_s = readl(LS7A_ACPI_PM1_CNT_REG);
	tgt_printf("Sleep type: %x - system is ", (system_s >> 10) & 0x7);
	if (((system_s >> 10) & 0x7) != SLEEP_TYPE_S3) {
		tgt_printf("normal boot.\n");
		return;
	}
	tgt_printf("S3 boot.\n");

	wakeup_ap(asm_wait_for_kernel, NULL);
	tgt_fpudisable();
	/* misc:0x1008,0000 -- 0x100f,ffff */
	/* acpi offset 0x50000 of misc */
	/* LS7A_ACPI_PM1_CNT_REG is cmos reg which storage s3 flag, now clear this flag */
	readl(LS7A_ACPI_PM1_CNT_REG) &= (~(0x7 << 10));
	ls7a_pcie_irq_fixup();
	loongson_ht_trans_init();

	fadt = acpi_find_fadt();
	acpi_resume(fadt);
}
#endif
