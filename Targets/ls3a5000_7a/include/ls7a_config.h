#ifndef _LS7A_CONFIG_H_
#define _LS7A_CONFIG_H_
//#define DEBUG_HT1
#ifdef  DEBUG_HT1
//#define PRINT_HT1_REG
//#define DEBUG_HT1_PARAM
#endif
#define CHECK_HT_PLL_MODE
#define CHECK_HT_PLL_LOCK
#define CHECK_7A_HT_PLL_LOCK

#define HT1_RECONNECT   1
//HT GEN1.0/3.0 cfg
#define HT1_GEN_CFG     3
//HT1 width cfg
#if defined(LS7A_2WAY_CONNECT) || (HT1_GEN_CFG == 1)
#define HT1_WIDTH_CFG   HT_WIDTH_CTRL_8BIT  //only support 8 bit
#else
#if	(TOT_NODE_NUM >= 8)
#define HT1_WIDTH_CFG   HT_WIDTH_CTRL_8BIT
#else
#define HT1_WIDTH_CFG   HT_WIDTH_CTRL_16BIT
#endif
#endif
//HT1 freq cfg
#if (HT1_GEN_CFG == 3)
#define LS7A_HT1_HARD_FREQ_CFG  HT_GEN3_FREQ_CTRL_1600M
#define LS3A_HT1_HARD_FREQ_CFG  ((chip_ver() == 0x43) ? HT_GEN3_FREQ_CTRL_1600M : HT_GEN3_FREQ_CTRL_400M) //1600
#define LS7A_HT1_SOFT_FREQ_CFG  (LS7A_HT_PLL_1600M | (0x1 << 1))
#define LS7A_HT1_SOFT_FREQ_CFG_C  (LS7A_C_HT_PLL_1600M | (0x1 << 1))
#define LS3A_HT1_SOFT_FREQ_CFG  (LS3A5000_HT_PLL_1600M | (0x1 << 1))
#else
#define LS7A_HT1_HARD_FREQ_CFG  HT_GEN1_FREQ_CTRL_800M
#define LS3A_HT1_HARD_FREQ_CFG  HT_GEN1_FREQ_CTRL_200M
//in HT GEN1 mode, define PLL freq to request freq x 2, for example, if you want to use HT1 800M, define HT_PLL_1600M
#define LS7A_HT1_SOFT_FREQ_CFG  (LS7A_HT_PLL_1600M | (0x1 << 1))
#define LS7A_HT1_SOFT_FREQ_CFG_C  (LS7A_C_HT_PLL_1600M | (0x1 << 1))
#define LS3A_HT1_SOFT_FREQ_CFG  (LS3A_HT_PLL_1600M | (0x1 << 1))
#endif

#define LS7A_GRAPHICS_DISABLE   0

//staticly disable some PCIE Ports, no matter whether there is device
#define LS7A_PCIE_F0_DISABLE    0
#define LS7A_PCIE_F1_DISABLE    0
#define LS7A_PCIE_H_DISABLE     0
#define LS7A_PCIE_G0_DISABLE    0
#define LS7A_PCIE_G1_DISABLE    0

#if (TOT_7A_NUM == 2) || (MULTI_7A_ON_PCIE == 1)
#define LS7A_ANOTHER_PCIE_F0_DISABLE    0
#define LS7A_ANOTHER_PCIE_F1_DISABLE    0
#define LS7A_ANOTHER_PCIE_H_DISABLE     0
#define LS7A_ANOTHER_PCIE_G0_DISABLE    0
#define LS7A_ANOTHER_PCIE_G1_DISABLE    0
#endif

#ifdef LS7A2000
#define LS7A_SATA_DISABLE   0
#define LS7A_SATA0_DISABLE  0
#define LS7A_SATA1_DISABLE  0
#define LS7A_SATA2_DISABLE  0
#define LS7A_SATA3_DISABLE  0
#define LS7A_XHCI_DISABLE   0
#define LS7A_IOMMU_DISABLE  0
#else
#define LS7A_SATA0_DISABLE  0
#define LS7A_SATA1_DISABLE  (LS7A_SATA0_DISABLE | 0)
#define LS7A_SATA2_DISABLE  (LS7A_SATA1_DISABLE | 0)
#endif
#define LS7A_USB0_DISABLE   0
#define LS7A_USB1_DISABLE   0
#define LS7A_GMAC0_DISABLE  0
#define LS7A_GMAC1_DISABLE  (LS7A_GMAC0_DISABLE | 0)
#define LS7A_LPC_DISABLE    0

