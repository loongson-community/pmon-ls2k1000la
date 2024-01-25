#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <machine/cpu.h>

#include <pmon.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <flash.h>
#include <diskfs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/net/if.h>
#include "mod_vgacon.h"
#include "mod_display.h"
#include <pflash.h>

#define CONFIG_PAGE_SIZE_64KB
#include "target/ls2k1000.h"
#include "../arch/loongarch/loongarchregs.h"
#include "gzip.h"
#if NGZIP > 0
#include <gzipfs.h>
#endif /* NGZIP */
#include <errno.h>






static int gpio_test(int argc, char **argv)
{

    return 0;
}

static int gpio_mult(int argc, char **argv)
{
    unsigned int config0,config1,config2;
    config0 = readl(LS2K1000_GENERAL_CFG0);
    config1 = readl(LS2K1000_GENERAL_CFG1);
    config2 = readl(LS2K1000_GENERAL_CFG2);
    printf("config reg is 0x%x    0x%x    0x%x\n",config0, config1, config2);
    if (config0 & 0x8)
        printf("GMAC1 sel\n");
    else 
        printf("GMAC1 is gpio4-13\n");
    if (config0 & 0x10)
        printf("HDA sel\n");
    else if (config0 & 0x40)
        printf("I2S sel\n");
    else
        printf("HDA is gpio24-30\n");
    if (config2 & 0x2){
        if ((config0 & 0x80) || (config1&0x30))
            printf("DVO0 multi config conflict,please check it\n");
        printf("DVO0 sel\n");
    }else if (config0 & 0x80){
        printf("LIO sel\n");

    }else if(config1 & 0x30){
        printf("UART1/2 sel\n");
    }

        


    return 0;
}

static int pci_status(int argc, char **argv)
{
    unsigned int base0,base1,base2,base3,base4,base5;
    unsigned int status0,status1,status2,status3,status4,status5;
    base0 = readl(PHYS_TO_UNCACHED(0x1a004810));
    base1 = readl(PHYS_TO_UNCACHED(0x1a005010));
    base2 = readl(PHYS_TO_UNCACHED(0x1a005810));
    base3 = readl(PHYS_TO_UNCACHED(0x1a006010));
    base4 = readl(PHYS_TO_UNCACHED(0x1a006810));
    base5 = readl(PHYS_TO_UNCACHED(0x1a007010));
    status0 = readl(PHYS_TO_UNCACHED(base0 + 0x8));
    status1 = readl(PHYS_TO_UNCACHED(base1 + 0x8));
    status2 = readl(PHYS_TO_UNCACHED(base2 + 0x8));
    status3 = readl(PHYS_TO_UNCACHED(base3 + 0x8));
    status4 = readl(PHYS_TO_UNCACHED(base4 + 0x8));
    status5 = readl(PHYS_TO_UNCACHED(base5 + 0x8));
    printf("PCIE0 port0 LTSSM status :0x%x \n", (status0&0x3f));
    printf("PCIE0 port1 LTSSM status :0x%x \n", (status1&0x3f));
    printf("PCIE0 port2 LTSSM status :0x%x \n", (status2&0x3f));
    printf("PCIE0 port3 LTSSM status :0x%x \n", (status3&0x3f));
    printf("PCIE1 port0 LTSSM status :0x%x \n", (status4&0x3f));
    printf("PCIE1 port1 LTSSM status :0x%x \n", (status5&0x3f));

    return 0;
}
extern unsigned long long str2addr(const char *nptr, char **endptr, int base);
static int mem_test(int argc, char **argv)
{
    unsigned int *xs = str2addr(argv[1],0,0);
    unsigned int len = strtoul(argv[2],0,0);
    unsigned int c = strtoul(argv[3],0,0);
    printf("fsy:: add==%p,len=%x,val=%x\n",xs,len,c);
    while(len--){
        *xs++ = c;
        if ((len%0x200) == 0)
           printf("fsy:: length=%x xs==%p\n",len,xs);
    }

    return 0;
}

static int cputemp(int argc, char **argv)
{
    int temp;
    temp = readl(PHYS_TO_UNCACHED(0x1fe01514));
    temp &= 0xffff;
    temp -= 100;
    printf("NOW, CPU node temperature is %d °C \n",temp);
    return 0;
}

static int cpucfg(int argc, char **argv)
{
    int val;
    int reg = 0;
    asm volatile (
        "cpucfg %[val], %[reg] \n\t"
        : [val] "=r" (val)
        : [reg] "r" (reg)
        : "memory");

    printf("CPUCFG is %x  \n",val);
    return 0;
}

static int iocsr(int argc, char **argv)
{
    int val;
    int reg = 0;
    asm volatile (
        "iocsrrd.w %[val], %[reg] \n\t"
        : [val] "=r" (val)
        : [reg] "r" (reg)
        : "memory");

    printf("iocsr is %x  \n",val);
    return 0;
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"gpiotest", "[gpio mask] [0|1]", 0, "gpio test ", gpio_test, 0, 99, CMD_REPEAT},
	{"gpiomult", "[gpio mask] [0|1]", 0, "gpio mult ", gpio_mult, 0, 99, CMD_REPEAT},
	{"pcistatus", "[pci mask] [0|1]", 0, "pci status ", pci_status, 0, 99, CMD_REPEAT},
	{"memtest", "[gpio mask] [0|1]", 0, "gpio test ", mem_test, 0, 99, CMD_REPEAT},
	{"cputemp", "", 0, "get cpu temperature ", cputemp, 0, 99, CMD_REPEAT},
	{"cpucfg", "", 0, "get cpucfg reg ", cpucfg, 0, 99, CMD_REPEAT},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
