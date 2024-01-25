#include <stdio.h>
//#include "include/fcr.h"
#include <stdlib.h>
#include <ctype.h>
#undef _KERNEL
#include <errno.h>
#include <pmon.h>
//#include <types.h>
#include <pflash.h>
#include <target/ls2k1000.h>
#include <linux/spi.h>
#include <linux/bitops.h>
//#include "spinand_mt29f.h"
#include "spinand_lld.h"
#include "m25p80.h"

char* ls_spi_base = LS2K1000_SPI0_BASE;		//ls2k1000 spi2~5 is apb mode

#define SPCR      0x0
#define SPSR      0x1
#define FIFO	  0x2
#define TXFIFO    0x2
#define RXFIFO    0x2
#define SPER      0x3
#define PARAM     0x4
#define SOFTCS    0x5
#define PARAM2    0x6
#define APB_SPCS	0x4

#if SPI_CLOCK == 25 
#define SPI_DIV 0x01
#elif SPI_CLOCK == 50
#define SPI_DIV 0x00
#endif



#define RFEMPTY 1

#define SET_SPI(addr,val)	writeb(val, ls_spi_base + addr)
#define GET_SPI(addr)		readb(ls_spi_base + addr)

#define SET_CE(x)									\
do {											\
	if (ls_spi_base == LS2K1000_SPI0_BASE || ls_spi_base == LS2K1000_SPI1_BASE) {	\
		SET_SPI(SOFTCS, x);							\
	} else {									\
		SET_SPI(APB_SPCS, (x >> 4) | (!(x & 1) << 1));				\
	}										\
} while(0)

void spi_initw(void)
{
	SET_SPI(SPSR, 0xc0);
	if (ls_spi_base == LS2K1000_SPI0_BASE || ls_spi_base == LS2K1000_SPI1_BASE) {
		SET_SPI(PARAM, 0x10);//espr:0100
		SET_SPI(PARAM2,0x01);
	}
	SET_SPI(SPER, 0x04);//spre:01
	SET_SPI(SPCR, 0x51);
}

void spi_initr(void)
{
	if (ls_spi_base == LS2K1000_SPI0_BASE || ls_spi_base == LS2K1000_SPI1_BASE) {
		SET_SPI(PARAM, 0x17);	     //espr:0100
	}
}

int send_spi_cmd(unsigned char command)
{
	int timeout = 1000;
	unsigned char val;

	SET_SPI(TXFIFO,command);
	while(((GET_SPI(SPSR)) & RFEMPTY) && timeout--);
	val = GET_SPI(RXFIFO);

	if (timeout < 0) {
		printf("wait rfempty timeout!\n");
		return -1;
	}
	return val;
}

///////////////////read status reg /////////////////
int read_sr(void)
{
	int val;

	SET_CE(0x01);
	send_spi_cmd(0x05);
	val = send_spi_cmd(0x00);
	SET_CE(0x11);

	return val;
}

int spi_wait_busy(void)
{
	int timeout = 1000;
	unsigned char res;

	do {
		res = read_sr();
	} while ((res & 0x01) && timeout--);

	if (timeout < 0) {
		printf("wait status register busy bit time out!\n");
		return -1;
	}
	return 0;
}

////////////set write enable//////////
int set_wren(void)
{
	if (spi_wait_busy() < 0) {
		return -1;
	}

	SET_CE(0x01);
	send_spi_cmd(0x6);
	SET_CE(0x11);

	return 1;
}

////////////Enable-Write-Status-Register//////////
int en_write_sr(void)
{
	if (spi_wait_busy() < 0)
		return -1;

	SET_CE(0x01);
	send_spi_cmd(0x50);
	SET_CE(0x11);

	return 1;
}

///////////////////////write status reg///////////////////////
int write_sr(char val)
{
	/*this command do'nt used to check busy bit otherwise cause error*/
	en_write_sr();

	SET_CE(0x01);
	send_spi_cmd(0x01);
	/*set status register*/
	send_spi_cmd(val);
	SET_CE(0x11);

	return 1;
}

