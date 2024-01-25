/*	$Id: fan.c,v 1.1.1.1 2006/09/14 01:59:08 xqch Exp $ */
/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/device.h>
#include <sys/queue.h>

#include <pmon.h>
#include "cpu.h"
#include <pmon/dev/ns16550.h>

#define pt32(x) (*(volatile unsigned int *)(x))
#define pt64(x) (*(volatile unsigned long long *)(x))
#define pt8(x) (*(volatile unsigned char *)(x))

unsigned char
cmd_usb2test(ac, av)
	int ac;
	char *av[];
{
	unsigned int tmp;
	unsigned long base;
	unsigned int cntl, port, test_mode;
	if (ac != 3) {
		printf("Usage:usb2test <port > <Test mode> \n");
		printf("<Test mode> 1: J_STATE\n");
		printf("<Test mode> 2: K_STATE\n");
		printf("<Test mode> 3: SE0_NAK\n");
		printf("<Test mode> 4: Packet\n");
		printf("<Test mode> 5: FORCE_ENABLE\n");
		printf("For example:usbtest 1 1 \n");
		return 0;
	}

	port = (unsigned int)atoi(av[1]);
	test_mode = (unsigned int)atoi(av[2]);

	if (port < 4) {
		cntl = 0;
	} else if (port < 8) {
		cntl = 1;
		port -= 4;
	} else {
		printf("Error: port number exceed valid value(5)\n");
		return 0;
	}
	if (test_mode < 1 || test_mode > 5) {
		printf("Error: test mode exceed valid value[1,5]\n");
		return 0;
	}
	printf("[debug]: USB cntl %d port %d\n", cntl, port);

	//get cntl base address
	base = *(volatile unsigned int *)(PHYS_TO_UNCACHED(0xefdfe000000) | (0 << 16) | ((4+cntl) << 11) | (1 << 8) | 0x10);
	base &= 0xfffffff0;
	base |= PHYS_TO_UNCACHED(0xe0000000000);

	base += 0x10; //HC operational register base
	printf("[debug]: USB operational register base is: 0x%llx\n", base);

	//reset USB to stop last test
	while ((pt32(base + 4) & 0x1000) != 0x1000);
	tmp = pt32(base);
	pt32(base) = tmp | 0x2;

	//make CF from 0 to 1
	pt32(base + 0x40) = 0;
	pt32(base + 0x40) = 1;
	//disable ASE and PSE
	tmp = pt32(base);
	pt32(base) = tmp & ~0x30;
	//suspend port
	tmp = pt32(base + 0x44 + port * 4);
	pt32(base + 0x44 + port * 4) = tmp | 0x84;
	//stop USB and wait HCHalted
	tmp = pt32(base);
	pt32(base) = tmp & ~0x1;
	while ((pt32(base + 4) & 0x1000) != 0x1000);
	//set test mode
	tmp = pt32(base + 0x44 + port * 4);
	pt32(base + 0x44 + port * 4) = (tmp & (~(0xf << 16))) | (test_mode << 16);

	if (test_mode == 5) {
		tmp = pt32(base);
		pt32(base) = tmp | 0x1;
	}
	printf("USB test ready and start...\n");

	return 1;
}

