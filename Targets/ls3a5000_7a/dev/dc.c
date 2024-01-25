#include <pmon.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/malloc.h>
#include <machine/pio.h>
#include "cpu.h"
#include "ls7a_vbios.h"
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef signed long s32;
typedef signed short s16;
typedef signed char s8;
typedef int bool;
typedef unsigned long dma_addr_t;

#define writeb(val, addr) (*(volatile u8*)(addr) = (val))
#define writew(val, addr) (*(volatile u16*)(addr) = (val))
#define writel(val, addr) (*(volatile u32*)(addr) = (val))
#define readb(addr) (*(volatile u8*)(addr))
#define readw(addr) (*(volatile u16*)(addr))
#define readl(addr) (*(volatile u32*)(addr))

#define write_reg(addr,val) writel(val,addr)

#define DIS_WIDTH  LS_FB_XSIZE
#define DIS_HEIGHT LS_FB_YSIZE
#define EXTRA_PIXEL  0
#define LO_OFF	0
#define HI_OFF	8
static struct pix_pll {
	unsigned int l2_div;
	unsigned int l1_loopc;
	unsigned int l1_refc;
}pll_cfg;

#undef USE_GMEM
#ifdef USE_GMEM
static char *ADDR_CURSOR = PHYS_TO_CACHED(0x46000000);
static char *MEM_ptr = PHYS_TO_CACHED(0x42000000);
#else
static char *ADDR_CURSOR = PHYS_TO_CACHED(0x6000000);
static char *MEM_ptr = PHYS_TO_CACHED(0x7000000);
#endif
unsigned long dc_base_addr;
static unsigned int MEM_ADDR = 0;

struct vga_struc {
	float pclk;
	int hr, hss, hse, hfl;
	int vr, vss, vse, vfl;
} vgamode[] = {
        {       5.26,   320,    304,    336,    352,    240,    241,    244,    249,    },      /*"320x240_60.00" */
//	{	28.56,	640,	664,	728,	816,	480,	481,	484,	500,	},	/*"640x480_70.00" */
//	{	23.86,	640,	656,	720,	800,	480,	481,	484,	497,	},	/*"640x480_60.00" */
	{	25.2,	640,	656,	752,	800,	480,	490,	492,	525,	},	/*"640x480_60.00" */
	{	33.10,	640,	672,	736,	832,	640,	641,	644,	663,	},	/*"640x640_60.00" */
	{	39.69,	640,	672,	736,	832,	768,	769,	772,	795,	},	/*"640x768_60.00" */
	{	42.13,	640,	680,	744,	848,	800,	801,	804,	828,	},	/*"640x800_60.00" */
	{	21.91,  720,    728,    800,    880,    400,    401,    404,    415,	},	/*"720x400_60.00" */
	{	35.84,	800,	832,	912,	1024,	480,	481,	484,	500,	},	/*"800x480_70.00" */
	{	38.22,	800,	832,	912,	1024,	600,	601,	604,	622,	},	/*"800x600_60.00" */
	{	40.73,	800,	832,	912,	1024,	640,	641,	644,	663,	},	/*"800x640_60.00" */
	{	40.01,	832,	864,	952,	1072,	600,	601,	604,	622,	},	/*"832x600_60.00" */
	{	40.52,	832,	864,	952,	1072,	608,	609,	612,	630,	},	/*"832x608_60.00" */
	{	38.17,	1024,	1048,	1152,	1280,	480,	481,	484,	497,	},	/*"1024x480_60.00" */
	{	48.96,	1024,	1064,	1168,	1312,	600,	601,	604,	622,	},	/*"1024x600_60.00" */
	{	52.83,	1024,	1072,	1176,	1328,	640,	641,	644,	663,	},	/*"1024x640_60.00" */
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/*"1024x768_60.00" */
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/*"1024x768_60.00" */
	{	64.11,	1024,	1080,	1184,	1344,	768,	769,	772,	795,	},	/*"1024x768_60.00" */
	{	71.38,	1152,	1208,	1328,	1504,	764,	765,	768,	791,	},	/*"1152x764_60.00" */
	{	83.46,	1280,	1344,	1480,	1680,	800,	801,	804,	828,	},	/*"1280x800_60.00" */
	{	74.25,	1280,	1391,	1430,	1650,	720,	726,	730,	750,	},	/*"1280x720_60.00" HDMI Format 4*/
	{	98.60,	1280,	1352,	1488,	1696,	1024,	1025,	1028,	1057,	},	/*"1280x1024_55.00" */
	{	93.80,	1440,	1512,	1664,	1888,	800,	801,	804,	828,	},	/*"1440x800_60.00" */
	{	120.28,	1440,	1528,	1680,	1920,	900,	901,	904,	935,	},	/*"1440x900_67.00" */
        {       74.25,  1280,   1650,   1390,   1430,   720,    750,    725,    730,    },      /*"1280x720_60.00" */
        {       148.50, 1920,   2009,   2052,   2200,   1080,   1085,   1089,   1125    },      /*"1920x1080_60.00"  HDMI Format 16*/
       // {       172.80, 1920,   2040,   2248,   2576,   1080,   1081,   1084,   1118    },      /*"1920x1080_60.00" */
        {       311.83, 2560,   2744,   3024,   3488,   1440,   1441,   1444,   1490    },      /*"2560x1440_60.00" */
       //{       146.27, 2560,   2680,   2944,   3328,   1440,   1441,   1444,   1465    },      /*"2560x1440_30.00" */
	{       339.57, 3840,   4080,   4496,   5152,   2160,   2161,   2164,   2197    },      /*"3840x2160_30.00" */
       // {       148.50, 1920,   2009,   2052,   2200,   1080,   1085,   1089,   1125    },      /*"1920x1080_60.00"  HDMI Format 16*/
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
	OF_GAMMA_INDEX = 0x2a0,
	OF_GAMMA_DATA = 0x2c0,
	OF_DBLBUF = 0x340,
	OF_HDMI_ZONE_IDLE = 0x4c0,
	OF_HDMI_CTRL = 0x4e0,
	OF_AUDIO_N = 0x520,
	OF_AUDIO_CTS = 0x540,
	OF_AUDIO_CTS_CAL = 0x560,
	OF_AUDIO_INFO_FRAME = 0x580,
	OF_AUDIO_SAMPLE = 0x5a0,
	OF_HDMI_PHY = 0x5c0,
	OF_PHY_PLL = 0x5e0,
	OF_HDMI_PHY_PEC0 = 0x600,
	OF_HDMI_PHY_PEC1 = 0x620,
	OF_HDMI_PHY_PEC2 = 0x640,
	OF_HDMI_PHY_CAL = 0x680,
	OF_AVI_INFOFRAME0 = 0x6a0,
	OF_AVI_INFOFRAME_CTRL0 = 0x720
};

