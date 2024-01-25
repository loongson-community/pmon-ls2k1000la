#ifndef _LS7A_H
#define _LS7A_H

#define HT1_CTRL_CONF_BASE  PHYS_TO_UNCACHED(0x0efdfb000000)
#define HT1_CONF_BASE       PHYS_TO_UNCACHED(0x0efdfe000000)
#define HT1_IO_BASE_ADDR    PHYS_TO_UNCACHED(0x0efdfc000000)
#define HT1_MEM_BASE_ADDR   PHYS_TO_UNCACHED(0x0e0000000000)

#define LS7A_UC_ACC

#define LS7A_PLL_DIV_REFC_OFFSET 0
#define LS7A_PLL_LOCK_OFFSET     7
#define LS7A_PLL_SEL0_OFFSET     8
#define LS7A_PLL_SEL1_OFFSET     9
#define LS7A_PLL_SEL2_OFFSET     10
#define LS7A_PLL_SET_OFFSET      11
#define LS7A_PLL_BYPASS_OFFSET   12
#define LS7A_PLL_PD_OFFSET       13

#define LS7A_GMEM_DIV_MODE_OFFSET   18
#define LS7A_GMEM_RESETn_OFFSET     17
#define LS7A_GMEM_DIV_RESETn_OFFSET 16

#define LS7A_GMEM_DIV_MODE       1

#define LS7A_PLL_DIV0_OFFSET     0
#define LS7A_PLL_DIV1_OFFSET     7
#define LS7A_PLL_DIV2_OFFSET     14
#define LS7A_PLL_LOOPC_OFFSET    21

#define LS7A_PLL_VALUE(LOOPC, DIV2, DIV1, DIV0) ((LOOPC << LS7A_PLL_LOOPC_OFFSET) | (DIV2 << LS7A_PLL_DIV2_OFFSET) | (DIV1 << LS7A_PLL_DIV1_OFFSET) | (DIV0 << LS7A_PLL_DIV0_OFFSET))

#define UART_BASE_ADDR_OFFSET   0x00000
#define I2C_BASE_ADDR_OFFSET    0x10000
#define PWM_BASE_ADDR_OFFSET    0x20000
#define ACPI_BASE_ADDR_OFFSET   0x50000
#define GPIO_BASE_ADDR_OFFSET   0x60000

#define INT_BASE_ADDR           0x10000000
#define HPET_BASE_ADDR          0x10001000
#define LPC_CNTL_BASE_ADDR      0x10002000
#define CONFBUS_BASE_ADDR       0x10010000
#define MISC_BASE_ADDR          0x10080000
#define LPC_MEM_BASE_ADDR       0x12000000

#define LS7A_CONFBUS_BASE_ADDR  (HT1_MEM_BASE_ADDR | CONFBUS_BASE_ADDR)
#define LS7A_MISC_BASE_ADDR     (HT1_MEM_BASE_ADDR | MISC_BASE_ADDR)

/* temp gmem device address 0x40000000 */
#define TEMP_GMEM_ADDR  0x40000000
#define LS7A_GMEM_TEMP_ADDR (HT1_MEM_BASE_ADDR | 0x40000000)

/* confbus address */
#define CONF_HT_CLKEN_OFFSET		0x418
#define CONF_HT_ROUTE_OFFSET		0x41c
#define CONF_NB_OFFSET			0x420
#define CONF_SB_OFFSET			0x430
#define CONF_PAD_OFFSET			0x438
#define CONF_PLL0_OFFSET		0x480
#define CONF_PLL1_OFFSET		0x490
#define CONF_PLL2_OFFSET		0x4a0
#define CONF_PLL3_OFFSET		0x4b0
#define CONF_PLL4_OFFSET		0x4c0
#define CONF_PCIE_F0_REG_OFFSET		0x588
#define CONF_PCIE_F0_PHY_OFFSET		0x590
#define CONF_PCIE_F1_REG_OFFSET		0x5a8
#define CONF_PCIE_F1_PHY_OFFSET		0x5b0
#define CONF_PCIE_H_REG_OFFSET		0x5c8
#define CONF_PCIE_H_PHY_LO_OFFSET	0x5d0
#define CONF_PCIE_H_PHY_HI_OFFSET	0x5d8
#define CONF_PCIE_G0_REG_OFFSET		0x5e8
#define CONF_PCIE_G0_PHY_LO_OFFSET	0x5f0
#define CONF_PCIE_G0_PHY_HI_OFFSET	0x5f8
#define CONF_PCIE_G1_REG_OFFSET		0x608
#define CONF_PCIE_G1_PHY_LO_OFFSET	0x610
#define CONF_PCIE_G1_PHY_HI_OFFSET	0x618
#define CONF_CHIP_ID_OFFSET		0x3ff8
/* configure offset */
#define CONF_DEFAULT_ROUTE_SB_OFFSET    0
#define CONF_DEFAULT_ROUTE_NB_OFFSET    0
#define CONF_SOFT_RESET_GPU_OFFSET      4
#define CONF_DISABLE_GMEM_CONFSPACE_OFFSET    8

