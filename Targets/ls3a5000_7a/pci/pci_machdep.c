/*	$Id: pci_machdep.c,v 1.1.1.1 2006/09/14 01:59:09 root Exp $ */

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

#include <sys/param.h>
#include <sys/device.h>
#include <sys/systm.h>

#include <cpu.h>
#include <sys/malloc.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>

#include <machine/bus.h>

#include "include/bonito.h"

#include <pmon.h>

extern void *pmalloc __P((size_t ));
extern int _pciverbose;

extern char hwethadr[6];
struct pci_device *_pci_bus[16];
int _max_pci_bus = 0;


/* PCI mem regions in PCI space */

/* soft versions of above */
static pcireg_t pci_local_mem_pci_base;


/****************************/
/*initial PCI               */
/****************************/

int
_pci_hwinit(initialise, iot, memt)
	int initialise;
	bus_space_tag_t iot;
	bus_space_tag_t memt;
{
	/*pcireg_t stat;*/
	struct pci_device *pd;
	struct pci_bus *pb;
	struct pci_device *ptr_d;
	struct pci_bus *ptr_b;
	int i;
	int newcfg = 0;
	if (getenv("newcfg"))
		newcfg=1;

	if (!initialise) {
		return(0);
	}

	/*
	 *  Allocate and initialize PCI bus heads.
	 */

	/*
	 * PCI Bus 0
	 */
	for (i = 0; i < TOT_7A_NUM; i++) {
		pd = pmalloc(sizeof(struct pci_device));
		pb = pmalloc(sizeof(struct pci_bus));
		if (pd == NULL || pb == NULL) {
			printf("pci: can't alloc memory. pci not initialized\n");
			return(-1);
		}

		pd->pa.pa_flags = PCI_FLAGS_IO_ENABLED | PCI_FLAGS_MEM_ENABLED;
		pd->pa.pa_iot = pmalloc(sizeof(bus_space_tag_t));
		pd->pa.pa_iot->bus_reverse = 1;
		pd->pa.pa_iot->bus_base = BONITO_PCIIO_BASE_VA;
		pd->pa.pa_memt = pmalloc(sizeof(bus_space_tag_t));
		pd->pa.pa_memt->bus_reverse = 1;
		pd->pa.pa_memt->bus_base = PHYS_TO_UNCACHED(0x0e0000000000);
		/* dual 7A anther 7A must in ls7a_link_id_buf[1] */
		if (i > 0) {
			extern char ls7a_link_id_buf[];
			uint64_t node_id = ls7a_link_id_buf[1];
			pd->pa.pa_iot->bus_base |=  (node_id << NODE_OFFSET);
			pd->pa.pa_memt->bus_base |= (node_id << NODE_OFFSET);
		}
		pd->pa.pa_dmat = &bus_dmamap_tag;
		pd->bridge.secbus = pb;
		if (i == 0)
			_pci_head = pd;
		else
			ptr_d->next = pd;
		ptr_d = pd;

#ifdef LS3_HT /* whd */
		pb->minpcimemaddr  = BONITO_PCILO0_BASE;
		pb->nextpcimemaddr = BONITO_PCILO0_BASE + BONITO_PCILO_SIZE;
		pb->minpciioaddr   = PCI_IO_SPACE_BASE + 0x0020000;
		pb->nextpciioaddr  = PCI_IO_SPACE_BASE + BONITO_PCIIO_SIZE;
		pb->pci_mem_base   = BONITO_PCILO_BASE_VA;
		pb->pci_io_base    = BONITO_PCIIO_BASE_VA;
#else
		if (newcfg) {
			pb->minpcimemaddr  = BONITO_PCILO1_BASE;
			pb->nextpcimemaddr = BONITO_PCIHI_BASE; 
			pd->pa.pa_memt->bus_base = 0xa0000000;
			BONITO_PCIMAP =
			BONITO_PCIMAP_WIN(0, PCI_MEM_SPACE_PCI_BASE + 0x00000000) |
			BONITO_PCIMAP_WIN(1, PCI_MEM_SPACE_PCI_BASE + 0x14000000) |
			BONITO_PCIMAP_WIN(2, PCI_MEM_SPACE_PCI_BASE + 0x18000000) |
			BONITO_PCIMAP_PCIMAP_2;
		}
		else {
			pb->minpcimemaddr  = 0x04000000;
			pb->nextpcimemaddr = 0x08000000; 
			pd->pa.pa_memt->bus_base = 0xb0000000;
			BONITO_PCIMAP =
			BONITO_PCIMAP_WIN(0, PCI_MEM_SPACE_PCI_BASE + 0x00000000) |
			BONITO_PCIMAP_WIN(1, PCI_MEM_SPACE_PCI_BASE + 0x04000000) |
			BONITO_PCIMAP_WIN(2, PCI_MEM_SPACE_PCI_BASE + 0x08000000) |
			BONITO_PCIMAP_PCIMAP_2;
		}
		pb->minpciioaddr  = 0x0004000;
		pb->nextpciioaddr = BONITO_PCIIO_SIZE;
		pb->pci_mem_base  = BONITO_PCILO_BASE_VA;
		pb->pci_io_base   = BONITO_PCIIO_BASE_VA;
#endif

		pb->max_lat = 255;
		pb->fast_b2b = 1;
		pb->prefetch = 1;
		pb->bandwidth = 4000000;
		pb->ndev = 1;
		if (i == 0)
			_pci_bushead = pb;
		else
			ptr_b->next = pb;
		ptr_b = pb;

		_pci_bus[_max_pci_bus++] = pd;

		bus_dmamap_tag._dmamap_offs = 0;

/*set pci base0 address and window size*/
		pci_local_mem_pci_base = 0x0;
#ifdef LS3_HT
#else
		BONITO_PCIBASE0 = 0x80000000;
#endif
	}

	return(TOT_7A_NUM);
}

