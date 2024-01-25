struct dts_node {
	char* name;
	unsigned char bus;
	unsigned char dev;
	unsigned char func;
};

struct dts_node nodes[] = {
	[0] = {
		.name	= "/soc/ethernet@0x406c8000",
		.bus	= 0,
		.dev	= 3,
		.func	= 0,
	},

	[1] = {
		.name	= "/soc/ethernet@0x406c0000",
		.bus	= 0,
		.dev	= 3,
		.func	= 1,
	},

	[2] = {
		.name	= "/soc/otg@0x40640000",
		.bus	= 0,
		.dev	= 4,
		.func	= 0,
	},

	[3] = {
		.name	= "/soc/ehci@0x406b8000",
		.bus	= 0,
		.dev	= 4,
		.func	= 1,
	},

	[4] = {
		.name	= "/soc/ohci@406b0000",
		.bus	= 0,
		.dev	= 4,
		.func	= 2,
	},

	[5] = {
		.name	= "/soc/gpu@0x40060000",
		.bus	= 0,
		.dev	= 5,
		.func	= 0,
	},

	[6] = {
		.name	= "/soc/dc@0x400a0000",
		.bus	= 0,
		.dev	= 6,
		.func	= 0,
	},

	[7] = {
		.name	= "/soc/hda@0x400d0000",
		.bus	= 0,
		.dev	= 7,
		.func	= 0,
	},

	[8] = {
		.name	= "/soc/ahci@0x40680000",
		.bus	= 0,
		.dev	= 8,
		.func	= 0,
	},
};
