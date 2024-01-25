#include <include/stdio.h>
#include <linux/io.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dev/pci/pcivar.h>
#include <cpu.h>

void clear_pcie_inter_irq(void)
{
	unsigned int dev;
	unsigned int val;
	uint64_t addr;
	int i;
	for (i = 9; i < 21; i++) {
		dev = _pci_make_tag(0, i, 0);
		val = _pci_conf_read(dev, 0x00);
		if (val != 0xffffffff) {
			addr = PHYS_TO_UNCACHED(_pci_conf_read(dev, 0x10) & (~0xf));
			val = inl(addr + 0x18);
			if (val) {
				outl(addr + 0x1c, val);
			}
		}
	}
}

int ls7a_get_irq(unsigned char dev, int fn)
{
	int irq;
	switch (dev) {
	case 2:
		/*APB 2*/
		irq = 0;
		break;

	case 3:
		/*GMAC0 3 0*/
		/*GMAC1 3 1*/
		irq = (fn == 0) ? 12 : 14;
		break;

	case 4:
		/* ohci:4 0 */
		/* ehci:4 1 */
		irq = (fn == 0) ? 49 : 48;
		break;

	case 5:
		/* ohci:5 0 */
		/* ehci:5 1 */
		irq = (fn == 0) ? 51 : 50;
		break;

	case 6:
		/* DC: 6 1 28 */
		/* GPU:6 0 29 */
		irq = (fn == 0) ? 29 : 28;
		break;

	case 7:
		/*HDA: 7 0 58 */
		irq = 58;
		break;

	case 8:
		/* sata */
		if (fn == 0)
			irq = 16;
		if (fn == 1)
			irq = 17;
		if (fn == 2)
			irq = 18;
		break;

	case 9:
		/* pcie_f0 port0 */
		irq = 32;
		break;

	case 10:
		/* pcie_f0 port1 */
		irq = 33;
		break;

	case 11:
		/* pcie_f0 port2 */
		irq = 34;
		break;

	case 12:
		/* pcie_f0 port3 */
		irq = 35;
		break;

	case 13:
		/* pcie_f1 port0 */
		irq = 36;
		break;

	case 14:
		/* pcie_f1 port1 */
		irq = 37;
		break;

	case 15:
		/* pcie_g0 port0 */
		irq = 40;
		break;

	case 16:
		/* pcie_g0 port1 */
		irq = 41;
		break;

	case 17:
		/* pcie_g1 port0 */
		irq = 42;
		break;

	case 18:
		/* pcie_g1 port1 */
		irq = 43;
		break;

	case 19:
		/* pcie_h port0 */
		irq = 38;
		break;

	case 20:
		/* pcie_h port1 */
		irq = 39;
		break;
	}
	return irq + 64;
}

void ls7a_pcie_interrupt_fixup(void)
{
	extern struct pci_device *_pci_head;
	extern int pci_roots;

	unsigned int tag;
	unsigned char i, irq;
	struct pci_device *pd, *pdd, *pddd;

	for (i = 0, pd = _pci_head; i < pci_roots; i++, pd = pd->next) {
		for (pdd = pd->bridge.child; pdd != NULL; pdd = pdd->next) {
			//printf("- bus %d device %d function %d\n", pdd->pa.pa_bus, pdd->pa.pa_device, pdd->pa.pa_function);
			tag = _pci_make_tag(pdd->pa.pa_bus, pdd->pa.pa_device, pdd->pa.pa_function);
			irq = ls7a_get_irq(pdd->pa.pa_device, pdd->pa.pa_function);
			_pci_conf_write8(tag, 0x3c, irq);
			//printf("irq -> %d\n", 0xff & _pci_conf_read(tag, 0x3c));
			for (pddd = pdd->bridge.child; pddd != NULL; pddd = pddd->next) {
				//printf("- bus %d device %d function %d\n", pddd->pa.pa_bus, pddd->pa.pa_device, pddd->pa.pa_function);
				tag = _pci_make_tag(pddd->pa.pa_bus, pddd->pa.pa_device, pddd->pa.pa_function);
				_pci_conf_write8(tag, 0x3c, irq);
				//printf("irq -> %d\n", 0xff & _pci_conf_read(tag, 0x3c));
			}
		}
	}
}

void ls7a_pcie_irq_fixup(void)
{
	clear_pcie_inter_irq();
	ls7a_pcie_interrupt_fixup();
}
