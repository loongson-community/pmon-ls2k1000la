#if defined(LS3_HT) || defined(LS2G_HT)
#include <target/bonito.h>
#endif

#ifndef __LINUXIO_H_
#define __LINUXIO_H_
#if defined(FCRSOC)||defined(BONITOEL)

#if defined(LS3_HT)||defined(LS2G_HT)
#define loongarch_io_port_base BONITO_PCIIO_BASE_VA
#else
#define loongarch_io_port_base PHYS_TO_UNCACHED(0x1fe20000)
#endif

#else
#ifdef CONFIG_PCI0_GAINT_MEM
#define loongarch_io_port_base PHYS_TO_UNCACHED(0x1ea00000)
#else
#define loongarch_io_port_base PHYS_TO_UNCACHED(0x10100000)
#endif
#endif
#define __SLOW_DOWN_IO \
	*(volatile unsigned char *)loongarch_io_port_base = 0;

#define SLOW_DOWN_IO {__SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; \
		__SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO; __SLOW_DOWN_IO;  \
		__SLOW_DOWN_IO; }

static inline unsigned char linux_inb(unsigned long port)
{
        return (*(volatile unsigned char *)(loongarch_io_port_base + port));
}

static inline unsigned short linux_inw(unsigned long port)
{
        return (*(volatile unsigned short *)(loongarch_io_port_base + port));
}

static inline unsigned int linux_inl(unsigned long port)
{
        return (*(volatile unsigned int *)(loongarch_io_port_base + port));
}

#define linux_outb(val,port)\
do {\
	*(volatile unsigned char *)(loongarch_io_port_base + (port)) = (val);  \
} while(0)

#define linux_outw(val,port)							\
do {									\
	*(volatile unsigned short *)(loongarch_io_port_base + (port)) = (val);	\
} while(0)

#define linux_outl(val,port)							\
do {									\
	*(volatile unsigned int *)(loongarch_io_port_base + (port)) = (val);\
} while(0)

#define linux_outb_p(val,port)                                                \
do {                                                                    \
        *(volatile unsigned char *)(loongarch_io_port_base + (port)) = (val);           \
        SLOW_DOWN_IO;                                                   \
} while(0)

static inline unsigned char linux_inb_p(unsigned long port)
{
	unsigned char __val;

        __val = *(volatile unsigned char *)(loongarch_io_port_base + port);
        SLOW_DOWN_IO;

        return __val;
}

#endif /* __LINUXIO_H_ */