///////////erase all memory/////////////
int erase_all(void)
{
	int i=1;
	spi_initw();
	set_wren();
	if (spi_wait_busy() < 0)
		return -1;

	SET_CE(0x01);
	send_spi_cmd(0xc7);
	SET_CE(0x11);
	while(i++) {
		if(read_sr() & 0x1) {
			if(i % 10000 == 0)
				printf(".");
		} else {
			printf("done...\n");
			break;
		}
	}
	return 1;
}

void spi_read_id(void)
{
	unsigned char val;
	char i;

	spi_initw();
	if (spi_wait_busy() < 0)
		return;

	/*CE 0*/
	SET_CE(0x01);
	/*READ ID CMD*/
	send_spi_cmd(0x90);

	/*address bit [23-0]*/
	for (i = 0;i < 3;i++) {
		send_spi_cmd(0);
	}

	/*Manufacturer’s ID*/
	val = send_spi_cmd(0);
	printf("Manufacturer's ID:         %x\n",val);
	/*Device ID*/
	val = send_spi_cmd(0);
	printf("Device ID:                 %x\n",val);
	/*CE 1*/
	SET_CE(0x11);
}

void spi_jedec_id(void)
{
	unsigned char val;
	spi_initw();

	if (spi_wait_busy() < 0)
		return;
	/*CE 0*/
	SET_CE(0x01);
	/*JEDEC ID CMD*/
	send_spi_cmd(0x9f);

	/*Manufacturer’s ID*/
	val = send_spi_cmd(0x00);
	printf("Manufacturer's ID:         %x\n",val);

	/*Device ID:Memory Type*/
	val = send_spi_cmd(0x00);
 	printf("Device ID-memory_type:     %x\n",val);

	/*Device ID:Memory Capacity*/
	val = send_spi_cmd(0x00);
	printf("Device ID-memory_capacity: %x\n",val);

	/*CE 1*/
	SET_CE(0x11);
}

void spi_write_byte(unsigned int addr,unsigned char data)
{
	write_sr(0x0);
	set_wren();
	if (spi_wait_busy() < 0)
		return;

	SET_CE(0x01);/*CE 0*/

	send_spi_cmd(0x2);

	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	/*send data(one byte)*/
	send_spi_cmd(data);

	/*CE 1*/
	SET_CE(0x11);
}

int write_pmon_byte(int argc,char ** argv)
{
	unsigned int addr;
	unsigned char val;
	if(argc != 3){
		printf("\nuse: write_pmon_byte  dst(flash addr) data\n");
		return -1;
	}
	addr = strtoul(argv[1],0,0);
	val = strtoul(argv[2],0,0);
	spi_write_byte(addr,val);
	return 0;
}

int write_pmon(int argc,char **argv)
{
	long int j=0;
	unsigned char val;
	unsigned int ramaddr,flashaddr,size;
	if(argc != 4){
		printf("\nuse: write_pmon src(ram addr) dst(flash addr) size\n");
		return -1;
	}

	ramaddr = strtoul(argv[1],0,0);
	flashaddr = strtoul(argv[2],0,0);
	size = strtoul(argv[3],0,0);

	spi_initw();
	write_sr(0);
	//read flash id command
	spi_read_id();
	val = GET_SPI(SPSR);
	printf("====spsr value:%x\n",val);

	SET_SPI(0x5,0x10);
	//erase the flash
	write_sr(0x00);
	//erase_all();
	printf("\nfrom ram 0x%08x  to flash 0x%08x size 0x%08x \n\nprogramming	    ",ramaddr,flashaddr,size);
	for(j=0;size > 0;flashaddr++,ramaddr++,size--,j++)
	{
		spi_write_byte(flashaddr,*((unsigned char*)ramaddr));
		if(j % 0x1000 == 0)
		printf("\b\b\b\b\b\b\b\b\b\b0x%08x",j);
	}
	printf("\b\b\b\b\b\b\b\b\b\b0x%08x end...\n",j);

	SET_CE(0x11);
	return 1;
}

