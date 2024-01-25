#ifndef __LOONGSON3_DEF_H__
#define __LOONGSON3_DEF_H__

#define DEBUG_LOCORE

/* if change the lock cache base addrress should change Makefile.inc GZROMSTARTADDR */
#define LOCK_CACHE_BASE	PHYS_TO_CACHED(0x90000000)
#define LOCK_CACHE_SIZE	0x400000

#ifdef DEBUG_LOCORE
#define TTYDBG(x) \
	.section .rodata;98: .asciz x; .text; la a0, 98b; bl stringserial
#define TTYDBG_COM1(x) \
	.section .rodata;98: .asciz x; .text; la a0, 98b; bl stringserial_COM1
#else
#define TTYDBG(x)
#define TTYDBG_COM1(x)
#endif
#define  PRINTSTR TTYDBG

#ifdef DEVBD2F_SM502
#define GPIOLED_DIR  0xe
#else
#define GPIOLED_DIR  0xf
#endif

#undef USE_GPIO_SERIAL
#ifndef USE_GPIO_SERIAL
#else
#define GPIOLED_SET(x)
#endif

/*1 sel GPIO function,otherwise sel HT function*/
#if	(TOT_NODE_NUM >= 2)
#ifdef LS7A_2WAY_CONNECT
	#define HT0_LO_DISABLE_UNUSED	(0 << 1)	//HT0_LO_RSTn
	#define HT0_HI_DISABLE_UNUSED	(0 << 5)	//HT0_HI_RSTn
	#define HT1_LO_DISABLE_UNUSED	(0 << 9)	//HT1_LO_RSTn
	#define HT1_HI_DISABLE_UNUSED	(1 << 13)	//HT1_HI_RSTn
#else
	#define HT0_LO_DISABLE_UNUSED	(0 << 1)
	#define HT0_HI_DISABLE_UNUSED	(0 << 5)
	#define HT1_LO_DISABLE_UNUSED	(0 << 9)
	#define HT1_HI_DISABLE_UNUSED	(0 << 13)
#endif
#else
	#define HT0_LO_DISABLE_UNUSED	(1 << 1)
	#define HT0_HI_DISABLE_UNUSED	(0 << 5)
	#define HT1_LO_DISABLE_UNUSED	(0 << 9)
	#define HT1_HI_DISABLE_UNUSED	(0 << 13)
#endif

#define DISABLE_UNUSED_HT_PIN	(HT0_LO_DISABLE_UNUSED | HT0_HI_DISABLE_UNUSED | HT1_LO_DISABLE_UNUSED | HT1_HI_DISABLE_UNUSED)

/******************************************************
 *used: t0~t2
 ******************************************************/
#define SET_GPIO_FUNC_EN(x)			\
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00500);	\
    slli.d  t2, s2, NODE_OFFSET;			\
    or      t0, t0, t2;					\
    ld.w    t1, t0, 0x4;				\
    li.w	t2, x;					\
    or      t1, t1, t2;					\
    st.w    t1, t0, 0x4;

/*Finish it as soon as possible.*/
#define UNUESED_HT_PIN_TO_GPIO(x) \
	li.d	t0, PHYS_TO_UNCACHED(0x1fe00500); \
	li.w	t1, x ; \
	st.h	t1, t0, 0;

/******************************************************
 *used: t0~t2
 ******************************************************/
#define STABLE_COUNTER_CLK_EN				\
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00420);		\
    slli.d  t2, s2, NODE_OFFSET;			\
    or      t0, t0, t2;					\
    ld.w    t1, t0, 0x4;				\
    li.w	t2, (1 << 15);				\
    or      t1, t1, t2;	/*clken_stable*/		\
    st.w    t1, t0, 0x4;

#define SRAM_CTRL_VALUE_3 0x5000180158081
#define SRAM_CTRL_VALUE_2 0x5000180150041
#define SRAM_CTRL_VALUE_1 0x6004000180110040
#define SRAM_CTRL_VALUE_0 0x100010014