unsigned char
cmd_usb3test(ac, av)
	int ac;
	char *av[];
{
	unsigned int i, tmp, port, gen, testmode;
	unsigned long base;
	if ((ac < 2) || (ac > 4)) {
		printf("Usage:usb3test < port > < gen > < mode >\n");
		printf("For example:usb3test 0 2\n");
		printf("For example:usb3test 0 3\n");
		printf("For example:usb3test 0 3 1 (default CP0, CP0 to CP8 switch circularly, 1 switch one, 2 switch two, and so on)\n");
		printf("For example:usb3test 0 2 0 (1:Test J_STATE  2:Test K_STATE 3:Test SE0_NAK 4:Test Packet 5:Test FORCE_ENABLE)\n");
		printf("CP0:A pseudo-random data pattern\n");
		printf("CP1:Nyquist frequency\n");
		printf("CP2:Nyquist/2\n");
		printf("CP3:COM pattern\n");
		printf("CP4:The low frequency periodic signaling pattern\n");
		printf("CP5:With de-emphasis\n");
		printf("CP6:Without de-emphasis\n");
		printf("CP7:With de-emphasis. Repeating 50-250 1’s and then 50-250 0’s\n");
		printf("CP8:With without de-emphasis. Repeating 50-250 1’s and then 50-250 0’s\n");
		return 0;
	}

	port = (unsigned int)atoi(av[1]);
	gen =  (unsigned int)atoi(av[2]);

	if (port > 3) {
		printf("Error: port number exceed valid value\n");
		return 0;
	}

	printf("[debug]: USB port %d\n",port);

	//get cntl base address
	base = *(volatile unsigned int *)(PHYS_TO_UNCACHED(0xefdfe000000) | (0 << 16) | (25 << 11) | (0 << 8) | 0x10);
	base &= 0xfffffff0;
	base |= PHYS_TO_UNCACHED(0xe0000000000);

	printf("[debug]: USB operational register base is: 0x%llx\n", base);
	if(gen == 3) {
		if (ac == 3) {
			pt32(PHYS_TO_UNCACHED(0x10010000) + 0x800) = 0x0;
			pt32(PHYS_TO_UNCACHED(0x10010000) + 0x804) = (0xf000000 | 0x413);
			pt32(PHYS_TO_UNCACHED(0x10010000) + 0x804) = (0x7000000 | 0x413);

			delay(10000);
			pt32(base) |= (0x1 << 1);
			delay(10000);

			tmp = pt32(base + 0x420 + (port + 4) * 0x10);
			pt32(base + 0x420 + (port + 4) * 0x10) = (tmp & (~(0xf << 5))) | (0xa << 5) | (1 << 16);
		} else {
			testmode =  (unsigned int)atoi(av[3]);
			//change test mode, cp0-cp8
			for(i = 0; i < testmode; i++) {
				printf("change test mode %d times\n", i);
				pt32(base + 0xc2c0 + 0x4 * port) &= ~(0x1 << 30);
				pt32(base + 0xc2c0 + 0x4 * port) |= (0x1 << 30);
			}
		}
	} else if (gen == 2) {
		testmode =  (unsigned int)atoi(av[3]);
		pt32(base + 0x420 + port * 0x10) &= ~(0x1 << 9);
		pt32(base + 0x424 + port * 0x10) |= (testmode << 28);
	}
	printf("USB test ready and start...\n");

	return 1;
}

cmd_satatest(ac, av)
	int ac;
	char *av[];
{
	unsigned int port, gen;
	unsigned long base;
	unsigned int test_mode;
	unsigned int tmp;
	if (ac != 4) {
		printf("Usage:satatest <port > <gen> <test_mode>\n");
		printf("<gen> 3: SATA 3.0, The ahci.c file needs to be modified for switching rate \n");
		printf("<test_mode> 0: SSOP( Simultaneous switching outputs pattern)\n");
		printf("<test_mode> 1: HTDP( High transition density pattern)       \n");
		printf("<test_mode> 2: LTDP( Low transition density pattern)        \n");
		printf("<test_mode> 3: LFSCP( Low frequency spectral component pattern)\n");
		printf("<test_mode> 4: COMP( Composite pattern)      \n");
		printf("<test_mode> 5: LBP( Lone bit pattern)        \n");
		printf("<test_mode> 6: MFTP( Mid frequency test pattern)\n");
		printf("<test_mode> 7: HFTP( High frequency test pattern)\n");
		printf("<test_mode> 8: LFTP( Low frequency test pattern)\n");
		return 0;
	}

	//port actual mean cntl
	port = (unsigned int)atoi(av[1]);
	gen = (unsigned int)atoi(av[2]);
	test_mode =(unsigned int)atoi(av[3]);

	if (port > 3) {
		printf("Error: port exceed max value(3)\n");
		return 0;
	}
	if (test_mode > 8) {
		printf("Error: test mode exceed valid value[0,8]\n");
		return 0;
	}
	//get cntl base address
	base = *(volatile unsigned int *)(PHYS_TO_UNCACHED(0xefdfe000000) | (0 << 16) | (8 << 11) | (0 << 8) | 0x24);
	base &= 0xfffffff0;
	base |= PHYS_TO_UNCACHED(0xe0000000000);

	printf("[debug]: request test: port: %d, gen: %d, test mode: %d\n", port, gen, test_mode);
	printf("[debug]: SATA ctrl register base is: 0x%llx\n", base);

	//set port num
	pt32(base + 0xf4) = (port << 16);

        //set DET
	tmp = pt32(base + 0x100 + 0x80 * port + 0x2c);
        tmp &= ~0xf;
        tmp |= 0x1;
	pt32(base + 0x100 + 0x80 * port + 0x2c) = tmp;

	tmp = pt32(base + 0x100 + 0x80 * port + 0x2c);

	tmp = pt32(base + 0x100 + 0x80 * port + 0x2c);
        tmp &= ~0xff;
        tmp |= (gen << 4);
	pt32(base + 0x100 + 0x80 * port + 0x2c) = tmp;

	tmp = pt32(base + 0x100 + 0x80 * port + 0x2c);

	//tmp = pt32(base + 0xa4);
	//transmit only
	//pt32(base + 0xa4) = ((1 << 18) | (tmp & ~0xf & ~(0x7 << 8)) | test_mode);
	//pt32(base + 0xa4) = ((1 << 18) | (tmp & ~0xf & ~(0x7 << 8)) | (0x1 << 8) | test_mode);
	tmp = ((1 << 18) | (0x1 << 8) | test_mode);
	pt32(base + 0xa4) = tmp;

	return(1);
}

