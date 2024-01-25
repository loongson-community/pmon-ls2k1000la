#ifndef _GMAC_H_
#define _GMAC_H_
#include <sys/types.h>

#define GMAC0_MAC_REG	((*(volatile unsigned int *)(PHYS_TO_UNCACHED(0xefdfe000000) | (0 << 16) | (3 << 11) | (0 << 8) | 0x10)))
#define GMAC1_MAC_REG	((*(volatile unsigned int *)(PHYS_TO_UNCACHED(0xefdfe000000) | (0 << 16) | (3 << 11) | (1 << 8) | 0x10)))
#define GMAC0_MAC_REG_ADDR	PHYS_TO_UNCACHED((GMAC0_MAC_REG & (~0xf)) | 0xe0000000000)
#define GMAC1_MAC_REG_ADDR	PHYS_TO_UNCACHED((GMAC1_MAC_REG & (~0xf)) | 0xe0000000000)
/*--------------------------------------------*/
#define GMAC0_DMA_REG_ADDR	(GMAC0_MAC_REG_ADDR + 0x1000)
#define GMAC1_DMA_REG_ADDR	(GMAC1_MAC_REG_ADDR + 0x1000)

/*--------------------------------------------*/
#define GMAC0_RX_DESC_BASE	0x9000000000060000
#define GMAC1_RX_DESC_BASE	0x9000000000060200
#define GMAC0_TX_DESC_BASE	0x9000000000040000
#define GMAC1_TX_DESC_BASE	0x9000000000040200

/*--------------------------------------------*/

#define ls_readl(x)	(*(volatile unsigned int*)(x))

/* Error Codes */
#define ESYNOPGMACNOERR		0
#define ESYNOPGMACNOMEM		1
#define ESYNOPGMACPHYERR	2
#define ESYNOPGMACBUSY		3

#define DEFAULT_DELAY_VARIABLE	10
#define DEFAULT_LOOP_VARIABLE	10000
#define SYNOP_PHY_LOOPBACK	1

enum GmacRegisters
{
	GmacConfig		= 0x0000,    /* Mac config Register                       */
	GmacFrameFilter		= 0x0004,    /* Mac frame filtering controls              */
	GmacHashHigh		= 0x0008,    /* Multi-cast hash table high                */
	GmacHashLow		= 0x000C,    /* Multi-cast hash table low                 */
	GmacGmiiAddr		= 0x0010,    /* GMII address Register(ext. Phy)           */
	GmacGmiiData		= 0x0014,    /* GMII data Register(ext. Phy)              */
	GmacFlowControl		= 0x0018,    /* Flow control Register                     */
	GmacVlan		= 0x001C,    /* VLAN tag Register (IEEE 802.1Q)           */

	GmacVersion		= 0x0020,    /* GMAC Core Version Register                */
	GmacWakeupAddr		= 0x0028,    /* GMAC wake-up frame filter adrress reg     */
	GmacPmtCtrlStatus	= 0x002C,    /* PMT control and status register           */

	GmacInterruptStatus	= 0x0038,    /* Mac Interrupt ststus register             */
	GmacInterruptMask	= 0x003C,    /* Mac Interrupt Mask register               */

