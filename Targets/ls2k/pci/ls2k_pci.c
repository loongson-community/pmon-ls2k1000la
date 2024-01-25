#include <linux/types.h>
#include <types.h>
#include <sys/param.h>
#include "target/bonito.h"
#include "sys/dev/pci/pcireg.h"
#include <dev/pci/pcivar.h>
#include <dev/pci/nppbreg.h>



typedef unsigned long device_t;

extern struct pci_device *_pci_bus[16];
extern int _max_pci_bus;

static int is_pcie_root_port(int bus)
{
	int i, exp;
	struct pci_device *pd;

	if (bus == 0)
		return 0;
	for (i = 1; i < _max_pci_bus; i++) {
		pd = _pci_bus[i];
		if (!pd)
			break;
		if (pd->bridge.secbus_num == bus) {
			if (pd->bridge.pribus_num == 0)
				return 1;
			if (pd->pcie_type == PCI_EXP_TYPE_ROOT_PORT)
				return 1;
			return 0;
		}
	}
	return 0;
}

u32 pci_read_type0_config32(u32 dev, u32 func, u32 reg)
{
	unsigned long long addr = BONITO_PCICFG0_BASE_VA;
	addr |= (dev << 11) | (func << 8) | (reg&0xff);
	if(reg > 0xff){
		addr |= ((reg >> 8)&0xf) << 24;
	}
	return *((uint32_t *) addr);
}

void pci_write_type0_config32(u32 dev, u32 func, u32 reg, u32 val)
{
	unsigned long long addr = BONITO_PCICFG0_BASE_VA;
	addr |=  (dev << 11) | (func << 8) | (reg&0xff);
	if(reg > 0xff){
		addr |= ((reg >> 8)&0xf) << 24;
	}
	addr |= (dev << 11 | func << 8 | reg);
	*((uint32_t *) addr) = val;
}

uint32_t pci_read_type1_config32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg)
{
	unsigned long long addr = BONITO_PCICFG1_BASE_VA;
	addr |= (bus << 16) | (dev << 11) | (func << 8) | (reg&0xff);
	if(reg > 0xff){
		addr |= ((reg >> 8)&0xf) << 24;
	}
	return *((uint32_t *) addr);
}

void pci_write_type1_config32(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg, uint32_t val)
{
	unsigned long long addr = BONITO_PCICFG1_BASE_VA;
	addr |= (bus << 16) | (dev << 11) | (func << 8) | (reg&0xff);
	if(reg > 0xff){
		addr |= ((reg >> 8)&0xf) << 24;
	}
	addr |= (bus << 16 | dev << 11 | func << 8 | reg);
	*((uint32_t *) addr) = val;
}

uint32_t _pci_conf_readn(device_t tag, uint8_t reg, int width)
{
	uint8_t bus, device, function;
	u32 data;

	if ((reg & (width-1)) || reg < 0 || reg >= 0x1000) {
		printf("_pci_conf_readn: bad reg 0x%x, tag 0x%x, width 0x%x\n", reg, tag, width);
		return ~0;
	}

	_pci_break_tag (tag, &bus, &device, &function);
	if(bus == 0 && device == 2) return -1;
//	if(bus == 0 && (device >= 10 && device <= 14 || device == 3 || device == 8)) return -1;	//skip pcie, gmac, sata
	//workaround pcie header
	if(bus == 0 && (device >=9 && device <= 14) && reg == 0x8){
		return 0x06040001;
	}

	if (bus == 0) {
		/* Type 0 configuration on onboard PCI bus */
		if (device > 31 || function > 7)
		{
			printf("_pci_conf_readn: bad device 0x%x, function 0x%x\n", device, function);
			return ~0;		/* device out of range */
		}
		if (reg == 0x78) {
			data = pci_read_type0_config32(device, function, reg);
			data &= ~0x7000;
			/*set mrrs same with mps so mps must be setted*/
			data |= (data & 0xe0) << 7;

			printf("data 0x%x \n",data);
			return data;
		}
		data = pci_read_type0_config32(device, function, reg & ~3);
	}
	else {


	if (is_pcie_root_port(bus) && device > 0)
		return ~0;		/* device out of range */
		/* Type 1 configuration on offboard PCI bus */
		if (bus > 255 || device > 31 || function > 7)
		{
    		//	printf("_pci_conf_readn: bad bus 0x%x, device 0x%x, function 0x%x\n", bus, device, function);
			return ~0;		/* device out of range */
		}
		data = pci_read_type1_config32(bus, device, function, reg & ~3);
	}

	/* move data to correct position */
	if (width == 1)
		data = (data >> ((reg & 3) << 3)) & 0xff;
	else if (width == 2)
		data = (data >> ((reg & 3) << 3)) & 0xffff;
	else
		data = data;

	return data;

}