/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible. We also set up the interrupt controller
 * routing and level control registers.
 */
void _pci_hwreinit (void)
{
}

void _pci_flush (void)
{
}

/*
 *  Map the CPU virtual address of an area of local memory to a PCI
 *  address that can be used by a PCI bus master to access it.
 */
vm_offset_t
_pci_dmamap(va, len)
	vm_offset_t va;
	unsigned int len;
{
#if 0
	return(VA_TO_PA(va) + bus_dmamap_tag._dmamap_offs);
#endif
	return(pci_local_mem_pci_base + VA_TO_PA (va));
}


#if 1
/*
 *  Map the PCI address of an area of local memory to a CPU physical
 *  address.
 */
vm_offset_t
_pci_cpumap(pcia, len)
	vm_offset_t pcia;
	unsigned int len;
{
	return PA_TO_VA(pcia - pci_local_mem_pci_base);
}
#endif


/*
 *  Make pci tag from bus, device and function data.
 */
pcitag_t
_pci_make_tag(bus, device, function)
	uint8_t bus;
	uint8_t device;
	uint8_t function;
{
	pcitag_t tag;

	tag = (bus << 16) | (device << 11) | (function << 8);
	return(tag);
}

/*
 *  Break up a pci tag to bus, device function components.
 */
void
_pci_break_tag(tag, busp, devicep, functionp)
	pcitag_t tag;
	uint8_t *busp;
	uint8_t *devicep;
	uint8_t *functionp;
{
	if (busp) {
		*busp = (tag >> 16) & 255;
	}
	if (devicep) {
		*devicep = (tag >> 11) & 31;
	}
	if (functionp) {
		*functionp = (tag >> 8) & 7;
	}
}

int _pci_canscan (pcitag_t tag)
{
	uint8_t bus, device, function;

	_pci_break_tag (tag, &bus, &device, &function); 
	if((bus == 0 ) && device == 0) {
		return(0);		/* Ignore the Discovery itself */
	}
	return (1);
}

void
pci_sync_cache(p, adr, size, rw)
	void *p;
	vm_offset_t adr;
	size_t size;
	int rw;
{
}