#define PLL_REF_CLK_MHZ		 100
#define PCLK_PRECISION_INDICATOR 10000

//for PLL using configuration: (refc, loopc, div). Like LS7A1000 PLL
//for C env without decimal fraction support.
static int calc_pll(unsigned int pixclock_khz)
{
	unsigned int refc_set[] = {1};
	unsigned int prec_set[] = {1, 5, 10, 50, 100};	//in 1/PCLK_PRECISION_INDICATOR
	unsigned int pstdiv, loopc, refc;
	int i, j;
	unsigned int precision_req, precision;
	unsigned int loopc_min, loopc_max, loopc_mid;
	unsigned long long real_dvo, req_dvo;
	int loopc_offset;

	if (pixclock_khz > 145) {
		for (i = 0; i < 5; i++)
			refc_set[i] = i + 1;
	}


	//try precsion from high to low
	for (j = 0; j < sizeof(prec_set) / sizeof(int); j++) {
		precision_req = prec_set[j];
		//try each refc
		for (i = 0; i < sizeof(refc_set) / sizeof(int); i++) {
			refc = refc_set[i];
			loopc_min = (3200 / PLL_REF_CLK_MHZ) * refc;  //1200 / (PLL_REF_CLK_MHZ / refc)
			loopc_max = (4800 / PLL_REF_CLK_MHZ) * refc;  //3200 / (PLL_REF_CLK_MHZ / refc)
			loopc_mid = (4000 / PLL_REF_CLK_MHZ) * refc;  //(loopc_min + loopc_max) / 2;

			loopc_offset = -1;
			//try each loopc
			for (loopc = loopc_mid; (loopc <= loopc_max) && (loopc >= loopc_min); loopc += loopc_offset) {
				if (loopc_offset < 0) {
					loopc_offset = -loopc_offset;
				} else {
					loopc_offset = -(loopc_offset + 1);
				}

				pstdiv = loopc * PLL_REF_CLK_MHZ * 1000 / refc / pixclock_khz;
				if ((pstdiv > 127) || (pstdiv < 1))
					continue;
				//real_freq / req_freq = (real_freq * pstdiv) / (req_freq * pstdiv) = (loopc * PLL_REF_CLK_MHZ * 1000 / refc) / (req_freq * pstdiv).
				//real_freq is float type which is not available, but read_freq * pstdiv is available.
				real_dvo = (loopc * PLL_REF_CLK_MHZ * 1000 / refc);
				req_dvo  = (pixclock_khz * pstdiv);
				precision = abs(real_dvo * PCLK_PRECISION_INDICATOR / req_dvo - PCLK_PRECISION_INDICATOR);

				if (precision < precision_req) {
					pll_cfg.l2_div = pstdiv;
					pll_cfg.l1_loopc = loopc;
					pll_cfg.l1_refc = refc;
					printf("for pixclock = %d khz, found: pstdiv = %d, "
							"loopc = %d, refc = %d, precision = %d / %d.\n", pixclock_khz,
							pll_cfg.l2_div, pll_cfg.l1_loopc, pll_cfg.l1_refc, precision+1, PCLK_PRECISION_INDICATOR);
					if (j > 1) {
						printf("Warning: PIX clock precision degraded to %d / %d\n", precision_req, PCLK_PRECISION_INDICATOR);
					}

					return 1;
				}
			}
		}
	}
	return 0;
}