unsigned char
cmd_pcietest(ac, av)
	int ac;
	char *av[];
{
	unsigned int port, gen;
	unsigned int base,test_mode;
	unsigned int pcie_clock_source;
	unsigned int port_num;
	unsigned int dev_num;
	unsigned int lane_num;
	unsigned long long header;
	unsigned long long bar0;
	unsigned int bar0_low;
	unsigned int bar0_high;

	if ((ac < 2) || (ac > 5)) {
		printf("usage: pcietest <port num> <gen> [test mode]\n");
		printf("port num: 0 -> f0 x4\n");
		printf("port num: 1 -> f1 x4\n");
		printf("port num: 2 -> g0 x16\n");
		printf("port num: 3 -> h  x8\n");
		printf("gen 1/2 test_mode 0: -3.5db De-emphasis\n");
		printf("gen 2 test_mode 1: -6db De-emphasis\n");
		printf("gen 3 test_mode 0: preset 0\n");
		printf("gen 3 test_mode 1: preset 1\n");
		printf("gen 3 test_mode 2: preset 2\n");
		printf("gen 3 test_mode 3: preset 3\n");
		printf("gen 3 test_mode 4: preset 4\n");
		printf("gen 3 test_mode 5: preset 5\n");
		printf("gen 3 test_mode 6: preset 6\n");
		printf("gen 3 test_mode 7: preset 7\n");
		printf("gen 3 test_mode 8: preset 8\n");
		printf("gen 3 test_mode 9: preset 9\n");
		printf("gen 3 test_mode 0xa: preset 0xa\n");
		printf("For example1: pcietest 0 3 1\n");
		return 0;
	}

	port_num = (unsigned int)atoi(av[1]);
	printf("test  port num %d\n", port_num);

	dev_num = port_num == 0 ? 9  :
		port_num == 1 ? 13 :
		port_num == 2 ? 15 :
		port_num == 3 ? 19 :
				9;

	lane_num= port_num  == 0 ? 4  :
		port_num  == 1 ? 4 :
		port_num  == 2 ? 16 :
		port_num  == 3 ? 8 :
				4;

	gen = (unsigned int)atoi(av[2]);
	test_mode = (unsigned int)atoi(av[3]);
	printf("test  gen%d\n", gen);
	printf("test mode %d\n", test_mode);
	//header = PHYS_TO_UNCACHED(0xefe00000000) | (dev_num << 11);
	header = PHYS_TO_UNCACHED(0xefdfe000000) | (dev_num << 11);
	printf("header 0x%llx\n", header);

	bar0_low  = (readl(header + 0x10) & 0xffffffff0);
	bar0_high = (readl(header + 0x14));
	bar0 = (bar0_high << 32 | bar0_low) + PHYS_TO_UNCACHED(0xe0000000000);

	writel(0xff2044, bar0);



#define LINK_CTRL2       0xa0
#define LINK_SPEED       0
#define ENTER_COMP       4
#define MODE_COMP        10
#define COMP_SOS         11
#define PRESET_DEMPSIS   12
	if (gen == 0x1) {
		writel(0x0<<PRESET_DEMPSIS|0x1<<COMP_SOS|0x1<<MODE_COMP|0x1<<ENTER_COMP|0x1<<LINK_SPEED, header + LINK_CTRL2);
	} else if (gen == 0x2) {
		switch (test_mode) {
		case 0:
			writel(0x0<<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x2<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 1:
			writel(0x1<<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x2<<LINK_SPEED, header + LINK_CTRL2);
			break;
		default:
			break;
		}
	} else if (gen == 0x3) {
		printf("set gen3 test mode\n");
		switch (test_mode) {
		case 0:
			writel(0x0 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 1:
			writel(0x1 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 2:
			writel(0x2 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 3:
			writel(0x3 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 4:
			writel(0x4 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 5:
			writel(0x5 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 6:
			writel(0x6 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 7:
			writel(0x7 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 8:
			writel(0x8 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 9:
			writel(0x9 <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		case 10:
			writel(0xa <<PRESET_DEMPSIS|0x0<<COMP_SOS|0x0<<MODE_COMP|0x1<<ENTER_COMP|0x3<<LINK_SPEED, header + LINK_CTRL2);
			break;
		default:
			break;
		}
	}

	writel(0xff204c, bar0);

	return(1);
}

#define LS7A_UART0_BASE PHYS_TO_UNCACHED(0x10080000)
#define         REG_OFFSET              1

/* register offset */
#define         OFS_RCV_BUFFER          0
#define         OFS_TRANS_HOLD          0
#define         OFS_SEND_BUFFER         0
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_FIFO             (2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)

inline void  serial_write(int offsize, unsigned char date){
//delay(10000);
((*((volatile unsigned char*)(LS7A_UART0_BASE + offsize))) = date);
}

inline unsigned char serial_read(int offsize){
//delay(10000);
return (*((volatile unsigned char*)(LS7A_UART0_BASE + offsize)));
}

static unsigned char pr_state()
{

	return (serial_read(OFS_LINE_STATUS) & 0x1) ;
}

static int pr_out(unsigned char byte)
{
	while ((serial_read(OFS_LINE_STATUS) & 0x20) == 0);
	serial_write(OFS_SEND_BUFFER, byte);
	return 1;
}


static unsigned char pr_in()
{

	while ((serial_read(OFS_LINE_STATUS) & 0x1) == 0);
	return serial_read(OFS_RCV_BUFFER);
}

unsigned char cmd_uarttest(int ac, char *av)
{
	int i;
	char c;

	printf("serial test\n");
	/* disable interrupts */
	serial_write(OFS_FIFO,FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4);

	/* set up buad rate */
	{
		unsigned int divisor;

		/* set DIAB bit */
		serial_write(OFS_LINE_CONTROL, 0x80);

		/* set divisor */
		serial_write(OFS_DIVISOR_LSB, 0x1b);

		/* clear DIAB bit */
	//	serial_write(line,OFS_LINE_CONTROL, 0x0);
	}

	/* set data format */
	serial_write(OFS_DATA_FORMAT, 0x3);
	serial_write(OFS_MODEM_CONTROL,0);

	for(i = 0; i < 10; i++) {
		pr_out('a' + i);
	}
	pr_out(0x0d);
	pr_out(0x0a);

	printf("input the char date in 7a\n");
	for (i = 0; i < 10; i++) {
		while(!(c = pr_state()));
		printf("%c\n",(c=pr_in()));
		pr_out(c);
	}
}

/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Misc"},
	{"usb2test",  "", 0, "7A usbtest : usbtest  ", cmd_usb2test, 1, 99, 0},
	{"usb3test",  "", 0, "7A usbtest : usbtest  ", cmd_usb3test, 1, 99, 0},
	{"pcietest", "", 0, "7A pcietest: pcietest ", cmd_pcietest, 1, 99, 0},
	{"satatest", "", 0, "7A satatest: satatest ", cmd_satatest, 1, 99, 0},
	{"uarttest", "", 0, "7A uarttest: uarttest ", cmd_uarttest, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
