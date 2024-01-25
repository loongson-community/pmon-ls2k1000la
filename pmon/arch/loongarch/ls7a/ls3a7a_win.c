#include <cpu.h>
#include "target/ls7a_config.h"
#include "ht.h"

extern int ls3a_ht_config(uint64_t base, uint64_t node_id);
extern int ls7a_ht_config(uint64_t base);

//#define PRINT_HT1_REG
int ls3a7a_ht_config()
{
	uint64_t node_id;
	uint64_t ls3a_ht_base, ls7a_ht_base;
	int i;

	for (i = 0; i < sizeof(ls7a_link_id_buf); i++) {
		node_id = ls7a_link_id_buf[i];
		ls3a_ht_base = (node_id << NODE_OFFSET) | HT1_CTRL_CONF_BASE;
		ls7a_ht_base = (node_id << NODE_OFFSET) | HT1_CONF_BASE;
#ifdef PRINT_HT1_REG
		uint64_t j;
		pr_info("LS3A NODE %d HT registers before init\n", node_id);
		for (j = 0; j < 0x180; j += 0x4) {
			pr_info("0x%llx: 0x%016lx\n", ls3a_ht_base + j, readl(ls3a_ht_base + j));
		}

		pr_info("LS7A to NODE %d HT registers before init\n", node_id);
		for (j = 0; j < 0x268; j += 0x4) {
			pr_info("0x%llx: 0x%016lx\n", ls7a_ht_base + j, readl(ls3a_ht_base + j));
		}
#endif
		/*3A side HT init*/
		ls3a_ht_config(ls3a_ht_base, node_id);
		/*7A side HT configure begin*/
		ls7a_ht_config(ls7a_ht_base);
#ifdef PRINT_HT1_REG
		pr_info("LS3A NODE %d HT registers after init\n", node_id);
		for (j = 0; j < 0x180; j += 0x4) {
			pr_info("0x%llx: 0x%016lx\n", ls3a_ht_base + j, readl(ls3a_ht_base + j));
		}

		pr_info("LS7A to NODE %d HT registers after init\n", node_id);
		for (j = 0; j < 0x268; j += 0x4) {
			pr_info("0x%llx: 0x%016lx\n", ls7a_ht_base + j, readl(ls3a_ht_base + j));
		}
#endif
	}
	return 0;
}