/* PAD sel inner offset */
#define HDA_ENABLE_OFFSET   11
#define AC97_ENABLE_OFFSET  12
#define UART_ENABLE_OFFSET  28
#define	HT1_CONF_BASE_ADDR	PHYS_TO_UNCACHED(0x0efdfe000000)
/* PCI device header */
#define HEADER_ADDR(X,Y)        (HT1_CONF_BASE_ADDR | (X << 11) | (Y << 8))
#define MISC_HEADER_ADDR        HEADER_ADDR(2, 0)
#define GMAC0_HEADER_ADDR       HEADER_ADDR(3, 0)
#define GMAC1_HEADER_ADDR       HEADER_ADDR(3, 1)
#define USB0_OHCI_HEADER_ADDR   HEADER_ADDR(4, 0)
#define USB0_EHCI_HEADER_ADDR   HEADER_ADDR(4, 1)
#define USB1_OHCI_HEADER_ADDR   HEADER_ADDR(4, 0)
#define USB1_EHCI_HEADER_ADDR   HEADER_ADDR(4, 1)
#define GPU_HEADER_ADDR         HEADER_ADDR(6, 0)
#define DC_HEADER_ADDR          HEADER_ADDR(6, 1)
#define HDA_HEADER_ADDR         HEADER_ADDR(7, 0)
#define AC97_HEADER_ADDR        HEADER_ADDR(7, 1)
#define SATA0_HEADER_ADDR       HEADER_ADDR(8, 0)
#define SATA1_HEADER_ADDR       HEADER_ADDR(8, 1)
#define SATA2_HEADER_ADDR       HEADER_ADDR(8, 2)
#define CONFBUS_HEADER_ADDR     HEADER_ADDR(21, 0)
#define SPI_HEADER_ADDR         HEADER_ADDR(22, 0)
#define LPC_HEADER_ADDR         HEADER_ADDR(23, 0)

#define GPIO0_OEN_OFFSET          0x00
#define GPIO1_OEN_OFFSET          0x08
#define GPIO0_O_OFFSET            0x10
#define GPIO1_O_OFFSET            0x18
#define GPIO0_I_OFFSET            0x20
#define GPIO1_I_OFFSET            0x28
#define GPIO0_INT_OFFSET          0x30
#define GPIO1_INT_OFFSET          0x38

#define GPIO_OEN_OFFSET          0x800
#define GPIO_O_OFFSET            0x900
#define GPIO_I_OFFSET            0xa00
#define GPIO_INT_OFFSET          0xb00

