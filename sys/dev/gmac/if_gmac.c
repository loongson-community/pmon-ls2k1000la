#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#if defined(LOONGSON_2K)
#include <autoconf.h>
#endif


#include "synopGMAC_plat.h"

static int syn_match(parent, match, aux)
	struct device *parent;
#if defined(__BROKEN_INDIRECT_CONFIG) || defined(__OpenBSD__)
	void *match;
#else
	struct cfdata *match;
#endif
	void *aux;
{
	return 1;
}

s32  synopGMAC_init_network_interface(char* xname, u64 synopGMACMappedAddr);

static void syn_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct device *sc = self;
	struct confargs *cf = (struct confargs *)aux;
	synopGMAC_init_network_interface(sc->dv_xname, CACHED_TO_UNCACHED(cf->ca_baseaddr));
}


struct cfattach syn_ca = {
	sizeof(struct device), syn_match, syn_attach
};

struct cfdriver syn_cd = {
	NULL, "syn", DV_IFNET
};


#if defined(LOONGSON_2K) || defined (LS7A)
#define PCI_VENDOR_2KGMAC 0x0014
#define PCI_PRODUCT_2KGMAC 0x7a03
#define PCI_VENDOR_7AGMAC 0x0014
#define PCI_PRODUCT_7AGMAC 0x7a13
/* product changed need add new macro */

static int pcisyn_match(parent, match, aux)
	struct device *parent;
#if defined(__BROKEN_INDIRECT_CONFIG) || defined(__OpenBSD__)
	void *match;
#else
	struct cfdata *match;
#endif
	void *aux;
{
	struct pci_attach_args *pa = aux;

	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_2KGMAC && ((PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_2KGMAC) ||
					(PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_7AGMAC)))
		return 1;
	return 0;
}


static void pcisyn_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct device *sc = self;
	struct pci_attach_args *pa = aux;
	struct pci_chipset_tag_t *pc = pa->pa_pc;
	bus_space_tag_t memt = pa->pa_memt;
	bus_addr_t membasep;
	bus_size_t memsizep;
	int i;
	u32 linkmap;

	if (pci_mem_find(NULL, pa->pa_tag, 0x10, &membasep, &memsizep, NULL)) {
		printf(" Can't find mem space\n");
		return;
	}

	printf("membasep=0x%x\n", (int)membasep);

	synopGMAC_init_network_interface(sc->dv_xname, PHYS_TO_UNCACHED(membasep));
}


struct cfattach pcisyn_ca = {
	sizeof(struct device), pcisyn_match, pcisyn_attach
};

struct cfdriver pcisyn_cd = {
	NULL, "syn", DV_IFNET
};


#endif