int config_cursor(unsigned long base)
{
	write_reg(base + 0x1520, 0x00020200);
	write_reg(base + 0x1530, ADDR_CURSOR);
	write_reg(base + 0x1540, 0x00060122);
	write_reg(base + 0x1550, 0x00eeeeee);
	write_reg(base + 0x1560, 0x00aaaaaa);
}

//For LS7A1000, configure PLL
static void config_pll(unsigned long long pll_base, struct pix_pll pll_cfg)
{
	unsigned int val;

	//printf("div = %d, loopc = %d, refc = %d.\n", 
	//		pll_cfg.l2_div, pll_cfg.l1_loopc, pll_cfg.l1_refc);

	/* set sel_pll_out0 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 8);
	writel(val, pll_base + 0x4);
	/* pll_pd 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 13);
	writel(val, pll_base + 0x4);
	/* set_pll_param 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 11);
	writel(val, pll_base + 0x4);

	/* set new div ref, loopc, div out */
	/* clear old value first*/
	val = readl(pll_base + 0x4);
	val &= ~(0x7fUL << 0);
	val |= (pll_cfg.l1_refc << 0);
	writel(val, pll_base + 0x4);

	val = readl(pll_base + 0x0);
	val &= ~(0x7fUL << 0);
	val |= (pll_cfg.l2_div << 0);
	val &= ~(0x1ffUL << 21);
	val |= (pll_cfg.l1_loopc << 21);
	writel(val, pll_base + 0x0);

	/* set_pll_param 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 11);
	writel(val, pll_base + 0x4);
	/* pll_pd 0 */
	val = readl(pll_base + 0x4);
	val &= ~(1UL << 13);
	writel(val, pll_base + 0x4);
	/* wait pll lock */
	while (!(readl(pll_base + 0x4) & 0x80));
	/* set sel_pll_out0 1 */
	val = readl(pll_base + 0x4);
	val |= (1UL << 8);
	writel(val, pll_base + 0x4);
}

