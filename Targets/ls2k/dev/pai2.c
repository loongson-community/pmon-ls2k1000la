void pai2_init(void)
{
#ifdef SATA_TX_REVERT
	*(volatile int *)0x800000001fe00450 |= (1<<9);
#endif

#ifdef SEL_HDA
	*(volatile int *)0x800000001fe00420 &= ~(7<<4);
	*(volatile int *)0x800000001fe00420 |= (1<<4);
#endif

#ifdef LCD_EN
	*(volatile int *)0x800000001fe00500 &= ~(1<<3);
	*(volatile int *)0x800000001fe00510 |= (1<<3);


//enalbe pwm 0
	*(volatile int *)0x800000001fe00420 |=(1<<12);
	*(volatile int *)0x800000001fe22004 = 150;
	*(volatile int *)0x800000001fe22008 = 1500;
	*(volatile int *)0x800000001fe2200c = 1;
#endif


// set touchscreen type of irq
	{
		unsigned int val;
		val =*(volatile int *)0x800000001fe01470;
		printf("470=0x%x\n", val);
		val =*(volatile int *)0x800000001fe01474;
		printf("474=0x%x\n", val);

		*(volatile int *)0x800000001fe01470 &= ~(1<<29);	
		*(volatile int *)0x800000001fe01474 |= (1<<29);	
		printf("after write\n");
		val =*(volatile int *)0x800000001fe01470;
		printf("470=0x%x\n", val);
		val =*(volatile int *)0x800000001fe01474;
		printf("474=0x%x\n", val);
	
	}
}