uint32_t _pci_conf_read(device_t tag, uint8_t reg)
{
	return _pci_conf_readn(tag, reg, 4);
}

u32 _pci_conf_read32(device_t tag,uint8_t reg)
{
	return _pci_conf_readn(tag,reg,4);
}

u8 _pci_conf_read8(device_t tag,uint8_t reg)
{
	u32 data;
	u32 offset;
	u32 new_reg;

	new_reg = reg & 0xfc;
	data = _pci_conf_readn(tag,new_reg,4);
	offset = reg & 3;
	data = data >> (offset * 8);
	data &= 0xff;

	return (u8)data;
}


u16 _pci_conf_read16(device_t tag,uint8_t reg)
{
	u32 data;
	u32 offset;
	u32 new_reg;

	new_reg = reg & 0xfc;
	data = _pci_conf_readn(tag,new_reg,4);
	offset = reg & 2;
	data = data >> (offset << 3);
	data &= 0xffff;

	return (u16)data;
}

void _pci_conf_write(device_t tag, uint8_t reg, u32 data)
{
	return _pci_conf_writen(tag, reg, data, 4);
}

void _pci_conf_write32(device_t tag, uint8_t reg, u32 data)
{
	return _pci_conf_writen(tag, reg, data, 4);
}

void _pci_conf_write8(device_t tag, uint8_t reg, u8 data)
{
	return _pci_conf_writen(tag, reg, data, 1);
}

void _pci_conf_write16(device_t tag, uint8_t reg, u16 data)
{
	return _pci_conf_writen(tag, reg, data, 2);
}

void _pci_conf_writen(device_t tag, uint8_t reg, uint32_t val, int width)
{
	uint8_t bus, device, function;
	u32 data;

	if ((reg & (width -1)) || reg < 0 || reg >= 0x1000) {
		printf("_pci_conf_writen: bad reg 0x%x, tag 0x%x, width 0x%x\n", reg, tag, width);
		return;
	}

	_pci_break_tag (tag, &bus, &device, &function);

	if (bus == 0) {
    	/* Type 0 configuration on onboard PCI bus */
    		if (device > 31 || function > 7)
    		{
			printf("_pci_conf_writen: bad device 0x%x, function 0x%x\n", device, function);
			return;		/* device out of range */
		}
	} else {
    	/* Type 1 configuration on offboard PCI bus */
		if (is_pcie_root_port(bus) && device > 0)
			return ;		/* device out of range */

		if (bus > 127 || device > 31 || function > 7)
		{
			printf("_pci_conf_writen: bad bus 0x%x, device 0x%x, function 0x%x\n", bus, device, function);
			return;		/* device out of range */
		}
	}

	if (width == 4)
		data = val;
	else {
		data = _pci_conf_read(tag, reg & ~3);

		if (width == 1)
			data = (data & ~(0xff << ((reg & 3) << 3))) |
				(val << ((reg & 3) << 3));
		else if (width == 2)
			data = (data & ~(0xffff << ((reg & 3) << 3))) |
				(val << ((reg & 3) << 3));
	}

	if (bus == 0) {
		return pci_write_type0_config32(device, function, reg & ~3, data);
	}
	else {
		return pci_write_type1_config32(bus, device, function, reg & ~3, data);
	}
}
