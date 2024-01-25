
void realinit_loongarch();
void init_loongarch(unsigned long long msize)
{
	unsigned long i;
	char * biosdata_flash = (unsigned long long)biosdata & 0xfffffULL | 0x1c000000ULL | (0x900000000f010000 & (0xffULL << 56));
	char * biosdata_mem = (0x900000000f010000 - sizeof(biosdata)) & ~0xffULL;

	early_printf("Copy Bios to memory ");	//Copy the biosdata from flash to the memory
	for (i = 0; i < sizeof(biosdata); i += sizeof(unsigned long)) {
		*(volatile unsigned long*)(biosdata_mem + i) = *(volatile unsigned long*)(biosdata_flash + i);
	}
	early_printf("OK, Uncompressing Bios");
	while(1) {
		if(run_unzip(biosdata_mem, 0x900000000f010000) >= 0)
			break;
	}
	memset((void *)0x900000000f2421a8, 0, 0x900000000f293670 - 0x900000000f2421a8);	//clear bss
	memset((void *)0x900000000f010000 - 0x1000, 0, 0x1000);	//0x900000000f010000-0x1000 for frame(registers),memset for pretty
	early_printf("OK, Booting Bios\r\n");
	realinit_loongarch(msize);
}


void realinit_loongarch(unsigned long long msize)
{
	__asm__ ("li.d  $r3,0x900000000f010000-0x4000;\n" \
		"	li.d $r12,0x900000000f0ac828;\n" \
		"	move $r4,%0;\n" \

		"	jirl $r0,$r12, 0;\n" \
		:
          	: "r" (msize)
		: "$r3", "$r12");
}