//#define USE_PCIE_PAD_REFCLK
//#define USE_SATA_PAD_REFCLK
#define USE_USB_SYS_REFCLK

#if (!LS7A_GRAPHICS_DISABLE) && ((TOT_NODE_NUM < 8) || !defined(TY_MULTI_BOARD))
#ifdef LS7A2000
#define LS7A_GMEM_DISABLE	0
#define LS7A_GPU_DISABLE	0
#define LS7A_VPU_DISABLE	0
#define DEBUG_GMEM		0
#else
#define LS7A_GMEM_CFG	1
#define DEBUG_GMEM	0
#endif
#else
#ifdef LS7A2000
#define LS7A_GMEM_DISABLE	1
#define LS7A_GPU_DISABLE	1
#define LS7A_VPU_DISABLE	1
#else
#define LS7A_GMEM_CFG	0
#endif
#endif

#ifndef LS7A_CODE_CLOSURE
#if     (TOT_NODE_NUM <= 4)
#if	(defined(LS7A_2WAY_CONNECT) || (TOT_7A_NUM == 2) || (MULTI_7A_ON_PCIE == 1))
	char ls7a_link_id_buf[2] = {0, 1};
#else
	char ls7a_link_id_buf[1] = {0};
#endif
#elif (TOT_NODE_NUM == 8)
#ifdef LS7A_2WAY_CONNECT
	char ls7a_link_id_buf[2] = {0, 5};
#else
	char ls7a_link_id_buf[1] = {0};
#endif
#elif (TOT_NODE_NUM == 16)
#if (TOT_7A_NUM == 2) || (MULTI_7A_ON_PCIE == 1)
#ifdef LS7A_2WAY_CONNECT
	char ls7a_link_id_buf[4] = {0, 5, 0xa, 0xf};
#else
	char ls7a_link_id_buf[2] = {0, 5};
#endif
#else
#ifdef LS7A_2WAY_CONNECT
	char ls7a_link_id_buf[2] = {0, 0xa};
#else
	char ls7a_link_id_buf[1] = {0};
#endif
#endif
#endif

#if	(TOT_NODE_NUM == 1)
uint64_t DMA_NODE_ID_OFFSET = 44;
#elif	(TOT_NODE_NUM <= 4)
uint64_t DMA_NODE_ID_OFFSET = 38;
#elif	(TOT_NODE_NUM == 8)
uint64_t DMA_NODE_ID_OFFSET = 37;
#elif	(TOT_NODE_NUM == 16)
uint64_t DMA_NODE_ID_OFFSET = 36;
#endif
#endif /* LS7A_CODE_CLOSURE */

#ifdef LS7A_2WAY_CONNECT
#if     (TOT_NODE_NUM <= 4)
#define	DMA_DEST_HT		(0xa << 16)
#elif (TOT_NODE_NUM == 8)
#define	DMA_DEST_HT		(0xf0 << 16)
#elif (TOT_NODE_NUM == 16)
#define	DMA_DEST_HT		(0xff00 << 16)
#endif
#else
#define	DMA_DEST_HT		0
#endif

/*
 * LS7A RESOURCE CONFIG
 */
#define USE_OUTSIDE_REFCLK	0
#define USE_INSIDE_REFCLK	1

/* HT */
typedef struct {
	uint8_t ls3a_node_num;
	uint8_t ls7a_node_num; /* HT link 7A number */
	uint8_t ls7a_2way_connect;
	uint8_t ls7a_con_type; /* another 7A2000 pcie connections type |0 HT |1 F|2 H pcie| */
} ls7a_ht_table;

enum {
	LS7A_PLL0 = 0,
	LS7A_PLL1,
	LS7A_PLL2,
	LS7A_PLL3,
	LS7A_PLL4,
	LS7A1_PLL0,
	LS7A1_PLL1,
	LS7A1_PLL2,
	LS7A1_PLL3,
	LS7A1_PLL4,
};
/* PLL */
typedef struct {
	uint32_t pll_val;
	uint32_t div;
} ls7a_pll_table;

