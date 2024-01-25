#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>
#include "target/ls7a.h"

#define PRER_LO_REG	0x0
#define PRER_HI_REG	0x1
#define CTR_REG    	0x2
#define TXR_REG    	0x3
#define RXR_REG    	0x3
#define CR_REG     	0x4
#define SR_REG     	0x4
#define SLV_CTRL_REG	0x7

#define writeb(reg, val)	outb(reg, val)
#define readb(reg)		inb(reg)
#define readl(addr) 		(*(volatile u32*)(addr))

unsigned long dc_i2c_base_addr;

static void ls7a_i2c_stop(void)
{
	do {
		writeb(dc_i2c_base_addr + CR_REG, CR_STOP);
		readb(dc_i2c_base_addr + SR_REG);
	} while (readb(dc_i2c_base_addr + SR_REG) & SR_BUSY);
}

void ls7a_i2c_init(ac, av)
    int ac;
    char *av[];
{
	int val,val1,val2;
	unsigned long long header;
	unsigned long long bar0;
	unsigned int bar0_low;
	unsigned int bar0_high;
	unsigned int status,desc_num;

	header = (PHYS_TO_UNCACHED(0xefdfe000000) | (6 << 11) | (1 << 8));

	bar0_low  = (readl(header + 0x10) & 0xffffffff0);
	bar0_high = readl(header + 0x14);
	bar0 = PHYS_TO_UNCACHED(bar0_high << 32 | bar0_low);

	printf("pcie header = 0x%llx\n",header);
	printf("pcie bar0_low = 0x%x\n",bar0_low);
	printf("pcie bar0_high = 0x%x\n",bar0_high);
	printf("pcie bar0 = 0x%llx\n",bar0);

	val = (unsigned char)strtoul(av[1], 0, 0);  //lo
	val1 = (unsigned char)strtoul(av[2], 0, 0); //hi
	val2 = (unsigned char)strtoul(av[3], 0, 0); //i2c 0 or 1

	if(val2) {
		dc_i2c_base_addr = bar0 + 0x1f10;
		readl(bar0 + 0x1730) |= (1 << 8);
	} else {
		dc_i2c_base_addr = bar0 + 0x1f00;
		readl(bar0 + 0x1720) |= (1 << 8);
	}

	printf("base 0x%lx \n", dc_i2c_base_addr);
	printf("val 0x%x,val1 0x%x \n", val, val1);

	readb(dc_i2c_base_addr + CTR_REG) &=  ~0x80;
	writeb(dc_i2c_base_addr + PRER_LO_REG, val);
	writeb(dc_i2c_base_addr + PRER_HI_REG, val1);
//	writeb(dc_i2c_base_addr + PRER_LO_REG, 0x52); //dc pll 300M
//	writeb(dc_i2c_base_addr + PRER_HI_REG, 0x2);
	readb(dc_i2c_base_addr + CTR_REG) |=  0x80;
}

static int ls7a_i2c_tx_byte(unsigned char data, unsigned char opt)
{
	int times = 1000000;
	writeb(dc_i2c_base_addr + TXR_REG, data);
	writeb(dc_i2c_base_addr + CR_REG, opt);
	while ((readb(dc_i2c_base_addr + SR_REG) & SR_TIP) && times--);
	if (times < 0) {
		printf("ls7a_i2c_tx_byte SR_TIP can not ready!\n");
		ls7a_i2c_stop();
		return -1;
	}

	if (readb(dc_i2c_base_addr + SR_REG) & SR_NOACK) {
		printf("device has no ack, Pls check the hardware!\n");
		ls7a_i2c_stop();
		return -1;
	}

	return 0;
}