#define LS7A_MISC_BASE			PHYS_TO_UNCACHED(0x10080000)
/* RTC regs */
#define LS7A_RTC_REG_BASE		(LS7A_MISC_BASE + 0x50100)
#define LS7A_TOY_TRIM_REG		(LS7A_RTC_REG_BASE + 0x0020)
#define LS7A_TOY_WRITE0_REG		(LS7A_RTC_REG_BASE + 0x0024)
#define LS7A_TOY_WRITE1_REG		(LS7A_RTC_REG_BASE + 0x0028)
#define LS7A_TOY_READ0_REG		(LS7A_RTC_REG_BASE + 0x002c)
#define LS7A_TOY_READ1_REG		(LS7A_RTC_REG_BASE + 0x0030)
#define LS7A_TOY_MATCH0_REG		(LS7A_RTC_REG_BASE + 0x0034)
#define LS7A_TOY_MATCH1_REG		(LS7A_RTC_REG_BASE + 0x0038)
#define LS7A_TOY_MATCH2_REG		(LS7A_RTC_REG_BASE + 0x003c)
#define LS7A_RTC_CTRL_REG		(LS7A_RTC_REG_BASE + 0x0040)
#define LS7A_RTC_TRIM_REG		(LS7A_RTC_REG_BASE + 0x0060)
#define LS7A_RTC_WRITE0_REG		(LS7A_RTC_REG_BASE + 0x0064)
#define LS7A_RTC_READ0_REG		(LS7A_RTC_REG_BASE + 0x0068)
#define LS7A_RTC_MATCH0_REG		(LS7A_RTC_REG_BASE + 0x006c)
#define LS7A_RTC_MATCH1_REG		(LS7A_RTC_REG_BASE + 0x0070)
#define LS7A_RTC_MATCH2_REG		(LS7A_RTC_REG_BASE + 0x0074)

#define LS7A_ACPI_REG_BASE		(LS7A_MISC_BASE + 0x50000)
#define LS7A_ACPI_PMCON_RTC_REG		(LS7A_ACPI_REG_BASE + 0x8)
#define LS7A_ACPI_PM1_STS_REG		(LS7A_ACPI_REG_BASE + 0xc)
#define LS7A_ACPI_PM1_CNT_REG		(LS7A_ACPI_REG_BASE + 0x14)
#define LS7A_ACPI_RST_CNT_REG		(LS7A_ACPI_REG_BASE + 0x30)
#define LS7A_ACPI_GEN_RTC1_REG		(LS7A_ACPI_REG_BASE + 0x50)
/*PWM*/
#define LS7A_PWM0_REG_BASE		(LS7A_MISC_BASE + 0x20000)
#define LS7A_PWM0_LOW			(LS7A_PWM0_REG_BASE + 0x4)
#define LS7A_PWM0_FULL			(LS7A_PWM0_REG_BASE + 0x8)
#define LS7A_PWM0_CTRL			(LS7A_PWM0_REG_BASE + 0xc)

#define LS7A_PWM1_REG_BASE		(LS7A_MISC_BASE + 0x20100)
#define LS7A_PWM1_LOW			(LS7A_PWM1_REG_BASE + 0x4)
#define LS7A_PWM1_FULL			(LS7A_PWM1_REG_BASE + 0x8)
#define LS7A_PWM1_CTRL			(LS7A_PWM1_REG_BASE + 0xc)

#define LS7A_PWM2_REG_BASE		(LS7A_MISC_BASE + 0x20200)
#define LS7A_PWM2_LOW			(LS7A_PWM2_REG_BASE + 0x4)
#define LS7A_PWM2_FULL			(LS7A_PWM2_REG_BASE + 0x8)
#define LS7A_PWM2_CTRL			(LS7A_PWM2_REG_BASE + 0xc)

#define LS7A_PWM3_REG_BASE		(LS7A_MISC_BASE + 0x20300)
#define LS7A_PWM3_LOW			(LS7A_PWM3_REG_BASE + 0x4)
#define LS7A_PWM3_FULL			(LS7A_PWM3_REG_BASE + 0x8)
#define LS7A_PWM3_CTRL			(LS7A_PWM3_REG_BASE + 0xc)

#define LS7A_I2C0_REG_BASE		(LS7A_MISC_BASE + 0x10000)
#define LS7A_I2C0_PRER_LO_REG		(LS7A_I2C0_REG_BASE + 0x0)
#define LS7A_I2C0_PRER_HI_REG		(LS7A_I2C0_REG_BASE + 0x1)
#define LS7A_I2C0_CTR_REG		(LS7A_I2C0_REG_BASE + 0x2)
#define LS7A_I2C0_TXR_REG		(LS7A_I2C0_REG_BASE + 0x3)
#define LS7A_I2C0_RXR_REG		(LS7A_I2C0_REG_BASE + 0x3)
#define LS7A_I2C0_CR_REG		(LS7A_I2C0_REG_BASE + 0x4)
#define LS7A_I2C0_SR_REG		(LS7A_I2C0_REG_BASE + 0x4)

