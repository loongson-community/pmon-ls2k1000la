/*
 * loongson3_clksetting.S
 * change the PLL settings of each core
 * NOTE:
 * Using S1 for passing the NODE ID
 */
#ifdef SOFT_CLKSEL

#define CORE_STEP       25

#define USE_LS_PLL      1

#ifdef BONITO_100M
#define SYS_CLOCK       100     //MUST BE 100 or 25, depend on the osillator
#elif BONITO_25M
#define SYS_CLOCK       25      //MUST BE 100 or 25, depend on the osillator
#SYS_CLOCK IS NOT GOOD FOR HIGH FREQUENCY
#endif

// L1_* define core frequency
#define LS_PLL		USE_LS_PLL
#define ST_PLL		(1 - USE_LS_PLL)

//PLL Constraint
//Refc = 25/50/100MHz, 50MHz is more prefered
//VCO = 4.8GHz - 6.4GHz
//LOOPC < 255
//DIV = 1/2/4/6/>6, 3/5 is NOT prefered

#define L1_REFC		2 //SYS_CLOCK/25
//#define L1_REFC		SYS_CLOCK/25
#if (CORE_FREQ >= 2200)
#define L1_DIV          2
#elif (CORE_FREQ >= 1600)
#define L1_DIV          3
#else
#define L1_DIV		4
#endif
#define L1_LOOPC	(L1_REFC*CORE_FREQ*L1_DIV/SYS_CLOCK)

#define VDDA_CTRL       0x3
#define VDDD_CTRL       0x3
#define VDDA_EN         0x1
#define VDDD_EN         0x1

#define SYS_PD			(((1 - ST_PLL) << 1) | (1 - LS_PLL))
#define PLL_L1_LOCKED 		((ST_PLL << 17) | (LS_PLL << 16))
#define PLL_CHANG_COMMIT 	0x1

#define CORE_CLKSEL		0xc0
#define CORE_HSEL		0x80
#define PLL_L1_ENA		(0x1 << 2)

#if (TOT_NODE_NUM >= 2)
    TTYDBG ("Change the scale of HT0 clock\r\n")
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00180)
    or      t0, t0, s1
    ld.w    a0, t0, 0x0
    li.w	a1, 0xf0ffffff
    and     a0, a0, a1
    li.w	a1, 0x0b000000 // 4/8 of node clock
    or      a0, a0, a1
    st.w    a0, t0, 0x0
#else
    TTYDBG ("Disable HT0 clock\r\n")
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00180)
    or      t0, t0, s1
    ld.w    a0, t0, 0x0
    li.w	a1, 0xf0ffffff
    and     a0, a0, a1
    li.w	a1, 0x00000000 // Disable
    or      a0, a0, a1
    st.w    a0, t0, 0x0
#endif

    TTYDBG ("Change the scale of HT1 clock\r\n")
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00180)
    or      t0, t0, s1
    ld.w    a0, t0, 0x0
    li.w	a1, 0x0fffffff
    and     a0, a0, a1
    li.w	a1, 0xb0000000 // 4/8 of node clock
    or      a0, a0, a1
    st.w    a0, t0, 0x0

    TTYDBG ("Change the scale of LS132 clock\r\n")
    li.d	t0, PHYS_TO_UNCACHED(0x1fe00420)
    or      t0, t0, s1
    ld.w    a0, t0, 0x0
    li.w	a1, 0xfff0ffff
    and     a0, a0, a1
    li.w	a1, 0x00080000 // 1/8 of node clock
    or      a0, a0, a1
    st.w    a0, t0, 0x0


    TTYDBG ("Soft CLK SEL adjust begin\r\n")

    li.d	t0, PHYS_TO_UNCACHED(0x1fe00194)
    or      t0, t0, s1
    ld.w    a0, t0, 0x0
    li.w	a1, CORE_CLKSEL
    and     a0, a0, a1
    li.w	a1, CORE_HSEL
    bne     a0, a1, 30f //soft_ht


//soft_sys:
    TTYDBG ("CORE & NODE:")
/* UEFI can not using this code */
    li.d	t0, PHYS_TO_UNCACHED(0x1fe001b0)
    or      t0, t0, s1
    li.w	t1, (0x7 << 19) //power down pll L1 first
    st.d    t1, t0, 0x0
    li.d	t1, (L1_DIV << 0) | (VDDD_CTRL << 60) | (VDDA_CTRL << 56)
    st.d    t1, t0, 0x8
    li.d	t1, (0x7 << 19) | (VDDD_EN << 9) | (VDDA_EN << 8) 	//power down pll L1 first
    st.d    t1, t0, 0x0
    li.d	t1, (L1_LOOPC << 54) | (L1_REFC << 48) | \
                 (L1_DIV << 42) | (L1_LOOPC << 32) | (L1_REFC << 26) | \
                 (SYS_PD << 19) | (ST_PLL << 22) | (0x3 << 10) | (VDDD_EN << 9) | (VDDA_EN << 8) | (0x1 << 7)

    st.d    t1, t0, 0x0
    ori     t1, t1, PLL_L1_ENA
    st.d    t1, t0, 0x0

11: //wait_locked_sys:
    ld.d    a0, t0, 0x0
    li.d	a1, PLL_L1_LOCKED
    and     a0, a1, a0
    beqz    a0, 11b //wait_locked_sys

    ld.d    a0, t0, 0x0
    ori     a0, a0, PLL_CHANG_COMMIT
    st.d    a0, t0, 0x0

    bl     hexserial


30: //soft_ht:
    TTYDBG ("\r\nHT :")

    li.d	t0, PHYS_TO_UNCACHED(0x1fe001b0)
    or      t0, t0, s1
    ld.w    a0, t0, 0x14
    bl     hexserial
#endif