static int ls7a_i2c_send_addr(unsigned char dev_addr,unsigned int data_addr)
{
	if (ls7a_i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0){
		printf("line %d tx dev byte error!\n",__LINE__);
		return 0;
	}

	if (ls7a_i2c_tx_byte(data_addr & 0xff, CR_WRITE) < 0){
		printf("line %d tx reg byte error!\n",__LINE__);
		return 0;
	}

	return 1;
}


 /*
 * the function write sequence data.
 * dev_addr : device id
 * data_addr : offset
 * buf : the write data buffer
 * count : size will be write
  */
int ls7a_i2c_write_seq(unsigned char dev_addr,unsigned int data_addr, unsigned char *buf, int count)
{
	int i;
	if (!ls7a_i2c_send_addr(dev_addr,data_addr))
		return 0;
	for (i = 0; i < count; i++)
		if (ls7a_i2c_tx_byte(buf[i] & 0xff, CR_WRITE) < 0)
			return 0;

	ls7a_i2c_stop();

	return i;
}

 /*
 * the function write one byte.
 * dev_addr : device id
 * data_addr : offset
 * buf : the write data
  */
int ls7a_i2c_write_byte(unsigned char dev_addr,unsigned int data_addr, unsigned char *buf)
{
	return ls7a_i2c_write_seq(dev_addr, data_addr, *buf, 1);
}
 /*
  * Sequential reads by a current address read.
 * dev_addr : device id
 * data_addr : offset
 * buf : the write data buffer
 * count : size will be write
  */
static int ls7a_i2c_read_seq_cur(unsigned char dev_addr,unsigned char *buf, int count)
{
	int i;
	dev_addr |= 0x1;

	if (ls7a_i2c_tx_byte(dev_addr, CR_START | CR_WRITE) < 0)
		return 0;

	for (i = 0; i < count; i++) {
		writeb(dc_i2c_base_addr + CR_REG, ((i == count - 1) ?
					(CR_READ | CR_ACK) : CR_READ));
		while (readb(dc_i2c_base_addr + SR_REG) & SR_TIP) ;
		buf[i] = readb(dc_i2c_base_addr + RXR_REG);
	}

	ls7a_i2c_stop();
	return i;
}

int ls7a_i2c_read_seq_rand(unsigned char dev_addr,unsigned int data_addr,
				unsigned char *buf, int count)
{
	if (!ls7a_i2c_send_addr(dev_addr,data_addr)){
		printf("send addr error!\n");
		return 0;
	}

	return ls7a_i2c_read_seq_cur(dev_addr,buf, count);
}

int read_edid(ac, av)
    int ac;
    char *av[];
{
	int i, num;
	uint8_t dev_addr;
	uint8_t buf[1] = {0};
	dev_addr = (unsigned char)strtoul(av[1], 0, 0);
	num = (unsigned char)strtoul(av[2], 0, 0);
	printf("dev_addr 0x%x,num 0x%x\n", dev_addr,num); //addr 0xa0
	//ls7a_i2c_init();
	for(i=0; i < num; i++){
		ls7a_i2c_read_seq_rand(dev_addr, i, buf, 1);
		printf("reg 0x%x value 0x%x\n", i, buf[0]);
	}
}

int write_eeprom(ac, av)
    int ac;
    char *av[];
{
	int i, num;
	uint8_t dev_addr;
	uint8_t tmp;
	dev_addr = (unsigned char)strtoul(av[1], 0, 0);
	tmp = (unsigned char)strtoul(av[2], 0, 0);
	printf("dev_addr 0x%x,num 0x%x\n", dev_addr,num);
	//ls7a_i2c_init();
	ls7a_i2c_write_seq(dev_addr, i, &tmp, 1);
}

static const Cmd Cmds[] = {
    {"Misc"},
    {"read_edid", "i2c addr regnum", 0, "read_edid", read_edid, 1, 5, 0},
    {"write_eeprom", "i2c addr regnum", 0, "write_eeprom", write_eeprom, 1, 5, 0},
    {"dc_i2c_init", "dc i2c init", 0, "i2c init", ls7a_i2c_init, 1, 5, 0},
    {0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
    cmdlist_expand(Cmds, 1);
}