int config_fb(unsigned long base)
{
	int i, mode = -1;
	int j;
	unsigned long long confbus;


	confbus = readl(PHYS_TO_UNCACHED(0xefdfe00a810));
	confbus = PHYS_TO_UNCACHED(confbus & ~0xfULL);

	printf("confbus = %llx\n", confbus);
	for (i = 0; i < sizeof(vgamode) / sizeof(struct vga_struc); i++) {
		if (vgamode[i].hr == LS_FB_XSIZE && vgamode[i].vr == LS_FB_YSIZE) {
			mode = i;
			if (calc_pll((unsigned int)(vgamode[i].pclk * 1000))) {
				config_pll(confbus + 0x4b0, pll_cfg);
				config_pll(confbus + 0x4c0, pll_cfg);
			} else {
				printf("\n\nError: Fail to find a proper PLL configuration.\n\n");
			}
			break;
		}
	}

	if (mode < 0) {
		printf("\n\n\nunsupported framebuffer resolution\n\n\n");
		return;
	}

	/* Disable the panel 0 */
	write_reg((base + OF_BUF_CONFIG), 0x00000000);
	/* framebuffer configuration RGB565 */
	write_reg((base + OF_BUF_CONFIG), 0x00000003);
	write_reg((base + OF_BUF_ADDR), MEM_ADDR);
	write_reg(base + OF_DBLBUF, MEM_ADDR);
	write_reg((base + OF_DITHER_CONFIG), 0x00000000);
	write_reg((base + OF_DITHER_TABLE_LOW), 0x00000000);
	write_reg((base + OF_DITHER_TABLE_HIGH), 0x00000000);
	write_reg((base + OF_PAN_CONFIG), 0x80001311);
	write_reg((base + OF_PAN_TIMING), 0x00000000);

#ifdef LS7A2000	/* HDMI CONFIGURE */
	unsigned int val, val1;
	write_reg((base + OF_GAMMA_INDEX), 0x0);
	val = 0xffffff;
	val1 = 0x10101;
	for(i = 0; i < 256; i++) {
		write_reg((base + OF_GAMMA_DATA), val);
		val -= val1;
	}

	//enable HDMI
	//write_reg((base + OF_HDMI_CTRL), 0x4f); //set video preamble length = 4,and set VideoLGBDisable
	//write_reg((base + OF_HDMI_CTRL), 0x383); //video preamble length = 8
	write_reg((base + OF_HDMI_CTRL), 0x387); //video preamble length = 8
	//write_reg((base + OF_HDMI_CTRL), 0x3); //video preamble length = 8

	//HMDI zone idle
	readl(base + OF_HDMI_ZONE_IDLE) =  0x00400040;

	//Audio N
	write_reg((base + OF_AUDIO_N), 6272); // 44.1KHz * 4, dynamic update N && CTS value
	//write_reg((base + OF_AUDIO_N), 25088 | 0x80000000); // 148.5MHz, 44.1KHz * 4, static

	//Audio CTS
	//write_reg((base + OF_AUDIO_CTS), 28000); // 25.2MHZ, 176.4KHz, static fixed CTS mode

	//Set CTS Value Fixed
	//readl(base + OF_AUDIO_CTS) |=  0x20000000;

	//Enable Send CTS
	readl(base + OF_AUDIO_CTS) =  0x80000000;

	//Update CTS Value
	//readl(base + OF_AUDIO_CTS) |=  0x40000000;

	//Audio CTS dynamic
	//readl(base + OF_AUDIO_CTS) &= 0x7fffffff;

	//Audio AIF
	write_reg((base + OF_AUDIO_INFO_FRAME), 0x11); //enable AIF,set freq,and set CC = 1, CA = 0

	//Update AIF
	readl(base + OF_AUDIO_INFO_FRAME) |=  0x4;

	//Audio Sample Packet
	write_reg((base + OF_AUDIO_SAMPLE), 0x1);

	if (((vgamode[mode].hr == 3840) && (vgamode[mode].vr == 2160)) ||
		((vgamode[mode].hr == 2560) && (vgamode[mode].vr == 1440))) {
		write_reg((base + OF_PHY_PLL), 0x10a20);
		write_reg((base + OF_PHY_PLL), 0x10a21);
	} else {
		write_reg((base + OF_PHY_PLL), ((0x8 << 1) | (0x28 << 6) | (0x1 << 13) | 0x0));
		write_reg((base + OF_PHY_PLL), ((0x8 << 1) | (0x28 << 6) | (0x1 << 13) | 0x1));
	}
	/* wait pll lock */
	printf("wait pll lock\n");
//	while(!(readl(base + OF_PHY_PLL) & 0x10000));
	printf("pll lock down\n");
	write_reg((base + OF_HDMI_PHY), 0xf03);

	write_reg((base + OF_HDMI_PHY_CAL), 0x4f000ff0);
	if ((vgamode[mode].hr == 640) && (vgamode[mode].vr == 480)) {
		readl(base + OF_AVI_INFOFRAME0) |= (1 << 24);
	} else if ((vgamode[mode].hr == 1280) && (vgamode[mode].vr == 720)) {
		readl(base + OF_AVI_INFOFRAME0) |= (4 << 24);
	} else if ((vgamode[mode].hr == 1920) && (vgamode[mode].vr == 1080)) {
	        write_reg((base + OF_HDMI_PHY_PEC0), 0xff);
	        write_reg((base + OF_HDMI_PHY_PEC1), 0xff);
	        write_reg((base + OF_HDMI_PHY_PEC2), 0xff);
	        readl(base + OF_AVI_INFOFRAME0) |= (16 << 24);
	} else if ((vgamode[mode].hr == 2560) && (vgamode[mode].vr == 1440)) {
	        write_reg((base + OF_HDMI_PHY_PEC0), 0x777f);
	        write_reg((base + OF_HDMI_PHY_PEC1), 0x777f);
	        write_reg((base + OF_HDMI_PHY_PEC2), 0x777f);
		write_reg((base + OF_HDMI_PHY_CAL), 0x4f00f0f0);
	} else if ((vgamode[mode].hr == 3840) && (vgamode[mode].vr == 2160)) {
	        write_reg((base + OF_HDMI_PHY_PEC0), 0x777f);
	        write_reg((base + OF_HDMI_PHY_PEC1), 0x777f);
	        write_reg((base + OF_HDMI_PHY_PEC2), 0x777f);
		write_reg((base + OF_HDMI_PHY_CAL), 0x4f00f0f0);
	} else {
		printf("NOTE: HDMI resolution not support\n");
	}
	readl(base + OF_AVI_INFOFRAME_CTRL0) |= 0x5;
#endif

	write_reg((base + OF_HDISPLAY),
		  (vgamode[mode].hfl << 16) | vgamode[mode].hr);
	write_reg((base + OF_HSYNC),
		  0x40000000 | (vgamode[mode].hse << 16) | vgamode[mode].hss);
	write_reg((base + OF_VDISPLAY),
		  (vgamode[mode].vfl << 16) | vgamode[mode].vr);
	write_reg((base + OF_VSYNC),
		  0x40000000 | (vgamode[mode].vse << 16) | vgamode[mode].vss);

#if defined(CONFIG_VIDEO_32BPP)
	write_reg((base + OF_BUF_CONFIG), 0x00100104);
	write_reg((base + OF_BUF_STRIDE), (LS_FB_XSIZE * 4 + 255) & ~255);
#elif defined(CONFIG_VIDEO_16BPP)
	write_reg((base + OF_BUF_CONFIG), 0x00100103);
#ifdef LS7A2000
	write_reg((base + OF_BUF_STRIDE), (LS_FB_XSIZE * 2));
#else
	write_reg((base + OF_BUF_STRIDE), (LS_FB_XSIZE * 2 + 255) & ~255);
#endif
#elif defined(CONFIG_VIDEO_15BPP)
	write_reg((base + OF_BUF_CONFIG), 0x00100102);
	write_reg((base + OF_BUF_STRIDE), (LS_FB_XSIZE * 2 + 255) & ~255);
#elif defined(CONFIG_VIDEO_12BPP)
	write_reg((base + OF_BUF_CONFIG), 0x00100101);
	write_reg((base + OF_BUF_STRIDE), (LS_FB_XSIZE * 2 + 255) & ~255);
#else /* 640x480-32Bits */
	write_reg((base + OF_BUF_CONFIG), 0x00100104);
	write_reg((base + OF_BUF_STRIDE), (LS_FB_XSIZE * 4 + 255) & ~255);
#endif /* 32Bits */

}

