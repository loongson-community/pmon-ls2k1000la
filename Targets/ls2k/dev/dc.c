#include <pmon.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/malloc.h>
#include <machine/pio.h>
#include <target/ls2k1000.h>

#if !defined(DC_FB0) && !defined(DC_FB1)
#define DC_FB0 1
#endif

#if !defined(FB_XSIZE) || !defined(FB_YSIZE)
//#define FB_XSIZE 1920
//#define FB_YSIZE 1080
#define FB_XSIZE 1280
#define FB_YSIZE 1024
#endif

#define DIS_WIDTH  FB_XSIZE
#define DIS_HEIGHT FB_YSIZE
#define EXTRA_PIXEL  0

#define DC0_BASE_ADDR_OFF	0x1240
#define DC1_BASE_ADDR_OFF	0x1250

#define RANDOM_HEIGHT_Z 37

static char *ADDR_CURSOR = PHYS_TO_CACHED(0x4000000);
static char *MEM_ptr = PHYS_TO_CACHED(0x5000000);

static int MEM_ADDR = 0;

struct vga_struc {
	float pclk;
	int hr, hss, hse, hfl;
	int vr, vss, vse, vfl;
} vgamode[] = {
	//{	28.56,	640,	664,	728,	816,	480,	481,	484,	500,	},	/*"640x480_70.00" */
	{	23.86,	640,	656,	720,	800,	480,	481,	484,	497,	},	/*"640x480_60.00" */
	{	33.10,	640,	672,	736,	832,	640,	641,	644,	663,	},	/*"640x640_60.00" */
	{	39.69,	640,	672,	736,	832,	768,	769,	772,	795,	},	/*"640x768_60.00" */
	{	42.13,	640,	680,	744,	848,	800,	801,	804,	828,	},	/*"640x800_60.00" */
	{	35.84,	800,	832,	912,	1024,	480,	481,	484,	500,	},	/*"800x480_70.00" */
	{	38.22,	800,	832,	912,	1024,	600,	601,	604,	622,	},	/*"800x600_60.00" */
	{	40.73,	800,	832,	912,	1024,	640,	641,	644,	663,	},	/*"800x640_60.00" */
	{	40.01,	832,	864,	952,	1072,	600,	601,	604,	622,	},	/*"832x600_60.00" */
	{	40.52,	832,	864,	952,	1072,	608,	609,	612,	630,	},	/*"832x608_60.00" */
	{	45.98,	960,	864,	952,	1072,	600,	609,	612,	630,	},	/*"960x600_60.00" */
	{	38.17,	1024,	1048,	1152,	1280,	480,	481,	484,	497,	},	/*"1024x480_60.00" */
	{	48.96,	1024,	1064,	1168,	1312,	600,	601,	604,	622,	},	/*"1024x600_60.00" */
	{	52.83,	1024,	1072,	1176,	1328,	640,	641,	644,	663,	},	/*"1024x640_60.00" */
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/*"1024x768_60.00" */
	{	71.38,	1152,	1208,	1328,	1504,	764,	765,	768,	791,	},	/*"1152x764_60.00" */
	{	83.46,	1280,	1344,	1480,	1680,	800,	801,	804,	828,	},	/*"1280x800_60.00" */
	{	108.88,	1280,	1360,	1496,	1712,	1024,	1025,	1028,	1060,	},	/*"1280x1024_60.00" */
	{	85.86,	1368,	1440,	1584,	1800,	768,	769,	772,	795,	},	/*"1368x768_60.00" */
	{	93.80,	1440,	1512,	1664,	1888,	800,	801,	804,	828,	},	/*"1440x800_60.00" */
	{	120.28,	1440,	1528,	1680,	1920,	900,	901,	904,	935,	},	/*"1440x900_67.00" */
	{	172.80,	1920,	2040,	2248,	2576,	1080,	1081,	1084,	1118,	},	/*"1920x1080_60.00" */
};

enum {
	OF_BUF_CONFIG = 0,
	OF_BUF_ADDR = 0x20,
	OF_BUF_STRIDE = 0x40,
	OF_BUF_ORIG = 0x60,
	OF_DITHER_CONFIG = 0x120,
	OF_DITHER_TABLE_LOW = 0x140,
	OF_DITHER_TABLE_HIGH = 0x160,
	OF_PAN_CONFIG = 0x180,
	OF_PAN_TIMING = 0x1a0,
	OF_HDISPLAY = 0x1c0,
	OF_HSYNC = 0x1e0,
	OF_VDISPLAY = 0x240,
	OF_VSYNC = 0x260,
	OF_DBLBUF = 0x340,
};

struct pix_pll {
	unsigned int l2_div;
	unsigned int l1_loopc;
	unsigned int l1_frefc;
};

static struct pix_pll pll_cfg;
#define ls2k_writeq(v, a) *(volatile long long *)(a) = (v)
#define ls2k_readq(a) (*(volatile long long *)(a))
#define LO_OFF	0
#define HI_OFF	8