int read_pmon_byte(unsigned int addr)
{
	unsigned char data;

	spi_wait_busy();
	SET_CE(0x01);
	// read flash command
	send_spi_cmd(0x03);
	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	data = send_spi_cmd(0x00);
	SET_CE(0x11);
	return data;
}

int read_pmon(int argc,char **argv)
{
	unsigned char data;
	int base = 0;
	int addr;
	int i;
	if(argc != 3) {
		printf("\nuse: read_pmon addr(flash) size\n");
		return -1;
	}
	addr = strtoul(argv[1],0,0);
	i = strtoul(argv[2],0,0);
	spi_initw();

	if (spi_wait_busy() < 0) {
		return -1;
	}
	/*CE 0*/
	SET_CE(0x01);
	// read flash command
	send_spi_cmd(0x3);
	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	printf("\n");
	while(i--) {
		data = send_spi_cmd(0x0);
		if(base % 16 == 0 ){
			printf("0x%08x    ",base);
		}
		printf("%02x ",data);
		if(base % 16 == 7)
			printf("  ");
		if(base % 16 == 15)
			printf("\n");
		base++;
	}
	printf("\n");
	return 1;
}

int spi_erase_area(unsigned int saddr,unsigned int eaddr,unsigned sectorsize)
{
	unsigned int addr;
	spi_initw();

	for(addr=saddr;addr<eaddr;addr+=sectorsize) {
		SET_CE(0x11);

		set_wren();
		write_sr(0x00);
		while(read_sr()&1);
		set_wren();
		SET_CE(0x01);
		/*
		 * 0x20 erase 4kbyte of memory array
		 * 0x52 erase 32kbyte of memory array
		 * 0xd8 erase 64kbyte of memory array
		 */
		send_spi_cmd(0x20);

		/*send addr [23 16]*/
		send_spi_cmd((addr >> 16) & 0xff);
		/*send addr [15 8]*/
		send_spi_cmd((addr >> 8) & 0xff);
		/*send addr [8 0]*/
		send_spi_cmd(addr & 0xff);
		SET_CE(0x11);

		while(read_sr()&1);
	}
	SET_CE(0x11);
	delay(10);

	return 0;
}

int spi_write_area(int flashaddr,char *buffer,int size)
{
	int j;
	spi_initw();
	SET_CE(0x01);
	write_sr(0x00);
	for(j=0;size > 0;flashaddr++,size--,j++) {
		spi_write_byte(flashaddr,buffer[j]);
		dotik(32, 0);
	}

	SET_CE(0x11);
	while(read_sr() & 1);
	SET_CE(0x11);
	delay(10);
	return 0;
}


int spi_read_area(int addr,char *buffer,int size)
{
	int i;
	spi_initw();

	SET_CE(0x01);

	send_spi_cmd(0x3);

	/*send addr [23 16]*/
	send_spi_cmd((addr >> 16) & 0xff);
	/*send addr [15 8]*/
	send_spi_cmd((addr >> 8) & 0xff);
	/*send addr [8 0]*/
	send_spi_cmd(addr & 0xff);

	for(i = 0;i < size; i++) {
		buffer[i] = send_spi_cmd(0x0);
	}

	SET_CE(0x11);
	delay(10);
	return 0;
}

struct fl_device myflash = {
	.fl_name="spiflash",
	.fl_size=0x100000,
	.fl_secsize=0x10000,
};

struct fl_device *spi_fl_devident(void *base, struct fl_map **m)
{
	if(m)
	*m = fl_find_map(base);
	return &myflash;
}

int spi_fl_program_device(void *fl_base, void *data_base, int data_size, int verbose)
{
	struct fl_map *map;
	int off;
	map = fl_find_map(fl_base);
	off = (int)(fl_base - map->fl_map_base) + map->fl_map_offset;
	spi_write_area(off,data_base,data_size);
	spi_initr();
	return 0;
}


