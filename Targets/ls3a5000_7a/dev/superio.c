#include <sys/types.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include <pmon.h>

static int superio_read(int dev, int addr)
{
	int data;
	/*enter*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0x87);
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0x87);
	/*select logic dev reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0x7);
	outb(BONITO_PCIIO_BASE_VA + 0x002f, dev);
	/*access reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e, addr);
	data=inb(BONITO_PCIIO_BASE_VA + 0x002f);
	/*exit*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0xaa);
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0xaa);
	return data;
}

static void superio_write(int dev, int addr, int data)
{
	/*enter*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0x87);
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0x87);
	/*select logic dev reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0x7);
	outb(BONITO_PCIIO_BASE_VA + 0x002f, dev);
	/*access reg */
	outb(BONITO_PCIIO_BASE_VA + 0x002e, addr);
	outb(BONITO_PCIIO_BASE_VA + 0x002f, data);
	/*exit*/
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0xaa);
	outb(BONITO_PCIIO_BASE_VA + 0x002e, 0xaa);
}

void superio_reinit()
{
	/*enable KBC*/
	superio_write(5, 0x30, 1);
	superio_write(5, 0x60, 0);	//set KBC base address @0xb8000060
	superio_write(5, 0x61, 0x60);
	superio_write(5, 0x62, 0);
	superio_write(5, 0x63, 0x64);
	superio_write(5, 0x70, 1);
	superio_write(5, 0x72, 0xc);
	superio_write(5, 0xf0, 0x80);
}