static void config_pll(unsigned long pll_base, struct pix_pll *pll_cfg)
{
	unsigned long long out;
	out = (1 << 7) | (1LL << 42) | (3 << 10) |
		((unsigned long long)(pll_cfg->l1_loopc) << 32) |
		((unsigned long long)(pll_cfg->l1_frefc) << 26);

	ls2k_writeq(0, pll_base + LO_OFF);
	ls2k_writeq(1 << 19, pll_base + LO_OFF);
	ls2k_writeq(out, pll_base + LO_OFF);
	ls2k_writeq(pll_cfg->l2_div, pll_base + HI_OFF);
	out = (out | (1 << 2));
	ls2k_writeq(out, pll_base + LO_OFF);

	while (!(ls2k_readq(pll_base + LO_OFF) & 0x10000)) ;

	ls2k_writeq((out | 1), pll_base + LO_OFF);
}

static unsigned int cal_freq(unsigned int pixclock_khz, struct pix_pll * pll_config)
{
	unsigned int pstdiv, loopc, frefc;
	unsigned long a, b, c;
	unsigned long min = 1000;

	for (pstdiv = 1; pstdiv < 64; pstdiv++) {
		a = (unsigned long)pixclock_khz * pstdiv;
		for (frefc = 3; frefc < 6; frefc++) {
			for (loopc = 24; loopc < 161; loopc++) {

				if ((loopc < 12 * frefc) ||
						(loopc > 32 * frefc))
					continue;

				b = 100000L * loopc / frefc;
				c = (a > b) ? (a - b) : (b - a);
				if (c < min) {

					pll_config->l2_div = pstdiv;
					pll_config->l1_loopc = loopc;
					pll_config->l1_frefc = frefc;
					printf("pll found pstdiv[0x%x], loopc[0x%x], frefc[0x%x]\n", pstdiv, loopc, frefc);

					return 1;
				}
			}

		}
	}
	printf("cal_freq error!!!\n");
	return 0;
}

int caclulatefreq(float PCLK)
{
	int pstdiv, ODF, LDF, idf, inta, intb;
	int mpd, modf, mldf, rodf;
	int out;
	float a, b, c, min;

	min = 100;
	for (pstdiv = 1; pstdiv < 32; pstdiv++)
		for (ODF = 1; ODF <= 8; ODF *= 2) {
			a = PCLK * pstdiv;
			b = a * ODF;
			LDF = ((int)(b / (50 * ODF))) * ODF;
			intb = 50 * LDF;
			inta = intb / ODF;
			c = b - intb;
			if (inta < 75 || inta > 1800)
				continue;
			if (intb < 600 || intb > 1800)
				continue;
			if (LDF < 8 || LDF > 225)
				continue;
			if (c < min) {
				min = c;
				mpd = pstdiv;
				modf = ODF;
				mldf = LDF;
			}
		}
	rodf = modf == 8 ? 3 : modf == 4 ? 2 : modf == 2 ? 1 : 0;
	out = (mpd << 24) + (mldf << 16) + (rodf << 5) + (5 << 2) + 1;
	printf("ODF=%d, LDF=%d, IDF=5, pstdiv=%d, prediv=1\n", rodf, mldf, mpd);
	return out;
}

int config_cursor(unsigned long base)
{
	/* framebuffer Cursor Configuration */
	outl(base + LS2K1000_FB_CUR_CFG_REG, 0x00020200);
	/* framebuffer Cursor Address */
	outl(base + LS2K1000_FB_CUR_ADDR_REG, ADDR_CURSOR);
	/* framebuffer Cursor Location */
	outl(base + LS2K1000_FB_CUR_LOC_ADDR_REG, 0x00060122);
	/* framebuffer Cursor Background */
	outl(base + LS2K1000_FB_CUR_BACK_REG, 0x00eeeeee);
	outl(base + LS2K1000_FB_CUR_FORE_REG, 0x00aaaaaa);
}

int config_fb(unsigned long base, int mode)
{
	int j;
	unsigned int chip_reg;


	/* Disable the panel 0 */
	outl((base + OF_BUF_CONFIG), 0x00000000);
	/* framebuffer configuration RGB565 */
	outl((base + OF_BUF_CONFIG), 0x00000003);
	outl((base + OF_BUF_ADDR), MEM_ADDR);
	outl(base + OF_DBLBUF, MEM_ADDR);
	outl((base + OF_DITHER_CONFIG), 0x00000000);
	outl((base + OF_DITHER_TABLE_LOW), 0x00000000);
	outl((base + OF_DITHER_TABLE_HIGH), 0x00000000);
	outl((base + OF_PAN_CONFIG), 0x80001311);
	outl((base + OF_PAN_TIMING), 0x00000000);

	outl((base + OF_HDISPLAY),
		  (vgamode[mode].hfl << 16) | vgamode[mode].hr);
	outl((base + OF_HSYNC),
		  0x40000000 | (vgamode[mode].hse << 16) | vgamode[mode].hss);
	outl((base + OF_VDISPLAY),
		  (vgamode[mode].vfl << 16) | vgamode[mode].vr);
	outl((base + OF_VSYNC),
		  0x40000000 | (vgamode[mode].vse << 16) | vgamode[mode].vss);

#if defined(CONFIG_VIDEO_32BPP)
	outl((base + OF_BUF_CONFIG), 0x00100104);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 4 + 255) & ~255);