#ifdef LS7A2000
void hdmi_codec_init(void)
{
	/* HDMI AUDIO codec init */
	unsigned long long reg_base = readl(PHYS_TO_UNCACHED(0xefdfe003210)) & ~0xf;
	readl(PHYS_TO_UNCACHED(reg_base) + 0x2404) = 1;
	readl(PHYS_TO_UNCACHED(reg_base) + 0x2400) = -1;
	/* tmp eld for hdmi codec test */
#if 0
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2200) = 0x10;
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2202) = 0x5;
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2205) = 0x10;
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2207) = 0x1;
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2214) = 0x9;
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2215) = 0x7;
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2216) = 0x7;
#endif
	readl(PHYS_TO_UNCACHED(reg_base) + 0x2200) = 0x00050010;
	readl(PHYS_TO_UNCACHED(reg_base) + 0x2204) = 0x01001000;
	readl(PHYS_TO_UNCACHED(reg_base) + 0x2214) = 0x70709;
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2254) = 1;

	readl(PHYS_TO_UNCACHED(reg_base) + 0x2300) = 0x00050010;
	readl(PHYS_TO_UNCACHED(reg_base) + 0x2304) = 0x01001000;
	readl(PHYS_TO_UNCACHED(reg_base) + 0x2314) = 0x70709;
	readb(PHYS_TO_UNCACHED(reg_base) + 0x2354) = 1;

	/* dc base */
	reg_base = readl(PHYS_TO_UNCACHED(0xefdfe003110)) & ~0xf;
	/* enable HDMI hotplug codec f09 command need it */
	readl(PHYS_TO_UNCACHED(reg_base) + 0x1570) |= (3 << 29);
}
#endif

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

	unsigned long dvo0_base_addr, dvo1_base_addr;

#ifdef LS7A2000
	hdmi_codec_init();
#endif
	printf("enter dc_init...\n");

#if defined(CONFIG_VIDEO_32BPP)
	MEM_SIZE = PIXEL_COUNT * 4;
	line_length = LS_FB_XSIZE * 4;
#elif defined(CONFIG_VIDEO_16BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = LS_FB_XSIZE * 2;
#elif defined(CONFIG_VIDEO_15BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = LS_FB_XSIZE * 2;
#elif defined(CONFIG_VIDEO_12BPP)
	MEM_SIZE = PIXEL_COUNT * 2;
	line_length = LS_FB_XSIZE * 2;
#else
	MEM_SIZE = PIXEL_COUNT * 4;
	line_length = LS_FB_XSIZE * 4;
#endif

	MEM_ADDR = (long)MEM_ptr & 0x0fffffff;
#ifdef USE_GMEM
	MEM_ADDR |= 0x40000000;
