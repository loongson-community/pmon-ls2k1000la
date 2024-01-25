 /*
  * This file is for SLT for 2K1000 
  *
  */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/device.h>
#include <sys/queue.h>

#include <pmon.h>

#define u64 unsigned long long
#define u32 unsigned int

#define __raw__readw(addr)     (*(volatile unsigned short *)(addr))
#define __raw__writew(addr,w)  ((*(volatile unsigned short *)(addr)) = (w))

void slt_test(void)
{
	unsigned int port;
	unsigned int test_mode;
	unsigned int port_num;
	unsigned int dev_num;
	unsigned long long header;
	unsigned long long bar0;
	unsigned int bar0_low;
	unsigned int bar0_high;
	unsigned int status;
	unsigned int i;


	for(port_num = 0; port_num < 3; port_num++)
	{	
		dev_num = port_num  == 0 ? 0xb :
		          port_num  == 1 ? 0xc :
		          port_num  == 2 ? 0xd :
		                           0xb;
	
		header = 0x900000fe00000000ULL | (dev_num << 11);
	
		bar0_low  = (__raw__readw(header + 0x10) & 0xffffffff0);
		bar0_high = (__raw__readw(header + 0x14));
		bar0 = (bar0_high << 32 | bar0_low) + 0x9000000000000000ULL;

		if(port_num == 2){	
			test_mode = 4;
			__raw__writew(bar0, 0xff204f);
		}else{
			test_mode = 1;
			__raw__writew(bar0, 0xff204c);
		}

	
		status = __raw__readw(bar0 + 0xc);
		i = 0;
		while((status & 0x7f) != 0x51){
			printf("wait dev 0x%x link up,status %x\n",dev_num,status);
			status = __raw__readw(bar0 + 0xc);
			if(i == 3){
				printf("!error: status %x ,dev 0x%x is not link up\n",status,dev_num);
				while(1);
			}
			i++;
		}
		printf("dev 0x%x link up\n",dev_num);
	
		status = __raw__readw(header + 0x80);
		if(((status >> 16) & 0xf) == 0x2){
			printf("dev 0x%x speed is 5GT\n",dev_num);
		}else{
			printf("!error: status %x ,dev 0x%x speed is not 5GT\n",status,dev_num);
			while(1);
		}
		if(((status >> 20) & 0x3f) == test_mode){
			printf("dev 0x%x width is X%d\n",dev_num,test_mode);
		}else{
			printf("!error: status %x ,dev 0x%x link width is not X%d\n",status,dev_num,test_mode);
			while(1);
		}
	}
	return(0);
}
unsigned char
cmd_slttest(ac, av)
	int ac;
	char *av[];
{
	unsigned int port;
	unsigned int test_mode;
	unsigned int port_num;
	unsigned int dev_num;
	unsigned long long header;
	unsigned long long bar0;
	unsigned int bar0_low;
	unsigned int bar0_high;
	unsigned int status;
	unsigned int i;

	if (ac != 3){
		printf("usage: pcietest <port num> [test mode for how many lines?]\n");
		printf("port num: 0 -> f0 port0 x1/x4\n");
		printf("port num: 1 -> f0 port1 x1\n");
		printf("port num: 2 -> f0 port2 x1\n");
		printf("port num: 3 -> f0 port3 x1\n");
		printf("port num: 4 -> f1 port0 x1/x4\n");
		printf("port num: 5 -> f1 port1 x1\n");
		printf("test_mode: 1 ->0x1, 1 lines\n");
		printf("test_mode: 4 ->0x4, 4 lines\n");
		printf("For example:slttest 0 1 \n");
		printf("For example:slttest 4 4 \n");
		return 0;
	}

	port_num = (unsigned int)atoi(av[1]);
	printf("pcie port = %d\n",port_num);
	if(port_num > 5) {
		printf("pcie port num error\n");
		return 0;
	}

	dev_num = port_num  == 0 ? 0x9 :
	          port_num  == 1 ? 0xa :
	          port_num  == 2 ? 0xb :
	          port_num  == 3 ? 0xc :
	          port_num  == 4 ? 0xd :
	          port_num  == 5 ? 0xe :
	                           0x9;

	header = 0x900000fe00000000ULL | (dev_num << 11);

	bar0_low  = (__raw__readw(header + 0x10) & 0xffffffff0);
	bar0_high = (__raw__readw(header + 0x14));
	bar0 = (bar0_high << 32 | bar0_low) + 0x9000000000000000ULL;
	//printf("pcie header = 0x%llx\n",header);
	//printf("pcie bar0_low = 0x%x\n",bar0_low);
	//printf("pcie bar0_high = 0x%x\n",bar0_high);
	//printf("pcie bar0 = 0x%llx\n",bar0);
	
	test_mode = (unsigned int)atoi(av[2]);
	if(test_mode == 1){
		__raw__writew(bar0, 0xff204c);
	}else{
		__raw__writew(bar0, 0xff204f);
	}

	status = __raw__readw(bar0 + 0xc);
	i = 0;
	while((status & 0x7f) != 0x51){
		printf("wait dev 0x%x link up,status %x\n",dev_num,status);
		status = __raw__readw(bar0 + 0xc);
		if(i == 3){
			printf("!error: status %x ,dev 0x%x is not link up\n",status,dev_num);
			return(0);
		}
		i++;
	}
	printf("dev 0x%x link up\n",dev_num);

	status = __raw__readw(header + 0x80);
	if(((status >> 16) & 0xf) == 0x2){
		printf("dev 0x%x speed is 5GT\n",dev_num);
	}else{
		printf("!error: status %x ,dev 0x%x speed is not 5GT\n",status,dev_num);
		return(0);
	}
	if(((status >> 20) & 0x3f) == test_mode){
		printf("dev 0x%x width is X%d\n",dev_num,test_mode);
	}else{
		printf("!error: status %x ,dev 0x%x link width is not X%d\n",status,dev_num,test_mode);
		return(0);
	}
	return(0);
}

/*
 *
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Misc"},
	{"slttest", "", 0, "LS2K pcie link status test: slttest ", cmd_slttest, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
