#include <pmon.h>
#include "target/dvfs.h"

int call_132(int argc,char **argv)
{
	if (argc == 2) {
		int flag = (uint32_t)strtoul(argv[1], 0, 0);
		if (flag == 0) {
			printf("turn off LS132 CORE\n");
			readl(PHYS_TO_UNCACHED(0x1fe00420)) &= ~0x100;
		} else if (flag == 1) {
			printf("turn on LS132 CORE\n");
			readl(PHYS_TO_UNCACHED(0x1fe00420)) |= 0x100;
		} else if (flag == 2) {
			printf("reset LS132 CORE\n");
			readl(PHYS_TO_UNCACHED(0x1fe00420)) &= ~0x100;
			delay(1000);
			readl(PHYS_TO_UNCACHED(0x1fe00420)) |= 0x100;
		} else if (flag == 3) {
			printf("irq LS132 CORE\n");
			readl(PHYS_TO_UNCACHED(0x1fe00420)) |= 0xf400;
		}
	} else if (argc == 4) {
		//config ls132 0x1c000000 window
		printf("set 1c000000 to 0x%x\n", (uint32_t)strtoul(argv[3], 0, 0));
		readl(PHYS_TO_UNCACHED(0x1fe00300)) = (uint32_t)strtoul(argv[1], 0, 0);
		readl(PHYS_TO_UNCACHED(0x1fe00320)) = (uint32_t)strtoul(argv[2], 0, 0);
		readl(PHYS_TO_UNCACHED(0x1fe00340)) = (uint32_t)strtoul(argv[3], 0, 0);
	} else {
		printf("default set 1c000000 to 0x8ee00000\n");
		//config ls132 0x00200000 window
		readl(PHYS_TO_UNCACHED(0x1fe00300)) = 0x1c000000;
		readl(PHYS_TO_UNCACHED(0x1fe00320)) = 0x1fe00000;
		readl(PHYS_TO_UNCACHED(0x1fe00340)) = 0x0ee00080;
		printf("call_132:	set default window\n");
		printf("call_132 <0/1/2/3>:	to turn off/on/reboot/irq ls132\n");
		printf("call_132 <base> <mask> <mmap>:	to map 0x1c000000 to new address\n");
	}
	return 0;
}

int cmd_freq(int argc,char **argv)
{
	unsigned int count_old, count_new, cur;
	if (argc == 3) {
//		cur = (readl(LS7A_HPET_PERIOD) / 1000000);

		int level = (uint32_t)strtoul(argv[1], 0, 0);
		int cpu_id = (uint32_t)strtoul(argv[2], 0, 0);
		printf("set cpu%d freq level[%d], write 0x1fe0051c: 0x%x\n", cpu_id, level, (0x21 << 24) | (level << 4) | (cpu_id));
		readl(PHYS_TO_UNCACHED(0x1fe0051c)) = (0x21 << 24) | (level << 4) | (cpu_id);

//		count_old = readl(LS7A_HPET_MAIN);
		readl(PHYS_TO_UNCACHED(0x1fe00420)) |= 0x400;
		while (readl(PHYS_TO_UNCACHED(0x1fe0051c)) != 0x80000000);
//		count_new = readl(LS7A_HPET_MAIN);
//		printf("old[0x%x] - new[0x%x] = %d, time[%dns], set cpu%d freq level[%d], write 0x1fe0051c: 0x%x\n", \
				count_old, count_new, count_new - count_old, (count_new - count_old) * cur / 1000,	\
				cpu_id, level, (0x21 << 24) | (level << 4) | (cpu_id));
	} else {
		printf("freq <cpu_id> <level:3-10>\n");
	}
	//_probe_frequencies();
	return 0;
}

void ls132_init(void)
{
	memcpy(PHYS_TO_UNCACHED(0xee00000), dvfs_data, sizeof(dvfs_data));
	printf("default set 1c000000 to 0x0ee00000\n");
	//config ls132 0x00200000 window
	readl(PHYS_TO_UNCACHED(0x1fe00300)) = 0x1c000000;
	readl(PHYS_TO_UNCACHED(0x1fe00320)) = 0x1fe00000;
	readl(PHYS_TO_UNCACHED(0x1fe00340)) = 0x0ee00081;

	readl(PHYS_TO_UNCACHED(0x1fe0051c)) = MPS_ADDR | (MPS_STEP << 8) | (LOW_VOL_ADJUST << 16);
	readl(PHYS_TO_UNCACHED(0x1fe00420)) &= ~0x100;
	delay(1000);
	readl(PHYS_TO_UNCACHED(0x1fe00420)) |= 0x100;
	while (readl(PHYS_TO_UNCACHED(0x1fe0051c)) != 0x80000000);
}


static const Cmd Cmds[] =
{
	{"ls132"},
	{"call_132","",0,"call_132 <1: start/0: stop>", call_132, 0, 99, CMD_REPEAT},
	{"freq","",0,"freq <1: start/0: stop>", cmd_freq, 0, 99, CMD_REPEAT},
	{0, 0}
};
static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