#endif
	if (MEM_ptr == NULL) {
		printf("frame buffer memory malloc failed!\n ");
		exit(0);
	}

	for (ii = 0; ii < 0x1000; ii += 4)
		*(volatile unsigned int *)(ADDR_CURSOR + ii) = 0x88f31f4f;

	ADDR_CURSOR = (long)ADDR_CURSOR & 0x0fffffff;
	printf("frame buffer addr: %x \n", MEM_ADDR);

	/* get dc base addr from pci config space */
	dc_base_addr = *(volatile unsigned int *)PHYS_TO_UNCACHED(0xefdfe003110);
	dc_base_addr = PHYS_TO_UNCACHED(dc_base_addr & ~0xfULL);
	dvo0_base_addr = dc_base_addr + 0x1240;
	dvo1_base_addr = dc_base_addr + 0x1250;

	/* dvo1 */
	config_fb(dvo1_base_addr);
	config_cursor(dvo1_base_addr);
	/* dvo0 */
	config_fb(dvo0_base_addr);
	config_cursor(dvo0_base_addr);
#ifdef LS7A2000
	/*config vga hotplug*/
	//readl(dc_base_addr + 0x1bb0) = ((0x35 << 8) | (0x3 << 2)); //4.02k, 150ohm
	readl(dc_base_addr + 0x1bb0) = ((0x63 << 8) | (0x3 << 2)); //4.02k, 75ohm or two 150ohm parallel

#endif
	printf("display controller reg config complete!\n");

	return MEM_ptr;
}
void dc_close(void)
{
	/* ensure this code have valid address */
	dc_base_addr = *(volatile unsigned int *)PHYS_TO_UNCACHED(0xefdfe003110);
	dc_base_addr = PHYS_TO_UNCACHED(dc_base_addr & ~0xfULL);
	//disable dc out put
	readl(dc_base_addr + 0x1240 + OF_BUF_CONFIG) &= ~(1 << 8);
	readl(dc_base_addr + 0x1250 + OF_BUF_CONFIG) &= ~(1 << 8);
}


void cmd_dual(int argc,char **argv)
{
	unsigned int num, num1, num2;
	unsigned long i, count, count1, fbaddress, fbaddress1;
	unsigned long dvo0_base_addr, dvo1_base_addr;

	/* get dc base addr from pci config space */
	dvo0_base_addr = dc_base_addr + 0x1240;
	dvo1_base_addr = dc_base_addr + 0x1250;

	fbaddress  = (unsigned long)MEM_ptr;
	fbaddress1 = (unsigned long)MEM_ptr + 0x800000;
	printf("frame buffer addr: 0x%lx 0x%lx\n", fbaddress, fbaddress1);
	num  = (unsigned int)strtoul(argv[1], 0, 0);
	num1 = (unsigned int)strtoul(argv[2], 0, 0);
	num2 = (unsigned int)strtoul(argv[3], 0, 0);
	count = (LS_FB_XSIZE * LS_FB_YSIZE);

	if(num2 == 1) {
		write_reg(dvo1_base_addr  + 0x20, fbaddress1);
		write_reg(dvo1_base_addr  + 0x340, fbaddress1);
	} else {
		write_reg(dvo0_base_addr  + 0x20, fbaddress1);
		write_reg(dvo0_base_addr  + 0x340, fbaddress1);
	}
	if(num2 == 1280) {
		for(i = 0; i < count; i++) {
#if defined(CONFIG_VIDEO_32BPP)
			readl(fbaddress + i * 4) = num;
#elif defined(CONFIG_VIDEO_16BPP)
			readl(fbaddress + i * 2) = num;
#endif
		}
		count1 = (1280 * 720);
		for(i = 0; i < count1; i++) {
#if defined(CONFIG_VIDEO_32BPP)
			readl(fbaddress1 + i * 4) = num1;
#elif defined(CONFIG_VIDEO_16BPP)
			readl(fbaddress1 + i * 2) = num1;
#endif
		}
	} else {
		for(i = 0; i < count; i++) {
#if defined(CONFIG_VIDEO_32BPP)
			readl(fbaddress + i * 4) = num;
			readl(fbaddress1 + i * 4) = num1;
#elif defined(CONFIG_VIDEO_16BPP)
			readl(fbaddress + i * 2) = num;
			readl(fbaddress1 + i * 2) = num1;
#endif
		}
	}
}

