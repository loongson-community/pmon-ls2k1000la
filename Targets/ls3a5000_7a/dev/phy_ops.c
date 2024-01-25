#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <types.h>
#include <fcntl.h>
#include <pmon.h>

uint32_t ls7a2000_phy_cfg_read_test(uint32_t base, uint32_t offset)
{
	uint64_t ls7a_base = 0x80000e0010010000;
	uint32_t tmp;
	int time_out = 1000;

	readl((ls7a_base + base + 0x4)) = (offset & 0xfffff) | (0x1<<25) | (0x1<<24) | (0x0<<26) | (0x1<<27);
	readl((ls7a_base + base + 0x4)) = (offset & 0xfffff) | (0x1<<25) | (0x1<<24) | (0x0<<26) | (0x0<<27);

	do {
		tmp = readl(ls7a_base + (base == 0x800 ? base + 0x20 : base) + 4);
	} while(!((tmp >> 28) & 0x1) /* && time_out-- */);

	if (time_out <= 0) {
		printf("- - %s %llx time out\n", __func__, base + ls7a_base + 4);
		while(1);
	}
	return readl(ls7a_base + (base == 0x800 ? base + 0x20 : base));
}

void ls7a2000_phy_cfg_write_test(uint32_t base, uint32_t offset, uint32_t data)
{
	uint64_t ls7a_base = 0x80000e0010010000;
	uint32_t tmp;
	int time_out = 1000;

	readl(ls7a_base + base) = data;
	readl(ls7a_base + base + 4) = (offset & 0xfffff) | (0x1<<25) | (0x1<<24) | (0x1<<26) | (0x1<<27);
	readl(ls7a_base + base + 4) = (offset & 0xfffff) | (0x1<<25) | (0x1<<24) | (0x1<<26) | (0x0<<27);

	do {
		tmp = readl(ls7a_base + (base == 0x800 ? base + 0x20 : base) + 4);
	} while(!((tmp >> 28) & 0x1)/* && time_out-- */);

	if (time_out <= 0) {
		printf("- - %s %llx time out\n", __func__, base + ls7a_base + 4);
	}
}

void ls7a_phy_read(int ac, unsigned char *av[])
{
	if (ac != 3) {
		printf("For example USB3 :ls7a_phy_read 0x800 regoffset\n");
		printf("For example SATA :ls7a_phy_read 0x748 regoffset\n");
		printf("For example PCIE FO :ls7a_phy_read 0x590 regoffset\n");
		printf("For example PCIE F1 :ls7a_phy_read 0x5b0 regoffset\n");
		printf("For example PCIE H :ls7a_phy_read 0x5d0 regoffset\n");
		printf("For example PCIE G0 :ls7a_phy_read 0x5f0 regoffset\n");
		return;
	}
	unsigned int base = strtoul(av[1], 0, 0);
	unsigned int offset = strtoul(av[2], 0, 0);
	printf("args 0x%x 0x%x\n",base, offset);
	printf("-> 0x%x\n",ls7a2000_phy_cfg_read_test(base, offset));
}

void ls7a_phy_write(int ac, unsigned char *av[])
{
	if (ac != 4) {
		printf("For example USB3 :ls7a_phy_write 0x800 regoffset data\n");
		printf("For example SATA :ls7a_phy_write 0x748 regoffset data\n");
		printf("For example PCIE FO :ls7a_phy_write 0x590 regoffset data\n");
		printf("For example PCIE F1 :ls7a_phy_write 0x5b0 regoffset data\n");
		printf("For example PCIE H :ls7a_phy_write 0x5d0 regoffset data\n");
		printf("For example PCIE G0 :ls7a_phy_write 0x5f0 regoffset data\n");
		return;
	}
	unsigned int base = strtoul(av[1], 0, 0);
	unsigned int offset = strtoul(av[2], 0, 0);
	unsigned int data = strtoul(av[3], 0, 0);
	printf("args 0x%x 0x%x 0x%x\n",base, offset, data);
	ls7a2000_phy_cfg_write_test(base, offset, data);
	printf("-> 0x%x\n",ls7a2000_phy_cfg_read_test(base, offset));
}
static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"ls7a_phy_read", "", 0, "read phy", ls7a_phy_read, 0, 99, CMD_REPEAT},
	{"ls7a_phy_write", "", 0, "write phy", ls7a_phy_write, 0, 99, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