int spi_fl_erase_device(void *fl_base, int size, int verbose)
{
	struct fl_map *map;
	int off;
	map = fl_find_map(fl_base);
	off = (int)(fl_base - map->fl_map_base) + map->fl_map_offset;
	spi_erase_area(off,off+size,0x1000);
	spi_initr();
	return 0;
}

static struct ls1x_spi {
	void	*base;
	int hz;
}  ls1x_spi0 = { 0x800000001fff0220} ;

struct spi_device spi_nand = 
{
.dev = &ls1x_spi0,
.chip_select = 1,
.max_speed_hz = 40000000,
}; 

struct spi_device spi_nand1 = 
{
.dev = &ls1x_spi0,
.chip_select = 0,
.max_speed_hz = 40000000,
}; 

int selected_lpc_spi(void *base)
{
	return 1;
}

int set_spi_base(int ac, unsigned char *av[])
{
	uint32_t bus_num;
	char dev_name[5] = "spi0";
	if(ac != 2) {
		printf("spi_base <spi bus num: 0/1/2...>\n");
		return 0;
	}
	bus_num = (uint32_t)strtoul(av[1], NULL, 0);
	dev_name[3] += bus_num;
	if (bus_num < 0 || bus_num > 5) {
		printf("error spi bus num: %d\n", bus_num);
		return 0;
	} else if (bus_num & 0x6) {
		ls_spi_base = LS2K1000_SPI2_BASE + (bus_num - 2) * 0x1000;
		printf("Note: now you select spi2~5 that has no spi memory space，just ignore the verify error after loading flash");
	} else {
		ls_spi_base = LS2K1000_SPI0_BASE + bus_num * 0x40000;
	}
	//cfg_func_multi(dev_name, 0);
	return 0;
}

static char ls1x_spi_write_reg(struct ls1x_spi *spi, 
				unsigned char reg, unsigned char data)
{
	(*(volatile unsigned char *)(spi->base +reg)) = data;
}

static char ls1x_spi_read_reg(struct ls1x_spi *spi, 
				unsigned char reg)
{
	return(*(volatile unsigned char *)(spi->base + reg));
}


static int 
ls1x_spi_write_read_8bit(struct spi_device *spi,
  const u8 **tx_buf, u8 **rx_buf, unsigned int num)
{
	struct ls1x_spi *ls1x_spi = spi->dev;
	unsigned char value;
	int i, ret;
	
	for(i = 0; i < 4 && i < num; i++) {
		if (tx_buf && *tx_buf)
			value = *((*tx_buf)++);
		else 
			value = 0;
		ls1x_spi_write_reg(ls1x_spi, FIFO, value);
	}

	ret = i;

	for(;i > 0; i--) {
 		while((ls1x_spi_read_reg(ls1x_spi, SPSR) & 0x1) == 1);
		value = ls1x_spi_read_reg(ls1x_spi, FIFO);
		if (rx_buf && *rx_buf) 
			*(*rx_buf)++ = value;
	}


	return ret;
}


static unsigned int
ls1x_spi_write_read(struct spi_device *spi, struct spi_transfer *xfer)
{
	struct ls1x_spi *ls1x_spi;
	unsigned int count;
	int ret;
	const u8 *tx = xfer->tx_buf;
	u8 *rx = xfer->rx_buf;

	ls1x_spi = spi->dev;
	count = xfer->len;

	do {
		if ((ret = ls1x_spi_write_read_8bit(spi, &tx, &rx, count)) < 0)
			goto out;
		count -= ret;
	} while (count);

out:
	return xfer->len - count;
	//return count;

}