void cmd_dcch(int argc,char **argv)
{
	unsigned int num;
	unsigned long i, n, count, time;
	unsigned long fbaddress;

	time = (unsigned int)strtoul(argv[1], 0, 0);
	printf("argc = 1, time %d ms\n", time);
	fbaddress = (unsigned long)MEM_ptr;
	printf("frame buffer addr: 0x%lx\n", fbaddress);
	count = (LS_FB_XSIZE * LS_FB_YSIZE);
	num = 0;
	while(1) {
		for(i = 0; i < count; i++) {
#if defined(CONFIG_VIDEO_32BPP)
			readl(fbaddress + i * 4) = num;
			num++;
			if(num == 0x1000000)
				num = 0;
#elif defined(CONFIG_VIDEO_16BPP)
			readw(fbaddress + i * 2) = num;
			num++;
			if(num == 0x10000)
				num = 0;
#endif
		}
		printf("wait a moment\n");
		delay(1000 * time);
	}
}

void cmd_dcselc(int argc,char **argv)
{
	unsigned int num, num1;
	unsigned long i, count, fbaddress;

	fbaddress = (unsigned long)MEM_ptr;
	printf("frame buffer addr: 0x%lx\n", fbaddress);

	if(argc == 2) {
		num = (unsigned int)strtoul(argv[1], 0, 0);
#if defined(CONFIG_VIDEO_32BPP)
		switch(num) {
		case 1:
			num = 0x00ff0000;
			break;
		case 2:
			num = 0x0000ff00;
			break;
		case 3:
			num = 0x000000ff;
			break;
		}
#elif defined(CONFIG_VIDEO_16BPP)
		switch(num) {
		case 1:
			num = 0xf800;
			break;
		case 2:
			num = 0x07e0;
			break;
		case 3:
			num = 0x001f;
			break;
		}
#endif
		count = (LS_FB_XSIZE * LS_FB_YSIZE);
		printf("argc = 2, num = 0x%x, count 0x%lx\n", num, count);

		for(i = 0; i < count; i++)
#if defined(CONFIG_VIDEO_32BPP)
			readl(fbaddress + i * 4) = num;
#elif defined(CONFIG_VIDEO_16BPP)
			readw(fbaddress + i * 2) = num;
#endif
	} else if(argc == 3) {
		num  = (unsigned int)strtoul(argv[1], 0, 0);
		num1 = (unsigned int)strtoul(argv[2], 0, 0);
		count = (LS_FB_XSIZE * LS_FB_YSIZE);
		for(i = 0; i < count; i++) {
#if defined(CONFIG_VIDEO_32BPP)
			if((i % 640) < 320) {
				readl(fbaddress + i * 4) = num;
			} else {
				readl(fbaddress + i * 4) = num1;
			}
#elif defined(CONFIG_VIDEO_16BPP)
			if((i % 640) < 320) {
				readw(fbaddress + i * 2) = num;
			} else {
				readw(fbaddress + i * 2) = num1;
			}
#endif
		}
	}
}

void cmd_dctest(int argc,char **argv)
{
	unsigned int num, num1, num2, num3;
	unsigned long i, n, count, time;
	unsigned long fbaddress;

	fbaddress = (unsigned long)MEM_ptr;
	printf("frame buffer addr: 0x%lx\n", fbaddress);

	if(argc == 5) {
		num1 = (unsigned int)strtoul(argv[1], 0, 0);
		num2 = (unsigned int)strtoul(argv[2], 0, 0);
		num3 = (unsigned int)strtoul(argv[3], 0, 0);
		time = (unsigned int)strtoul(argv[4], 0, 0);
		count = (LS_FB_XSIZE * LS_FB_YSIZE);
		printf("argc = 5, num1 0x%x, num2 0x%x num3 0x%x time %d ms\n", num1, num2, num3, time);

		n = 1;
		while(1) {
			if(n%3 == 0) {
				num = num1;
				printf("set once\n");
			} else if(n%3 == 1) {
				num = num2;
				printf("set twice\n");
			} else if(n%3 == 2) {
				num = num3;
				printf("set third\n");
			}

			for(i = 0; i < count; i++)
#if defined(CONFIG_VIDEO_32BPP)
				readl(fbaddress + i * 4) = num;
#elif defined(CONFIG_VIDEO_16BPP)
				readw(fbaddress + i * 2) = num;
#endif
			n++;
			printf("wait a moment\n");
			delay(1000 * time);
		}
	} else if(argc == 3) {
		num = (unsigned int)strtoul(argv[1], 0, 0);
		count = (unsigned int)strtoul(argv[2], 0, 0);
		printf("argc = 3, num = 0x%x, count 0x%lx\n", num, count);

		for(i = 0; i < count; i++)
#if defined(CONFIG_VIDEO_32BPP)
			readl(fbaddress + i * 4) = num;
#elif defined(CONFIG_VIDEO_16BPP)
			readw(fbaddress + i * 2) = num;
#endif
	} else if(argc == 2) {
		time = (unsigned int)strtoul(argv[1], 0, 0);
		count = (LS_FB_XSIZE * LS_FB_YSIZE);
		printf("argc = 1, count 0x%lx time %d ms\n", count, time);
		n = 1;
		while(1) {
#if defined(CONFIG_VIDEO_32BPP)
			if(n%3 == 0) {
				num = 0x00ff0000;
				printf("set red\n");
			} else if(n%3 == 1) {
				num = 0x0000ff00;
				printf("set green\n");
			} else if(n%3 == 2) {
				num = 0x000000ff;
				printf("set blue\n");
			}
#elif defined(CONFIG_VIDEO_16BPP)
			if(n%3 == 0) {
				num = 0xf800;
				printf("set red\n");
			} else if(n%3 == 1) {
				num = 0x07e0;
				printf("set green\n");
			} else if(n%3 == 2) {
				num = 0x001f;
				printf("set blue\n");
			}
#endif

			for(i = 0; i < count; i++)
#if defined(CONFIG_VIDEO_32BPP)
			readl(fbaddress + i * 4) = num;
#elif defined(CONFIG_VIDEO_16BPP)
			readw(fbaddress + i * 2) = num;
#endif
			n++;
			printf("wait a moment\n");
			delay(1000 * time);
		}
	}
}