#elif defined(CONFIG_VIDEO_16BPP)
	outl((base + OF_BUF_CONFIG), 0x00100103);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#elif defined(CONFIG_VIDEO_15BPP)
	outl((base + OF_BUF_CONFIG), 0x00100102);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#elif defined(CONFIG_VIDEO_12BPP)
	outl((base + OF_BUF_CONFIG), 0x00100101);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 2 + 255) & ~255);
#else /* 640x480-32Bits */
	outl((base + OF_BUF_CONFIG), 0x00100104);
	outl((base + OF_BUF_STRIDE), (FB_XSIZE * 4 + 255) & ~255);
#endif /* 32Bits */

}

unsigned long dc_init()
{
	int print_count;
	int i;
	int PIXEL_COUNT = DIS_WIDTH * DIS_HEIGHT + EXTRA_PIXEL;
	int MEM_SIZE;
	int init_R = 0;
	int init_G = 0;
	int init_B = 0;
	int j;
	int ii = 0, tmp = 0;
	int MEM_SIZE_3 = MEM_SIZE / 6;

	int line_length = 0;

	int print_addr;
	int print_data;
	int mode = -1;
	printf("enter dc_init...\n");

#if defined(CONFIG_VIDEO_32BPP)
	MEM_SIZE = PIXEL_COUNT * 4;
	line_length = FB_XSIZE * 4;
#elif defined(CONFIG_VIDEO_16BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#elif defined(CONFIG_VIDEO_15BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#elif defined(CONFIG_VIDEO_12BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = FB_XSIZE * 2;
#else
	MEM_SIZE = PIXEL_COUNT * 4;
	line_length = FB_XSIZE * 4;
#endif

	MEM_ADDR = (long)MEM_ptr & 0x0fffffff;
	if (MEM_ptr == NULL) {
		printf("frame buffer memory malloc failed!\n ");
		exit(0);
	}

	for (ii = 0; ii < 0x1000; ii += 4)
		outl(ADDR_CURSOR + ii, 0x88f31f4f);

	ADDR_CURSOR = (long)ADDR_CURSOR & 0x0fffffff;
	printf("frame buffer addr: %x \n", MEM_ADDR);
	/* Make DVO from panel1, it's the same with VGA*/

	for (i = 0; i < sizeof(vgamode) / sizeof(struct vga_struc); i++) {
		int out;
		if (vgamode[i].hr == FB_XSIZE && vgamode[i].vr == FB_YSIZE) {
			mode = i;
			//out = caclulatefreq(vgamode[i].pclk);
			//printf("out=%x\n", out);
			cal_freq(vgamode[i].pclk*1000, &pll_cfg);
		}//if
	}

	if (mode < 0) {
		printf("\n\n\nunsupported framebuffer resolution\n\n\n");
		return MEM_ptr;
	}

	config_pll(LS2K1000_PIXCLK0_CTRL0_REG, &pll_cfg);
	config_pll(LS2K1000_PIXCLK1_CTRL0_REG, &pll_cfg);
	config_fb(LS2K1000_DC_BASE + DC0_BASE_ADDR_OFF, mode);	//for dvo_0 1240
	config_fb(LS2K1000_DC_BASE + DC1_BASE_ADDR_OFF, mode);	//for dvo_1 1250
	config_cursor(LS2K1000_DC_BASE);

	printf("display controller reg config complete!\n");

	return MEM_ptr;
}

static int cmd_dc_freq(int argc, char **argv)
{
	float pclk;
	struct pix_pll ts_pll_cfg;
	if (argc < 2)
		return -1;
	pclk = strtoul(argv[1], 0, 0);
	cal_freq(pclk*1000, &ts_pll_cfg);
	unsigned int l2_div;
	unsigned int l1_loopc;
	unsigned int l1_frefc;
	printf("l1_loopc[0x%x], l1_frefc[0x%x], l2_div[0x%x]\n", ts_pll_cfg.l1_loopc, \
				ts_pll_cfg.l1_frefc ,ts_pll_cfg.l2_div);
	/* change to refclk */
#ifdef DC_FB0
	config_pll(LS2K1000_PIXCLK0_CTRL0_REG, &ts_pll_cfg);
#endif
#ifdef DC_FB1
	config_pll(LS2K1000_PIXCLK1_CTRL0_REG, &ts_pll_cfg);
#endif
	return 0;
}

static const Cmd Cmds[] = {
	{"MyCmds"},
	{"dc_freq", " pclk sysclk", 0, "config dc clk(khz)", cmd_dc_freq, 0, 99,
	 CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