	GmacAddr0High		= 0x0040,    /* Mac address0 high Register                */
	GmacAddr0Low		= 0x0044,    /* Mac address0 low Register                 */
	GmacAddr1High		= 0x0048,    /* Mac address1 high Register                */
	GmacAddr1Low		= 0x004C,    /* Mac address1 low Register                 */
	GmacAddr2High		= 0x0050,    /* Mac address2 high Register                */
	GmacAddr2Low		= 0x0054,    /* Mac address2 low Register                 */
	GmacAddr3High		= 0x0058,    /* Mac address3 high Register                */
	GmacAddr3Low		= 0x005C,    /* Mac address3 low Register                 */
	GmacAddr4High		= 0x0060,    /* Mac address4 high Register                */
	GmacAddr4Low		= 0x0064,    /* Mac address4 low Register                 */
	GmacAddr5High		= 0x0068,    /* Mac address5 high Register                */
	GmacAddr5Low		= 0x006C,    /* Mac address5 low Register                 */
	GmacAddr6High		= 0x0070,    /* Mac address6 high Register                */
	GmacAddr6Low		= 0x0074,    /* Mac address6 low Register                 */
	GmacAddr7High		= 0x0078,    /* Mac address7 high Register                */
	GmacAddr7Low		= 0x007C,    /* Mac address7 low Register                 */
	GmacAddr8High		= 0x0080,    /* Mac address8 high Register                */
	GmacAddr8Low		= 0x0084,    /* Mac address8 low Register                 */
	GmacAddr9High		= 0x0088,    /* Mac address9 high Register                */
	GmacAddr9Low		= 0x008C,    /* Mac address9 low Register                 */
	GmacAddr10High		= 0x0090,    /* Mac address10 high Register               */
	GmacAddr10Low		= 0x0094,    /* Mac address10 low Register                */
	GmacAddr11High		= 0x0098,    /* Mac address11 high Register               */
	GmacAddr11Low		= 0x009C,    /* Mac address11 low Register                */
	GmacAddr12High		= 0x00A0,    /* Mac address12 high Register               */
	GmacAddr12Low		= 0x00A4,    /* Mac address12 low Register                */
	GmacAddr13High		= 0x00A8,    /* Mac address13 high Register               */
	GmacAddr13Low		= 0x00AC,    /* Mac address13 low Register                */
	GmacAddr14High		= 0x00B0,    /* Mac address14 high Register               */
	GmacAddr14Low		= 0x00B4,    /* Mac address14 low Register                */
	GmacAddr15High		= 0x00B8,    /* Mac address15 high Register               */
	GmacAddr15Low		= 0x00BC,    /* Mac address15 low Register                */

	/*Time Stamp Register Map*/
	GmacTSControl		= 0x0700,  /* Controls the Timestamp update logic                         : only when IEEE 1588 time stamping is enabled in corekit            */
	GmacTSSubSecIncr	= 0x0704,  /* 8 bit value by which sub second register is incremented     : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSHigh		= 0x0708,  /* 32 bit seconds(MS)                                          : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSLow		= 0x070C,  /* 32 bit nano seconds(MS)                                     : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSHighUpdate	= 0x0710,  /* 32 bit seconds(MS) to be written/added/subtracted           : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSLowUpdate		= 0x0714,  /* 32 bit nano seconds(MS) to be writeen/added/subtracted      : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSAddend		= 0x0718,  /* Used by Software to readjust the clock frequency linearly   : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSTargetTimeHigh	= 0x071C,  /* 32 bit seconds(MS) to be compared with system time          : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSTargetTimeLow	= 0x0720,  /* 32 bit nano seconds(MS) to be compared with system time     : only when IEEE 1588 time stamping without external timestamp input */
	GmacTSHighWord		= 0x0724,  /* Time Stamp Higher Word Register (Version 2 only); only lower 16 bits are valid                                                   */
	GmacTSStatus		= 0x0728,  /* Time Stamp Status Register                                                                                                       */
	//GmacTSHighWordUpdate	= 0x072C,  /* Time Stamp Higher Word Update Register (Version 2 only); only lower 16 bits are valid                                            */
};

/* MAC register0 Mac Config */
#define PORT_SELECT		(1 << 15)	/* 0:GMII 1000M, 1:MII 10/100M */
#define SPEED_SELECT		(1 << 14)	/* 0:10M, 1:100M */
#define MAC_LOOPBACK		(1 << 12)
#define MAC_DUPLEX_FULL		(1 << 11)
#define MAC_IPC			(1 << 10)
#define MAC_LINK_UP		(1 << 8)
#define TRANS_ENABLE		(1 << 3)	/* tx state machine enable */
#define RECEIV_ENABLE		(1 << 2)	/* rx state machine enable */

