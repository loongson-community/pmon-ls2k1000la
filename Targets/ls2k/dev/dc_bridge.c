#include <pmon.h>
#include <linux/types.h>
void i2c_init(int speed,  int slaveaddr);
int i2c_read(u8 chip, uint addr, int alen, u8 *buffer, int len);
int i2c_write(u8 chip, uint addr, int alen, u8 *buffer, int len);
#if  defined(SII9022A)
#define CHIP_9022_ADDR	(0x72/2)

void config_dc_bridge(void)
{
	unsigned char id0;
	unsigned char id1;
	unsigned char id2;
	unsigned char data;

	unsigned char dev_addr = CHIP_9022_ADDR;

#define  gpioi2c_write(chip, reg, val) data = val; i2c_write(chip, reg, 1, &data, 1);
#define  gpioi2c_read(chip, reg, data) i2c_read(chip, reg, 1, data, 1);

	ls2k_i2c_init(0, 0x800000001fe21800);

	gpioi2c_write(dev_addr, 0xc7, 0x00);
	gpioi2c_read(dev_addr, 0x1b, &id0);
	gpioi2c_read(dev_addr, 0x1c, &id1);
	gpioi2c_read(dev_addr, 0x1d, &id2);

	if (id0 != 0xb0 || id1 != 0x2 || id2 != 0x3) {
		printf("id err\n");
		return;
	}
	
	gpioi2c_read(dev_addr, 0x1e, &data);
	data &= ~(0x3);
	gpioi2c_write(dev_addr, 0x1e, data);

	gpioi2c_read(dev_addr, 0x1a, &data);
	data &= ~(1 << 4);
	gpioi2c_write(dev_addr, 0x1a, data);
}
#elif defined(ADV7513)
#define ADV7513_ADDR	(0x7a/2)

void config_dc_bridge(void)
{
	unsigned char id0;
	unsigned char id1;
	unsigned char id2;
	unsigned char data;

	unsigned char dev_addr = ADV7513_ADDR;

#define  gpioi2c_write(chip, reg, val) data = val; i2c_write(chip, reg, 1, &data, 1);
#define  gpioi2c_read(chip, reg, data) i2c_read(chip, reg, 1, data, 1);

	ls2k_i2c_init(0, 0x800000001fe21800);
	
    gpioi2c_write(dev_addr, 0x43, 0xa0);

	gpioi2c_write(dev_addr, 0x41, 0x10);
	gpioi2c_write(dev_addr, 0x98, 0x3);
	gpioi2c_write(dev_addr, 0x9a, 0xe0);
	gpioi2c_write(dev_addr, 0x9c, 0x30);
	gpioi2c_write(dev_addr, 0x9d, 0x61);
	gpioi2c_write(dev_addr, 0xa2, 0xa4);
	gpioi2c_write(dev_addr, 0xa3, 0xa4);
	gpioi2c_write(dev_addr, 0xe0, 0xd0);
	gpioi2c_write(dev_addr, 0xf9, 0x0);
	//gpioi2c_write(dev_addr, 0x0, 0x0);
	gpioi2c_write(dev_addr, 0x41, 0x10);
}

#endif
