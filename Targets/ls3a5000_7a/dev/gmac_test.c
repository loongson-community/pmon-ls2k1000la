/**********************************************
******* GMAC1 LOOPBACK TEST for 7A2000 ********
**********************************************/

#include "gmac.h"

#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <pmon.h>


#define GMAC_DEBUG	0
#if GMAC_DEBUG
#define dbg printf
#else
#define dbg(fmt, arg...)
#endif

#define PHYS_TO_DMA(x)		(x)
#define VIRT_TO_PHYS(x)		((x) & 0x1fffffff)
#define VIRT_TO_DMA(x)		PHYS_TO_DMA(VIRT_TO_PHYS(x))

#define PHY_REG0		0
#define PHY_REG16		16
#define PHY_REG17		17
#define PHY_REG18		18
#define PHY_REG20		20
#define PHY_REG21		21
#define PHY_REG_PAGE		22

/* PHY REG0 define */
#define PHY_RESET		(1 << 15)
#define PHY_LOOPBACK		(1 << 14)
#define PHY_SPEED0		(1 << 13)
#define PHY_AUTO_NEGOTIATION	(1 << 12)
#define PHY_DUMPLEX_FULL	(1 << 8)
#define PHY_SPEED1		(1 << 6)

/* PHY REG 0xd8 define */
#define PHY_LINK		(0x8)

/* Normal desc define */
#define NEXT_ONE_DESC		0x20
#define NORMAL_DESC_SIZE	0x20

#define TDES0			0x0
#define TDES1			0x4
#define TDES2			0x8
#define TDES3			0xc
#define TDES4			0x10

#define RDES0			0x0
#define RDES1			0x4
#define RDES2			0x8
#define RDES3			0xc
#define RDES4			0x10

/* TDES0 bit define */
#define OWNER			(1 << 31)	/* the same to RDES0 */

#define LAST_SEGMENT		(1 << 29)
#define FIRST_SEGMENT		(1 << 28)
#define TRANS_END_RING		(1 << 21)	/* bit set 1: final desc of ring */
/* TDES1 bit define */
#define TCH_ADDR		(1 << 20)	/* bit set 1: next desc address, bit set 0: second buf address */

/* RDES1 bit define */
#define RECEIV_END_RING		(1 << 15)
#define RCH_ADDR		(1 << 14)	/* bit set 1: next desc address, bit set 0: second buf address */

#define SOFT_RESET		0x1
#define RECEIV_ENABLE		0x4
#define TRANS_ENABLE		0x8

unsigned int loopmode = 3;
unsigned int speed = 1000;
unsigned int timing = 0;

unsigned int trans_buf_size = 0xfc;

char my_gmac_tx_data[0x10000];
char my_gmac_rx_data[0x10000];

static unsigned int gmac_read(unsigned long base, unsigned int reg)
{
	unsigned long addr;
	unsigned int data;

	addr = base + reg;
	data = ls_readl(addr);
	return data;
}

static void gmac_write(unsigned long base, unsigned int reg, unsigned int data)
{
	unsigned long addr;

	addr = base + reg;
	ls_readl(addr) = data;
	return;
}