#define LS7A_I2C1_REG_BASE		(LS7A_MISC_BASE + 0x10100)
#define LS7A_I2C1_PRER_LO_REG		(LS7A_I2C1_REG_BASE + 0x0)
#define LS7A_I2C1_PRER_HI_REG		(LS7A_I2C1_REG_BASE + 0x1)
#define LS7A_I2C1_CTR_REG		(LS7A_I2C1_REG_BASE + 0x2)
#define LS7A_I2C1_TXR_REG		(LS7A_I2C1_REG_BASE + 0x3)
#define LS7A_I2C1_RXR_REG		(LS7A_I2C1_REG_BASE + 0x3)
#define LS7A_I2C1_CR_REG		(LS7A_I2C1_REG_BASE + 0x4)
#define LS7A_I2C1_SR_REG		(LS7A_I2C1_REG_BASE + 0x4)

/* HPET */
#define LS7A_HPET_BASE			PHYS_TO_UNCACHED(0x10001000)
#define LS7A_HPET_PERIOD		LS7A_HPET_BASE + 0x4
#define LS7A_HPET_CONF			LS7A_HPET_BASE + 0x10
#define LS7A_HPET_MAIN			LS7A_HPET_BASE + 0xF0


#define CR_START			0x80
#define CR_STOP				0x40
#define CR_READ				0x20
#define CR_WRITE			0x10
#define CR_ACK				0x8
#define CR_IACK				0x1

#define SR_NOACK			0x80
#define SR_BUSY				0x40
#define SR_AL				0x20
#define SR_TIP				0x2
#define SR_IF

#define SLEEP_TYPE_S3		0x5

/*------------------------------------------------------------
| CID_NUM            | 2'h0    | no cid            |
|                    | 2'h1    | 2dies             |
|                    | 2'h2    | 4dies             |
|                    | 2'h3    | 8dies             |
| SDRAM_BG_SIZE      | 2'h0    | no bank group     |
|                    | 2'h1    | 2  bank group     |
|                    | 2'h2    | 4  bank group     |
| SDRAM_BA_SIZE      | 1'h0    | 4 bank            |
|                    | 1'h1    | 8 bank            |
| SDRAM_ROW_SIZE     | 3'hx    | 18-x              |
| SDRAM_COL_SIZE     | 2'hx    | 12-x              |
| ADDR_MIRROR        | 1'b1    | ADDR MIRROR       |
|                    | 1'b0    | STANDARD          |
| DIMM_MEMSIZE       |12'bx    | x*1G              |
| DIMM_WIDTH         | 2'h0    | 8bit//not support |
|                    | 2'h1    | 16bit             |
|                    | 2'h2    | 32bit             |
|                    | 2'h3    | 64bit             |
| DIMM_ECC           | 1'b1    | With ECC          |
|                    | 1'b0    | No ECC            |
| DIMM_TYPE          | 2'h0    | Unbuffered DIMM   |
|                    | 2'h1    | Registered DIMM   |
|                    | 2'h2    | SO-DIMM           |
|                    | 2'h3    | Load Reduced DIMM |
| SDRAM_WIDTH        | 2'h0    | x4                |
|                    | 2'h1    | x8                |
|                    | 2'h2    | x16               |
| SDRAM_TYPE         | 3'h0    | NO_DIMM           |
|                    | 3'hb    | DDR3              |
|                    | 3'hc    | DDR4              |
| MC_CSMAP           | 8'b0    | CS7-CS0           |
------------------------------------------------------------*/
#define	GMEM_MEMSIZE        4ULL
#define	GMEM_DRAM_TYPE      0xc
#define	GMEM_DIMM_TYPE      0
#define	GMEM_CID_NUM        0
#define	GMEM_BA_NUM         0
#define	GMEM_BG_NUM         1
#define	GMEM_CSMAP          1
#define	GMEM_DRAM_WIDTH     2
#define	GMEM_MODULE_WIDTH   2
#define	GMEM_ECC            0
#define	GMEM_SDRAM_CAPACITY 0
#define	GMEM_COL_NUM        2
#define	GMEM_ROW_NUM        1
#define	GMEM_ADDR_MIRROR    0

#endif /*_LS7A_H*/