#define LS3A5000_SRAM_CTRL				\
    bl      chip_ver;					\
    li.d	a2, 0x41;					\
    li.d	t1, SRAM_CTRL_VALUE_0;			\
    beq     v0, a2, 1f;					\
    li.d	a2, 0x42;					\
    li.d	t1, SRAM_CTRL_VALUE_1;			\
    beq     v0, a2, 1f;					\
    li.d	a2, 0xf;					\
    li.d	t1, SRAM_CTRL_VALUE_2;			\
    beq     v1, a2, 1f;					\
    li.d	t1, SRAM_CTRL_VALUE_3;			\
1:							\
    li.d	a0, TOT_NODE_NUM;				\
    move    a1, zero;					\
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00430); 		\
2:							\
    slli.d  t2, a1, NODE_OFFSET;			\
    or      t2, t0, t2;					\
    st.d    t1, t2, 0x0;				\
    addi.w  a1, a1, 1;					\
    blt     a1, a0, 2b;

#define SET_NODEMASK(value)				\
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00400);		\
    slli.d  t2, s2, NODE_OFFSET;			\
    or      t0, t0, t2;					\
    ld.d    t2, t0, 0x0;				\
    li.d	t1, ~(0xf << 4);				\
    and     t2, t1, t2;					\
    li.d	t1, (value << 4);				\
    or      t2, t2, t1;					\
    st.d    t2, t0, 0x0;

#define ENABLE_XLINK(node)				\
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00400) | (node << NODE_OFFSET);	\
    ld.d    t2, t0, 0x0;				\
    li.d	t1, (0x1 << 8);				\
    or      t2, t2, t1;					\
    st.d    t2, t0, 0x0;

#define SET_HT_REG_DISABLE(node, value)			\
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00400) | (node << NODE_OFFSET);	\
    ld.d    t2, t0, 0x0;				\
    li.d	t1, ~(0xf << NODE_OFFSET);			\
    and     t2, t1, t2;					\
    li.d	t1, (value << NODE_OFFSET);			\
    or      t2, t2, t1;					\
    st.d    t2, t0, 0x0;

/* set GPIO as output
 * x : 0x1 << offset
 */
/******************************************************
 *used: t0~t2
 ******************************************************/
#define GPIO_SET_OUTPUT(x)			\
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00500);		\
    ld.w    t1, t0, 0xc;			\
    li.w	t2, x;				\
    or      t1, t1, t2;				\
    st.w    t1, t0, 0x8;			\
    ld.w    t1, t0, 0x0;			\
    or      t1, t1, t2;				\
    xor     t1, t1, t2;				\
    st.w    t1, t0, 0x0;			\
    ld.w    t1, t0, 0x4;			\
    or      t1, t1, t2;				\
    xor     t1, t1, t2;				\
    st.w    t1, t0, 0x4;


/* clear GPIO as output
 *  * x : 0x1 <<offsest
 *	*/
#define GPIO_CLEAR_OUTPUT(x)			\
    li.d	a0, PHYS_TO_UNCACHED(0x1fe00500);		\
    ld.w    a1, a0, 0xc;			\
    li.w	a2, x;				\
    or      a1, a1, a2;				\
    xor     a1, a1, a2;				\
    st.w    a1, a0, 0x8;			\
    ld.w    a1, a0, 0x0;			\
    or      a1, a1, a2;				\
    xor     a1, a1, a2;				\
    st.w    a1, a0, 0x0;			\
    ld.w    a1, a0, 0x4;			\
    or      a1, a1, a2;				\
    xor     a1, a1, a2;				\
    st.w    a1, a0, 0x4;

#if	(TOT_NODE_NUM == 1)
/* WatchDog Close for chip MAX6369*/
#define WATCHDOG_CLOSE \
	GPIO_CLEAR_OUTPUT(0x1<<5); \
	GPIO_SET_OUTPUT(0x1<<6|0x1<<4); \
	GPIO_CLEAR_OUTPUT(0x1<<13); \