typedef struct {
	uint32_t val_0;
	uint32_t val_4;
	uint32_t val_8;
} pcie_phy_param_t;
#define LS7A_PCIE_X1_MODE	0x2
#define LS7A_PCIE_X4_MODE	0x4
#define LS7A_PCIE_X8_MODE	0x8
#define LS7A_PCIE_X16_MODE	0x10
#define LS7A_PCIE_ADAPT_MODE	0xe
#define LS7A_PCIE_FLIP_EN	0x10
#define LS7A_PCIE_NO_POWERDOWN	0x20
#define LS7A_PCIE_FORCE_X1_MODE	(LS7A_PCIE_X1_MODE | LS7A_PCIE_NO_POWERDOWN)
#define LS7A_PCIE_FORCE_X4_MODE	(LS7A_PCIE_X4_MODE | LS7A_PCIE_NO_POWERDOWN)
#define LS7A_PCIE_FORCE_X8_MODE	(LS7A_PCIE_X8_MODE | LS7A_PCIE_NO_POWERDOWN)
#define LS7A_PCIE_GEN1_MODE	0x1
#define LS7A_PCIE_GEN2_MODE	0x2
#define LS7A_PCIE_GEN3_MODE	0x3
#define LS7A_PCIE_EQ_MODE0	0x0
#define LS7A_PCIE_EQ_MODE1	0x1
#define LS7A_PCIE_EQ_MODE2	0x2
/* PCIE */
#ifdef LS7A2000
typedef struct {
	const char* name;
	int port_clk_disable;
	int ctrl_disable;
	int preset;
	int gen;
	int lane_mode;
	int eq_mode;
} pcie_desc;
#else
typedef struct {
	uint64_t bus_base;
	int first_port_num;
	const char* name;
	int port_bits;
	int disable;
	pcie_phy_param_t *phy_param;
	int mode;
} pcie_desc;
#endif

#define ENABLE_PCIEX8_CAL	0x10
#define PCIE_MAX_SIZE		12
typedef struct {
	uint8_t ref_clk;
	uint8_t x8_cal_en;
	uint64_t pcie_cfg_buffer[PCIE_MAX_SIZE];
	uint32_t num;
	pcie_desc* controller;
} ls7a_pcie_table;

/* SATA */
#ifdef LS7A2000
typedef struct {
	uint8_t disable;
} sata_desc;
#else
typedef struct {
	uint8_t disable;
	uint8_t ovrd_en;	//if OVRD_SATA_PHY
	uint32_t ovrd_val;
} sata_desc;
#endif

typedef struct {
	uint8_t ref_clk;
	sata_desc* controller;
} ls7a_sata_table;

/* USB */
#ifdef LS7A2000
typedef struct {
	uint8_t disable;
	uint32_t afetrim;
	union {
		struct {
			uint8_t ihstx : 4;
			uint8_t zhsdrv : 2;
		};
		uint8_t usb2_param;
	};
} usb_desc;
#else
typedef struct {
	uint8_t disable;
	uint8_t port0_ovrd_en;
	uint32_t port0_ovrd_val;
	uint8_t port1_ovrd_en;
	uint32_t port1_ovrd_val;
	uint8_t port2_ovrd_en;
	uint32_t port2_ovrd_val;
	uint8_t clk_frescale;
} usb_desc;
#endif

typedef struct {
	uint8_t ref_clk;
	usb_desc* controller;
} ls7a_usb_table;

/* GMAC */
typedef struct {
	uint8_t disable;
} gmac_desc;

/* IOMMU */
typedef struct {
	uint8_t disable;
} ls7a_iommu_table;

typedef struct {
	uint8_t ref_clk;
	gmac_desc* controller;
} ls7a_gmac_table;

/* DISPLAY */
#ifdef LS7A2000
typedef struct {
	uint8_t graphics_disable;
	uint8_t gmem_disable;
	uint8_t gpu_disable;
	uint8_t vpu_disable;
} ls7a_dc_table;
#else
typedef struct {
	uint8_t graphics_disable;
	uint8_t gmem_cfg;
} ls7a_dc_table;
#endif

typedef struct {
	uint32_t min_rpm;
	uint32_t max_rpm;
} ls7a_fan_desc;
/* MISC */
typedef struct {
	uint8_t	lpc_disable;
	ls7a_fan_desc fan;
} ls7a_misc_table;

typedef struct ls7a_resource_table {
	ls7a_ht_table		ht;
	ls7a_pll_table		pll[10];
	ls7a_pcie_table		pcie;
	ls7a_sata_table		sata;
	ls7a_usb_table		usb;
	ls7a_gmac_table		gmac;
	ls7a_dc_table		dc;
	ls7a_misc_table		misc;
	ls7a_iommu_table	iommu;
} ls7a_resource_table_t;

extern ls7a_resource_table_t ls7a_cfg_t;

#define LS3A_NODE_NUM		(ls7a_cfg_t.ht.ls3a_node_num)
#endif