static signed int gmac_phy_read(unsigned long base, unsigned int PhyBase, unsigned int reg, unsigned short *data)
{
	unsigned int addr;
	unsigned int loop_variable;
	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((reg << GmiiRegShift) & GmiiRegMask) | GmiiCsrClk3;
	addr = addr | GmiiBusy;

	gmac_write(base, GmacGmiiAddr, addr);

	for (loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++) {
		if (!(gmac_read(base, GmacGmiiAddr) & GmiiBusy)) {
			break;
		}
		int i = DEFAULT_DELAY_VARIABLE;
		while (i--);
	}
	if (loop_variable < DEFAULT_LOOP_VARIABLE)
		*data = (unsigned short)(gmac_read(base,GmacGmiiData) & 0xFFFF);
	else {
		printf("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
	}

	return -ESYNOPGMACNOERR;
}

int phyread(int argc, char **argv)
{
	if (argc != 3) {
		printf("usage: phyread gmac reg\n");
		return 0;
	}
	unsigned int reg = strtoul(argv[2], 0, 0);
	unsigned long base = (strtoul(argv[1], 0, 0) == 0) ? GMAC0_MAC_REG_ADDR : GMAC1_MAC_REG_ADDR;
	unsigned int PhyBase = (strtoul(argv[1], 0, 0) == 0) ? 2 : 0;
	unsigned short data;
	unsigned int addr;
	unsigned int loop_variable;
	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((reg << GmiiRegShift) & GmiiRegMask) | GmiiCsrClk3;
	addr = addr | GmiiBusy ;
	gmac_write(base, GmacGmiiAddr, addr);

	for (loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++) {
		if (!(gmac_read(base, GmacGmiiAddr) & GmiiBusy)) {
			break;
		}
		int i = DEFAULT_DELAY_VARIABLE;
		while (i--);
	}
	if (loop_variable < DEFAULT_LOOP_VARIABLE) {
		data = (unsigned short)(gmac_read(base, GmacGmiiData) & 0xFFFF);
		printf("%x\n", data);
	} else {
		printf("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
	}
	return -ESYNOPGMACNOERR;
}

static signed int gmac_phy_write(unsigned long base, unsigned int PhyBase, unsigned int reg, unsigned short data)
{
	unsigned int addr;
	unsigned int loop_variable;
	gmac_write(base, GmacGmiiData, data);

	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((reg << GmiiRegShift) & GmiiRegMask) | GmiiWrite | GmiiCsrClk3;

	addr = addr | GmiiBusy ;
	gmac_write(base, GmacGmiiAddr, addr);
	for (loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++) {
		if (!(gmac_read(base, GmacGmiiAddr) & GmiiBusy)){
			break;
		}
		int i = DEFAULT_DELAY_VARIABLE;
		while (i--);
	}

	if (loop_variable < DEFAULT_LOOP_VARIABLE) {
		return -ESYNOPGMACNOERR;
	} else {
		printf("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
	}
}

int phywrite(int argc, char **argv)
{
	if (argc != 4) {
		printf("usage: phywrite gmac reg data");
		return 0;
	}
	unsigned long base = (strtoul(argv[1], 0, 0) == 0) ? GMAC0_MAC_REG_ADDR : GMAC1_MAC_REG_ADDR;
	unsigned int reg = strtoul(argv[2], 0, 0);
	unsigned short data = strtoul(argv[3], 0, 0);
	unsigned int PhyBase = (strtoul(argv[1], 0, 0) == 0) ? 2 : 0;
	unsigned int addr;
	unsigned int loop_variable;
	gmac_write(base, GmacGmiiData, data);

	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((reg << GmiiRegShift) & GmiiRegMask) | GmiiWrite | GmiiCsrClk3;

	addr = addr | GmiiBusy ;
	gmac_write(base, GmacGmiiAddr, addr);
	for (loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++) {
		if (!(gmac_read(base, GmacGmiiAddr) & GmiiBusy)) {
			break;
		}
		int i = DEFAULT_DELAY_VARIABLE;
		while (i--);
	}

	if (loop_variable < DEFAULT_LOOP_VARIABLE) {
		return -ESYNOPGMACNOERR;
	} else {
		printf("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
	}
}

static int p88e1512_config_loopback(unsigned long base, unsigned int PhyBase)
{
	unsigned short data;
	if (loopmode == 2) {
		gmac_phy_write(base, PhyBase, PHY_REG_PAGE, 2);	//page 2
		gmac_phy_read(base, PhyBase, PHY_REG21, &data);
		if (speed == 10) {
			data &= (~PHY_SPEED0);
			data &= (~PHY_SPEED1);
		} else if (speed == 100) {
			data |= PHY_SPEED0;
			data &= (~PHY_SPEED1);
		} else if (speed == 1000) {
			data |= PHY_SPEED1;
			data &= (~PHY_SPEED0);
		}
		gmac_phy_write(base, PhyBase, PHY_REG21, data);

		//reset
		gmac_phy_write(base, PhyBase, PHY_REG_PAGE, 0x0);	//page 0
		gmac_phy_read(base, PhyBase, PHY_REG0, &data);
		data |= PHY_RESET;
		gmac_phy_write(base, PhyBase, PHY_REG0, data);
		delay(1000);

		gmac_phy_read(base, PhyBase, PHY_REG0, &data);
		data |= PHY_LOOPBACK;
		gmac_phy_write(base, PhyBase, PHY_REG0, data);
		dbg("\r=== phy reg 0x0: %x\n",data);

	} else if (loopmode == 3) {
		if (speed == 10)
			gmac_phy_write(base, PhyBase, PHY_REG0, PHY_RESET | PHY_DUMPLEX_FULL);
		else if (speed == 100)
			gmac_phy_write(base, PhyBase, PHY_REG0, PHY_RESET | PHY_DUMPLEX_FULL | PHY_SPEED0);
		else if (speed == 1000){
			gmac_phy_write(base, PhyBase, PHY_REG_PAGE, 6);
			gmac_phy_read(base, PhyBase, PHY_REG18, &data);
			data |= (1 << 3);
			gmac_phy_write(base, PhyBase, PHY_REG18, data);
			gmac_phy_write(base, PhyBase, PHY_REG_PAGE, 0);

			gmac_phy_write(base, PhyBase, PHY_REG0, PHY_RESET | PHY_DUMPLEX_FULL | PHY_SPEED1);
		}
		//Wait the phy link state is ok.
		do {
			gmac_phy_read(base, PhyBase, PHY_REG17, &data);
		} while (!(data & PHY_LINK));
		dbg("PHY link up.\n");
	}

	gmac_phy_write(base, PhyBase, PHY_REG_PAGE, 0x0);

	return 1;
}

static int p88e1512_dis_loopback(unsigned long MacBase, unsigned int PhyBase)
{
	int err;
	unsigned short data;

	gmac_phy_write(MacBase, PhyBase, PHY_REG_PAGE, 0x0);	//page 0
	gmac_phy_read(MacBase, PhyBase, PHY_REG0, &data);
	dbg("\rloopback mode PHY reg0  %x\n", data);
	gmac_phy_write(MacBase, PhyBase, PHY_REG0, data & (~PHY_LOOPBACK));

	err = gmac_phy_read(MacBase, PhyBase, PHY_REG0, &data);
	dbg("\rdisloopback mode PHY reg0  %x\n", data);
	return err;
}

static int gmac_memcpy(const char *addr1, char *addr2, int len)
{
	while(len) {
		*addr2++ = *addr1++;
		len--;
	}
	return len;
}

static int gmac_check(const char *s1, const char *s2,int len)
{
	while(len) {
		if (*s1++ != *s2++)
			break;
		len--;
	}
	if (len)
		dbg("\r-------------------- check over len is end %d\n",len);
	return len;
}

static void gmac_loopback_test(unsigned long mac_base, unsigned long dma_base, unsigned long tx_base, unsigned long rx_base)
{
	printf("LOOPMODE: %d, SPEED: %dM, buffer size: 0x%x\n", loopmode, speed, trans_buf_size);
	printf("mac_base = 0x%llx; dma_base = 0x%llx\ntx_base = 0x%llx; rx_base = 0x%llx\n", mac_base, dma_base, tx_base, rx_base);
	static char gmac_tx_data[0xc00];
	int i;
	for (i = 0; i < 12; i++)
		memset(gmac_tx_data + 0x100 * i, 0x11 * (i + 1), 0x100);

	unsigned int tmp, tmp2 = 0;
	unsigned short data;
	unsigned int PhyBase = (rx_base == GMAC0_RX_DESC_BASE) ? 2 : 0;

	tmp = gmac_read(dma_base, DmaBusMode);
	tmp |= SOFT_RESET;
	gmac_write(dma_base, DmaBusMode, tmp);	//soft reset

	/* wait reset, hardware clear soft_reset bit */
	while(gmac_read(dma_base, DmaBusMode) & SOFT_RESET) {
		dbg("\r %x\n",gmac_read(dma_base, DmaBusMode));
		delay(10);
	}
	gmac_write(dma_base, DmaBusMode, PBL_SELECT(4) | 4);

	gmac_write(mac_base, GmacFrameFilter, RECEIVE_ALL | PERFECT_FILTER | PROMISCUOUS_MODE);

	if (speed == 10)
		tmp = MAC_MODE_10M;
	else if (speed == 100)
		tmp = MAC_MODE_100M;
	else if (speed == 1000)
		tmp = 0;
	gmac_write(mac_base, GmacConfig, gmac_read(mac_base, GmacConfig) & ~MAC_MODE_100M | MAC_DUPLEX_FULL | MAC_LINK_UP | tmp);

	if (loopmode == 1)
		gmac_write(mac_base, GmacConfig, gmac_read(mac_base, GmacConfig) | MAC_LOOPBACK);
	else
		p88e1512_config_loopback(mac_base, PhyBase);


	unsigned long tx_addr = PHYS_TO_CACHED((unsigned int)my_gmac_tx_data & 0xfffff00);
	unsigned long rx_addr = PHYS_TO_CACHED((unsigned int)my_gmac_rx_data & 0xfffff00);
	memset(rx_addr, 0, sizeof(my_gmac_rx_data));
	dbg("size: gmac_tx_data[0x%x], tx_buf[0x%x]\n", sizeof(gmac_tx_data), sizeof(my_gmac_tx_data));
	printf("tx_addr = 0x%llx; rx_addr = 0x%llx\n", tx_addr, rx_addr);

	tmp = gmac_memcpy((char *)gmac_tx_data, (char *)tx_addr, sizeof(gmac_tx_data));

	unsigned int receiv_buf_size =	trans_buf_size + 0x4;	//considering checksum

	//fill the RDS descriptor.
	for (i = 0; i < 12; i++) {
		if (i == 11) {
			ls_readl(rx_base + (i * NORMAL_DESC_SIZE) + RDES0) = OWNER;
			ls_readl(rx_base + (i * NORMAL_DESC_SIZE) + RDES1) = RECEIV_END_RING | receiv_buf_size;

			tmp2 = (((rx_addr + (receiv_buf_size * i )) & 0x1fffffff));
			ls_readl(rx_base + (i * NORMAL_DESC_SIZE) + RDES2) = PHYS_TO_DMA(tmp2);
		} else {
			ls_readl(rx_base + (i * NORMAL_DESC_SIZE) + RDES0) = OWNER;
			ls_readl(rx_base + (i * NORMAL_DESC_SIZE) + RDES1) = RCH_ADDR | receiv_buf_size;

			tmp2 = (((rx_addr + (receiv_buf_size * i )) & 0x1fffffff));
			ls_readl(rx_base + (i * NORMAL_DESC_SIZE) + RDES2) = PHYS_TO_DMA(tmp2);

			tmp = rx_base;
			ls_readl(rx_base + (i * NORMAL_DESC_SIZE) + RDES3) =
					PHYS_TO_DMA((tmp & 0x1fffffff) + (i * NORMAL_DESC_SIZE) + NEXT_ONE_DESC);
		}
	}

	gmac_write(dma_base, DmaRxBaseAddr, VIRT_TO_DMA(rx_base));
	gmac_write(mac_base, GmacConfig, gmac_read(mac_base, GmacConfig) | RECEIV_ENABLE);

	//fill the TDS descriptor.
	for (i = 0; i < 12; i++) {
		if (i == 11) {
			ls_readl(tx_base + (i * NORMAL_DESC_SIZE) + TDES0) = LAST_SEGMENT | FIRST_SEGMENT | TRANS_END_RING | OWNER;
			ls_readl(tx_base + (i * NORMAL_DESC_SIZE) + TDES1) = trans_buf_size;

			/* TDES2 is phy addr buf1 */
			tmp2 = tx_addr + (receiv_buf_size * i);
			ls_readl(tx_base + (i * NORMAL_DESC_SIZE) + TDES2) = VIRT_TO_DMA(tmp2);
		} else {
			ls_readl(tx_base + (i * NORMAL_DESC_SIZE) + TDES0) = LAST_SEGMENT | FIRST_SEGMENT | OWNER;
			ls_readl(tx_base + (i * NORMAL_DESC_SIZE) + TDES1) = trans_buf_size;

			tmp2 = tx_addr + (receiv_buf_size * i);
			/* TDES2 is phy addr buf1 */
			ls_readl(tx_base + (i * NORMAL_DESC_SIZE) + TDES2) = VIRT_TO_DMA(tmp2);

			/* TDES3 is phy addr buf2, or next desc addr */
			tmp = VIRT_TO_PHYS(tx_base);
			ls_readl(tx_base + (i * NORMAL_DESC_SIZE) + TDES3) =
						PHYS_TO_DMA(tmp + (i * NORMAL_DESC_SIZE) + NEXT_ONE_DESC);
		}
	}
	gmac_write(dma_base, DmaTxBaseAddr, VIRT_TO_DMA(tx_base));
	gmac_write(mac_base, GmacConfig, gmac_read(mac_base, GmacConfig) | TRANS_ENABLE);
	gmac_write(dma_base, DmaStatus, 0xffff);
	dbg("\rDmaStatus reg = %x\n",gmac_read(dma_base, DmaStatus));

	gmac_write(dma_base, DmaControl, gmac_read(dma_base, DmaControl) | DmaControl_SR | DmaControl_ST | DmaControl_TSF | RTC_128);

	if (timing) {
		struct timeval tv1, tv2;
		gettimeofday(&tv1, NULL);
		while (!(gmac_read(dma_base, DmaStatus) & RECEIVE_INTERRUPT)) ;
		gettimeofday(&tv2, NULL);
		unsigned long long t1 = tv1.tv_sec * 1000000 + tv1.tv_usec;
		unsigned long long t2 = tv2.tv_sec * 1000000 + tv2.tv_usec;
		printf("transfer: %ld\n", t1);
		printf("receive : %ld\n", t2);
		printf("cost: %ld\n", t2 - t1);
	}
	delay(10000);
	for (i = 0; i < 12; i++) {
		tmp2 = gmac_check((char *)tx_addr + i * receiv_buf_size, (char *)rx_addr + i * receiv_buf_size, trans_buf_size);
		if (tmp2) {
			printf("--- GMAC%d test ERROR at FRAME %d!\n",rx_base == GMAC1_RX_DESC_BASE ? 1 : 0, i);
			break;
		}
	}
	if (!tmp2)
		printf("--- GMAC%d test PASS!\n",rx_base == GMAC1_RX_DESC_BASE ? 1 : 0);

	if (loopmode == 1)
		gmac_write(mac_base, GmacConfig, gmac_read(mac_base, GmacConfig) & ~MAC_LOOPBACK);
	else if (loopmode == 2)
		p88e1512_dis_loopback(mac_base, PhyBase);
}

const Optdesc cmd_gmacloop_opts[] =
{
	{"-m", "mode<1:gmac / 2:phy / 3:external>"},
	{"-s", "speed<10/100/1000>"},
	{"-t", "timing on/off"},
	{"-l", "transmit buffer size"},
	{0}
};

int gmac_test(int argc, char **argv)
{
	int c, err = 0, tmp;
	extern int optind;
	extern char *optarg;
	optind = 0;
	while ((c = getopt(argc, argv, "m:s:tl:")) != EOF) {
		switch (c) {
		case 'm':
			if (!get_rsa(&tmp, optarg))
				err++;
			if (tmp < 1 || tmp > 3) {
				printf("invalid mode\n");
				return 0;
			}
			loopmode = tmp;
			break;
		case 's':
			if (!get_rsa(&tmp, optarg))
				err++;
			if (tmp < 1 || tmp > 3) {
				printf("invalid speed\n");
				return 0;
			}
			speed = 1;
			while (tmp--) speed *= 10;
			break;
		case 't':
			timing = ~timing;
			break;
		case 'l':
			if (!get_rsa(&trans_buf_size, optarg))
				err++;
			printf("Warning: change buffer size may cause test 'ERROR' when there's actually no error.\n");
			printf("\tLeave it alone or take a look at rx_addr to make sure.\n");
			break;
		default:
			err++;
			break;
		}
	}

	if (err) {
		return EXIT_FAILURE;
	}

	printf("GMAC1 TEST BEGIN\n");
	unsigned long mac_base = GMAC1_MAC_REG_ADDR;
	unsigned long dma_base = GMAC1_DMA_REG_ADDR;
	unsigned long tx_base = GMAC1_TX_DESC_BASE;
	unsigned long rx_base = GMAC1_RX_DESC_BASE;

	gmac_loopback_test(mac_base, dma_base, tx_base, rx_base);

	printf("GMAC TEST END\n");
	return 0;
}


static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"gmacloop", "[-mstl]", cmd_gmacloop_opts, "gmac1 loopback test", gmac_test, 0, 99, 0},
	{"phyread", "gmac reg", 0, "read phy reg", phyread, 0, 99, CMD_REPEAT},
	{"phywrite", "gmac reg data", 0, "write phy reg", phywrite, 0, 99, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