/* WatchDog Enable for chip MAX6369*/
#define WATCHDOG_ENABLE \
	GPIO_CLEAR_OUTPUT(0x1<<13); \
	GPIO_SET_OUTPUT(0x1<<14); \
	GPIO_SET_OUTPUT(0x1<<5); \
	GPIO_SET_OUTPUT(0x1<<4); \
	GPIO_CLEAR_OUTPUT(0x1<<6); \
	GPIO_CLEAR_OUTPUT(0x1<<14); \
	li.w	a1,0x100;\
	78:; \
	addi.d a1,a1,-1 /*subu v1,1*/; \
	bnez   a1, 78b; \
	GPIO_SET_OUTPUT(0x1 << 13);
#else  //4 way watchdog uesed gpio was changed
/* WatchDog Close for chip MAX6369*/
#define WATCHDOG_CLOSE \
	GPIO_CLEAR_OUTPUT(0x1<<5); \
	GPIO_SET_OUTPUT(0x1<<6|0x1<<4); \
	GPIO_CLEAR_OUTPUT(0x1<<6); \

/* WatchDog Enable for chip MAX6369*/
#define WATCHDOG_ENABLE \
	GPIO_CLEAR_OUTPUT(0x1<<6); \
	GPIO_SET_OUTPUT(0x1<<14); \
	GPIO_SET_OUTPUT(0x1<<5); \
	GPIO_CLEAR_OUTPUT(0x1<<4); \
	GPIO_SET_OUTPUT(0x1<<6); \
	GPIO_CLEAR_OUTPUT(0x1<<14); \
	li.w	a1,0x100;\
	78:; \
	li.w	a0, 0x1;		\
	sub.w   a1, a1, a0;	     \
	bnez    a1, 78b;		\
	GPIO_SET_OUTPUT(0x1 << 6);
#endif

#define w83627write(x,y,z) \
	li.d	a0, PHYS_TO_UNCACHED(0x1800002e); \
	li.d	a1, 0x87; \
	st.b    a1, a0, 0; \
	st.b    a1, a0, 0; \
	li.d	a1, 0x7; \
	st.b    a1, a0, 0; \
	li.d	a1, x; \
	st.b    a1, a0, 1; \
	li.d	a1, y; \
	st.b    a1, a0, 0; \
	li.d	a1, z; \
	st.b    a1, a0, 1; \
	li.d	a1, 0xaa; \
	st.b    a1, a0, 0; \
	st.b    a1, a0, 0;

#define CONFIG_CACHE_64K_4WAY 1

#define tmpsize		s1
#define msize		s2
#define bonito		s4
#define dbg		s5
#define sdCfg		s6

#define	NODE0_CORE0_BUF0	PHYS_TO_UNCACHED(0x1fe01000)
#define	NODE1_CORE0_BUF0	PHYS_TO_UNCACHED(0x10001fe01000)
#define	NODE2_CORE0_BUF0	PHYS_TO_UNCACHED(0x20001fe01000)
#define	NODE3_CORE0_BUF0	PHYS_TO_UNCACHED(0x30001fe01000)

#define	HT_REMOTE_NODE		PHYS_TO_UNCACHED(0x10001fe01000)

#define LS3A5000_I2C1_REG_BASE  PHYS_TO_UNCACHED(0x1fe00130)
#define LS3A5000_I2C0_REG_BASE  PHYS_TO_UNCACHED(0x1fe00120)


#define FN_OFF			0x020
#define SP_OFF			0x028
#define GP_OFF			0x030
#define A1_OFF			0x038


#define L2_CACHE_OK		0x1111
#define L2_CACHE_DONE		0x2222
#define TEST_HT			0x3333
#define NODE_MEM_INIT_DONE	0x4444
#define ALL_CORE0_INIT_DONE	0x5555
#define NODE_SCACHE_ENABLED	0x6666
#define SYSTEM_INIT_OK		0x5a5a

#define PRINT_CSR(offset)	\
	PRINTSTR("\r\ncsr 0x");	\
	li.w	a0, offset;	\
	bl 	hexserial;	\
	PRINTSTR(" ->0x");	\
	csrrd	a0, offset;	\
	bl	hexserial64;	\
	PRINTSTR("\r\n");

#define VOLTAGE_CTRL

#endif
