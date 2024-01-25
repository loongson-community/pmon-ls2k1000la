#include <pmon.h>
#include <stdio.h>

#define SCL1	3
#define	SDA1	2

#define SCL0	1
#define	SDA0	0

#define	LOW	0
#define HIGH	1

#define	OUT	0
#define	IN	1

static unsigned long dc_base = 0;
static unsigned long gpio_val_base = 0;
static unsigned long gpio_dir_base = 0;

static void gpioi2c_init(ac, av)
    int ac;
    char *av[];
{
	int val;
	unsigned long long header;
	unsigned int bar0_low;
	unsigned int bar0_high;
	unsigned int status,desc_num;

	val = (unsigned char)strtoul(av[1], 0, 0); //gpio

	header = (PHYS_TO_UNCACHED(0xefdfe000000) | (6 << 11) | (1 << 8));

	bar0_low  = (readl(header + 0x10) & 0xffffffff0);
	bar0_high = readl(header + 0x14);

	dc_base = PHYS_TO_UNCACHED(bar0_high << 32 | bar0_low);

	printf("pcie header = 0x%llx\n",header);
	printf("pcie bar0_low = 0x%x\n",bar0_low);
	printf("pcie bar0_high = 0x%x\n",bar0_high);
	printf("pcie dc base = 0x%llx\n",dc_base);

	gpio_val_base = dc_base + 0x1650;
	gpio_dir_base = dc_base + 0x1660;

	if(val)
		readl(dc_base + 0x1730) &= ~(1<<8);
	else
		readl(dc_base + 0x1720) &= ~(1<<8);
#if 1
	printf("gpioi2c init regs base\n");
	printf("gpio_val_base 0x%llx\n", gpio_val_base);
	printf("gpio_dir_base 0x%llx\n", gpio_dir_base);
#endif
}

static void set_gpio_value(int gpio, int val)
{
	if (val == 1)
		*(volatile unsigned int *)gpio_val_base |= (1U << gpio);
	else
		*(volatile unsigned int *)gpio_val_base &= ~(1U << gpio);
}

static unsigned int get_gpio_value(int gpio)
{
	return (*(volatile unsigned int *)gpio_val_base & (1U << gpio)) ? 1 : 0;
}

static void set_gpio_direction(unsigned int gpio, int val)
{
	if (val == 1)
		*(volatile unsigned int *)gpio_dir_base |= (1 << gpio);
	else
		*(volatile unsigned int *)gpio_dir_base &= ~(1 << gpio);
}

static int gpioi2c_read_ack(int scl, int sda)
{
	set_gpio_direction(sda, IN);
	delay(10);

	set_gpio_value(scl, HIGH);
	delay(10);

	/* the ack signal will hold on sda untill scl falling edge */
	while(get_gpio_value(sda))
		delay(10);

	set_gpio_value(scl, LOW);
	delay(100);

	return 0;
}

static int gpioi2c_write_byte(unsigned char i2c, unsigned char c)
{
	int i;
	int sda, scl;

	if (i2c) {
		sda = SDA1;
		scl = SCL1;
	} else {
		sda = SDA0;
		scl = SCL0;
	}

	set_gpio_direction(sda, OUT);

	for(i = 7; i >= 0; i--) {
		set_gpio_value(scl, LOW);
		delay(5);
		set_gpio_value(sda, (c & (1U << i)) ? 1 : 0);//high bit --> low bit
		delay(5);
		set_gpio_value(scl, HIGH);
		delay(10);
	}

	set_gpio_value(scl, LOW);
	delay(10);

	if (gpioi2c_read_ack(scl, sda)) {
		printf("read slave dev ack invalid\n");//0:valid
		return 0;
	}
	return 1;
}

static void gpioi2c_read_byte(unsigned char i2c, unsigned char *c)
{
	int i;
	int sda, scl;

	if (i2c) {
		sda = SDA1;
		scl = SCL1;
	} else {
		sda = SDA0;
		scl = SCL0;
	}

	*c = 0;
	set_gpio_direction(sda, IN);

	for (i = 7; i >= 0; i--) {
		set_gpio_value(scl, HIGH);
		delay(10);
		*c = (*c << 1) | get_gpio_value(sda);
		set_gpio_value(scl, LOW);
		delay(10);
	}
}

static void gpioi2c_start(unsigned char i2c)
{
	int sda, scl;

	if (i2c) {
		sda = SDA1;
		scl = SCL1;
	} else {
		sda = SDA0;
		scl = SCL0;
	}

	/* if set sda output without setting sda high,
	 * the sda output value may be low
	 * */
	set_gpio_value(sda, HIGH);

	set_gpio_direction(sda, OUT);
	set_gpio_direction(scl, OUT);

	set_gpio_value(scl, HIGH);
	delay(10);

	/* start signal: sda from high to low */
	set_gpio_value(sda, HIGH);
	delay(10);
	set_gpio_value(sda, LOW);
	delay(10);
}

static void gpioi2c_stop(unsigned char i2c)
{
	int sda, scl;

	if (i2c) {
		sda = SDA1;
		scl = SCL1;
	} else {
		sda = SDA0;
		scl = SCL0;
	}

	set_gpio_direction(sda, OUT);

	set_gpio_value(scl, HIGH);
	delay(10);

	set_gpio_value(sda, LOW);
	delay(10);
	set_gpio_value(sda, HIGH);
	delay(10);
}

static void gpioi2c_write(unsigned char dev_addr, unsigned char data_addr, unsigned char data, unsigned char i2c)
{
	gpioi2c_start(i2c);
	if (!gpioi2c_write_byte(i2c, dev_addr)) {
		printf("gpioi2c write dev_addr fail\n");
		return;
	}
	if (!gpioi2c_write_byte(i2c, data_addr)) {
		printf("gpioi2c write data_addr fail\n");
		return;
	}
	if (!gpioi2c_write_byte(i2c, data)) {
		printf("gpioi2c write data fail\n");
		return;
	}
	gpioi2c_stop(i2c);
}

static void gpioi2c_read(unsigned char dev_addr, unsigned char data_addr, unsigned char *data, unsigned char i2c)
{
	/* bit0 :1 read, 0 write */

	gpioi2c_start(i2c);

	if (!gpioi2c_write_byte(i2c, dev_addr)) {
		printf("gpioi2c write dev_addr fail\n");
		return;
	}

	if (!gpioi2c_write_byte(i2c, data_addr)) {
		printf("gpioi2c write data_addr fail\n");
		return;
	}

	gpioi2c_start(i2c);
	if (!gpioi2c_write_byte(i2c, dev_addr | 0x01)) {//for read
		printf("gpioi2c write dev_addr fail\n");
		return;
	}

	gpioi2c_read_byte(i2c, data);

	gpioi2c_stop(i2c);
}

int gpio_read_edid(ac, av)
    int ac;
    char *av[];
{
	int i, num;
	uint8_t dev_addr,data, i2c;

	dev_addr = (unsigned char)strtoul(av[1], 0, 0);
	num = (unsigned char)strtoul(av[2], 0, 0);
	i2c = (unsigned char)strtoul(av[3], 0, 0);

	for (i = 0; i < num; i++) {
		gpioi2c_read(dev_addr, i, &data, i2c);
		printf("reg 0x%x value 0x%x\n", i, data);
	}
}

static const Cmd Cmds[] = {
	{"Misc"},
	{"gpio_read_edid", "gpio addr regnum", 0, "gpio_read_edid", gpio_read_edid, 1, 5, 0},
	{"dc_gpio_init", "dc gpio init", 0, "gpio init", gpioi2c_init, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