#define MAC_MODE_100M		(PORT_SELECT | SPEED_SELECT)
#define MAC_MODE_10M		(PORT_SELECT)

/* MAC register1 Mac Frame Filter */
#define RECEIVE_ALL		(1 << 31)
#define PERFECT_FILTER		(1 << 10)
#define PROMISCUOUS_MODE	(1 << 0)
/**********************************************************
 * GMAC DMA registers
 * For Pci based system address is BARx + GmaDmaBase
 * For any other system translation is done accordingly
 **********************************************************/

enum DmaRegisters
{
	DmaBusMode        = 0x0000,    /* CSR0 - Bus Mode Register                          */
	DmaTxPollDemand   = 0x0004,    /* CSR1 - Transmit Poll Demand Register              */
	DmaRxPollDemand   = 0x0008,    /* CSR2 - Receive Poll Demand Register               */
	DmaRxBaseAddr     = 0x000C,    /* CSR3 - Receive Descriptor list base address       */
	DmaTxBaseAddr     = 0x0010,    /* CSR4 - Transmit Descriptor list base address      */
	DmaStatus         = 0x0014,    /* CSR5 - Dma status Register                        */
	DmaControl        = 0x0018,    /* CSR6 - Dma Operation Mode Register                */
	DmaInterrupt      = 0x001C,    /* CSR7 - Interrupt enable                           */
	DmaMissedFr       = 0x0020,    /* CSR8 - Missed Frame & Buffer overflow Counter     */
	DmaTxCurrDesc     = 0x0048,    /*      - Current host Tx Desc Register              */
	DmaRxCurrDesc     = 0x004C,    /*      - Current host Rx Desc Register              */
	DmaTxCurrAddr     = 0x0050,    /* CSR20 - Current host transmit buffer address      */
	DmaRxCurrAddr     = 0x0054,    /* CSR21 - Current host receive buffer address       */
};

/* DMA CSR0 DmaBusMode register bit define */
#define SOFT_RESET		(1 << 0)
#define PBL_SELECT(x)		(x << 8)

/* DMA CSR5 Dma Status register bit define */
#define RECEIVE_INTERRUPT	(1 << 6)

/* DMA CSR6 Dma Operation register bit define */
#define RTC_128			(3 << 3)	/* Receive Threshold Control */
#define DmaControl_SR		(1 << 1)
#define DmaControl_ST		(1 << 13)
#define DmaControl_TSF		(1 << 21)

enum GmacGmiiAddrReg
{
	GmiiDevMask     = 0x0000F800,     /* (PA)GMII device address                 15:11     RW         0x00    */
	GmiiDevShift    = 11,

	GmiiRegMask     = 0x000007C0,     /* (GR)GMII register in selected Phy       10:6      RW         0x00    */
	GmiiRegShift    = 6,

	GmiiCsrClkMask  = 0x0000001C,     /*CSR Clock bit Mask                  4:2                               */
	GmiiCsrClk5     = 0x00000014,     /* (CR)CSR Clock Range     250-300 MHz      4:2      RW         000     */
	GmiiCsrClk4     = 0x00000010,     /*                         150-250 MHz                                  */
	GmiiCsrClk3     = 0x0000000C,     /*                         35-60 MHz                                    */
	GmiiCsrClk2     = 0x00000008,     /*                         20-35 MHz                                    */
	GmiiCsrClk1     = 0x00000004,     /*                         100-150 MHz                                  */
	GmiiCsrClk0     = 0x00000000,     /*                         60-100 MHz                                   */

	GmiiWrite       = 0x00000002,     /* (GW)Write to register                      1      RW                 */
	GmiiRead        = 0x00000000,     /* Read from register                                            0      */

	GmiiBusy        = 0x00000001,     /* (GB)GMII interface is busy                 0      RW          0      */
};
#endif /*_GMAC_H_*/