cmd_dctestxx(void)
{

	unsigned int num, num1, num2, num3;
	unsigned long i, n, count, time;
	unsigned long fbaddress;

	fbaddress = (unsigned long)MEM_ptr;
	printf("frame buffer addr: 0x%lx\n", fbaddress);

	count = (LS_FB_XSIZE * LS_FB_YSIZE);
	printf("argc = 1, count 0x%lx\n", count);
	n = 1;
	while(1) {
		if(n%3 == 0) {
			num = 0xf800;
			printf("set red\n");
		} else if(n%3 == 1) {
			num = 0x07e0;
			printf("set green\n");
		} else if(n%3 == 2) {
			num = 0x001f;
			printf("set blue\n");
		}

		for(i = 0; i < count; i++)
			readw(MEM_ptr + i * 2) = num;
		n++;
		printf("wait a moment\n");
		delay(3000000);
	}
}


cmd_dc_color(void)
{

	unsigned long long int num, num1, num2, num3;
	unsigned long long int i, n, count, time;
	unsigned long long int fbaddress;

	fbaddress = (unsigned long long int)MEM_ptr;
	printf("frame buffer addr: 0x%lx\n", fbaddress);

	count = (LS_FB_XSIZE * LS_FB_YSIZE);
	printf("argc = 1, count 0x%lx\n", count);
	n = 1;
	num = 0;
	while(1) {

		for(i = 0; i < count; i++)
			readw(MEM_ptr + i * 2) = num;
		num += 4;
		printf("wait a moment\n");
		delay(300000);
	}
}

void cmd_shut(int argc,char **argv)
{
	readl(dc_base_addr + 0x1bb0) &= ~(1 << 4); //vga

	uint64_t base = 0x8000000010010000;
	readb(base + 0x770) &= ~(1 << 24); //gmac0 phy
	readb(base + 0x430) &= ~((1 << 5) | (1 << 7) | (1 << 11) | (1 << 15) | (1 << 19)); //gmac0 gmac1 usb20 usb21 usb3
}

void cmd_close_hdmi(int argc,char **argv)
{
	unsigned int num;
	num = (unsigned int)strtoul(argv[1], 0, 0);
	/* ensure this code have valid address */
	dc_base_addr = *(volatile unsigned int *)PHYS_TO_UNCACHED(0xefdfe003110);
	dc_base_addr = PHYS_TO_UNCACHED(dc_base_addr & ~0xfULL);

	uint64_t base = 0x8000000010010000;
	if(num == 0) {
		readl(base + 0x4b4) = 0x2f81;
		//disable dc out put
		readl(dc_base_addr + 1800) = 0xf02;
	} else {
		readl(base + 0x4c4) = 0x2f81;
		readl(dc_base_addr + 1810) = 0xf02;
	}
}

/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Dc"},
	{"dc_color", "", 0, "7A2000 DC controler", cmd_dc_color, 1, 99, 0},
	{"dctest", "", 0, "LS7A dc fb test ", cmd_dctest, 1, 99, 0},
	{"dcselc", "", 0, "LS7A dc fb one color", cmd_dcselc, 1, 99, 0},
	{"dcch", "", 0, "LS7A dc fb color change", cmd_dcch, 1, 99, 0},
	{"dcdual", "", 0, "LS7A dc fb color change", cmd_dual, 1, 99, 0},
	{"pdxshy", "", 0, "shut down others ctrl", cmd_shut, 1, 99, 0},
	{"dcclose", "", 0, "shut down others ctrl", cmd_close_hdmi, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