#define DIV_ROUND_UP(n,d)       (((n) + (d) - 1) / (d))
static int ls_spi_setup(struct ls1x_spi *ls1x_spi,  struct spi_device *spi)
{
	unsigned int hz;
	unsigned int div, div_tmp;
	unsigned int bit;
	unsigned long clk;
	unsigned char val, spcr, sper;
	const char rdiv[12]= {0,1,4,2,3,5,6,7,8,9,10,11}; 

	hz  = spi->max_speed_hz;

	if (hz) {
		clk = 100000000;
		div = DIV_ROUND_UP(clk, hz);

		if (div < 2)
			div = 2;

		if (div > 4096)
			div = 4096;

		bit = fls(div) - 1;
		if((1<<bit) == div) bit--;
		div_tmp = rdiv[bit];
		ls1x_spi->hz = hz;
		spcr = div_tmp & 3;
		sper = (div_tmp >> 2) & 3;

		val = ls1x_spi_read_reg(ls1x_spi, SPCR);
		ls1x_spi_write_reg(ls1x_spi, SPCR, (val & ~3) | spcr);
		val = ls1x_spi_read_reg(ls1x_spi, SPER);
		ls1x_spi_write_reg(ls1x_spi, SPER, (val & ~3) | sper);

	}
	return 0;
}



int spi_sync(struct spi_device *spi, struct spi_message *m)
{

	struct ls1x_spi *ls1x_spi = &ls1x_spi0;
	struct spi_transfer *t = NULL;
	unsigned long flags;
	int cs;
	int param;
	ls_spi_setup(ls1x_spi, spi);
	
	m->actual_length = 0;
	m->status		 = 0;

	if (list_empty(&m->transfers) /*|| !m->complete*/)
		return -EINVAL;


	list_for_each_entry(t, &m->transfers, transfer_list) {
		
		if (t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			printf("message rejected : "
				"invalid transfer data buffers\n");
			goto msg_rejected;
		}

	/*other things not check*/

	}

	param = ls1x_spi_read_reg(ls1x_spi, PARAM);
	ls1x_spi_write_reg(ls1x_spi, PARAM, param&~1);

	cs = 0xff & ~(0x11<<spi->chip_select);
	ls1x_spi_write_reg(ls1x_spi, SOFTCS, (0x1 << spi->chip_select)|cs);

	list_for_each_entry(t, &m->transfers, transfer_list) {

		if (t->len)
			m->actual_length +=
				ls1x_spi_write_read(spi, t);
	}

	ls1x_spi_write_reg(ls1x_spi, SOFTCS, (0x11<<spi->chip_select)|cs);
	ls1x_spi_write_reg(ls1x_spi, PARAM, param);

	return 0;
msg_rejected:

	m->status = -EINVAL;
 	if (m->complete)
		m->complete(m->context);
	return -EINVAL;
}


#if NSPINAND_MT29F||NSPINAND_LLD
int spinand_probe(struct spi_device *spi_nand);

int ls2k_spi_nand_probe()
{
    spi_initw();

#ifdef CONFIG_SPINAND_CS
spi_nand.chip_select = CONFIG_SPINAND_CS;
#endif
spinand_probe(&spi_nand);
    spi_initr();
}
#endif

#if NM25P80
int ls2k_m25p_probe()
{
    spi_initw();
    m25p_probe(&spi_nand1, "gd25q80");
    m25p_probe(&spi_nand, "gd25q256");
    spi_initr();
}
#endif

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"spi_base", "", NULL, "set spi base", set_spi_base , 0, 99, 1},
	{"spi_initw","",0,"spi_initw(sst25vf080b)",spi_initw,0,99,CMD_REPEAT},
	{"read_pmon","",0,"read_pmon(sst25vf080b)",read_pmon,0,99,CMD_REPEAT},
	{"write_pmon","",0,"write_pmon(sst25vf080b)",write_pmon,0,99,CMD_REPEAT},
	{"erase_all","",0,"erase_all(sst25vf080b)",erase_all,0,99,CMD_REPEAT},
	{"write_pmon_byte","",0,"write_pmon_byte(sst25vf080b)",write_pmon_byte,0,99,CMD_REPEAT},
	{"read_flash_id","",0,"read_flash_id(sst25vf080b)",spi_read_id,0,99,CMD_REPEAT},
	{"spi_id","",0,"read_flash_id(sst25vf080b)",spi_jedec_id,0,99,CMD_REPEAT},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(Cmds,1);
}
