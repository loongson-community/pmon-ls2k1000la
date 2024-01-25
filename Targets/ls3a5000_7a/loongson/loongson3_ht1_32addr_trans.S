/*
 * need configure slave device id 0 1 2 3 a b f
 * 0x1e000000 -> 0xe0000000000
 */
    li.d	t0, PHYS_TO_UNCACHED(0x1fe02000)
    li.w	t1, 0xc00
    add.d   t2, t0, t1
1:
 //map HT: PCI IO : 0xe00_00000000 <-- 0x1e000000
    li.d	t1, 0x000000001e000000
    st.d    t1, t0, 0x18
    li.d	t1, 0xffffffffff000000
    st.d    t1, t0, 0x58
    li.d	t1, 0x00000e000000008e
    st.d    t1, t0, 0x98

    li.d	t1, 0x0000000020000000;
    st.d    t1, t0,0x20;
    li.d	t1, 0xffffffffe0000000;
    st.d    t1, t0,0x60;
    li.d	t1, 0x00000e002000008e;
    st.d    t1, t0,0xa0;

    addi.d  t0, t0, 0x100
    li.d	t3, PHYS_TO_UNCACHED(0x1fe02400)
    bne     t0, t3, 2f

    addi.d  t0, t0, 0x600
2:
    bne     t0, t2, 1b

    li.d	t0, PHYS_TO_UNCACHED(0x1fe02000)
    li.d	t1, 0x000000001e000000
    stptr.d    t1, t0, 0xf18
    li.d	t1, 0xffffffffff000000
    stptr.d    t1, t0, 0xf58
    li.d	t1, 0x00000e000000008e
    stptr.d    t1, t0, 0xf98

#if	(TOT_NODE_NUM == 8) && !defined(LS7A_2WAY_CONNECT)
    /*node 0 HT1_Lo win*/
    li.d	t0, PHYS_TO_UNCACHED(0x1fe02e00)

    li.d	t1, 0x0000000000000000
    st.d    t1, t0, 0x20
    li.d	t1, 0x00003f6000000000
    st.d    t1, t0, 0x60
    li.d	t1, 0x00000000000000f0
    st.d    t1, t0, 0xa0

    li.d	t1, 0x0000002000000000
    st.d    t1, t0, 0x28
    li.d	t1, 0x00003f6000000000
    st.d    t1, t0, 0x68
    li.d	t1, 0x00001000000000f0
    st.d    t1, t0, 0xa8

    li.d	t1, 0x0000004000000000
    st.d    t1, t0, 0x30
    li.d	t1, 0x00003f6000000000
    st.d    t1, t0, 0x70
    li.d	t1, 0x00002000000000f0
    st.d    t1, t0, 0xb0

    li.d	t1, 0x0000006000000000
    st.d    t1, t0, 0x38
    li.d	t1, 0x00003f6000000000
    st.d    t1, t0, 0x78
    li.d	t1, 0x00003000000000f0
    st.d    t1, t0, 0xb8
#if (TOT_7A_NUM == 2)
    /*node 5 HT1_Lo win*/
    li.d	t0, PHYS_TO_UNCACHED(0x50001fe02e00)

    li.d	t1, 0x0000000000000000
    st.d    t1, t0, 0x20
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x60
    li.d	t1, 0x00000000000000f0
    st.d    t1, t0, 0xa0

    li.d	t1, 0x0000001000000000
    st.d    t1, t0, 0x28
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x68
    li.d	t1, 0x00001000000000f0
    st.d    t1, t0, 0xa8

    li.d	t1, 0x0000002000000000
    st.d    t1, t0, 0x30
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x70
    li.d	t1, 0x00002000000000f0
    st.d    t1, t0, 0xb0

    li.d	t1, 0x0000003000000000
    st.d    t1, t0, 0x38
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x78
    li.d	t1, 0x00003000000000f0
    st.d    t1, t0, 0xb8
#endif
#endif
#if	(TOT_NODE_NUM == 16)
    /*node 0 and node 10 HT1_Lo win*/
    li.d	t0, PHYS_TO_UNCACHED(0x1fe02e00)
    li.d	t1, 0x0000a00000000000
    add.d   t2, t0, t1
1:
    li.d	t1, 0x0000000000000000
    st.d    t1, t0, 0x20
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x60
    li.d	t1, 0x00000000000000f0
    st.d    t1, t0, 0xa0

    li.d	t1, 0x0000001000000000
    st.d    t1, t0, 0x28
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x68
    li.d	t1, 0x00001000000000f0
    st.d    t1, t0, 0xa8

    li.d	t1, 0x0000002000000000
    st.d    t1, t0, 0x30
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x70
    li.d	t1, 0x00002000000000f0
    st.d    t1, t0, 0xb0

    li.d	t1, 0x0000003000000000
    st.d    t1, t0, 0x38
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x78
    li.d	t1, 0x00003000000000f0
    st.d    t1, t0, 0xb8

#ifdef	LS7A_2WAY_CONNECT
    li.d	t1, 0x0000a00000000000
    add.d   t0, t0, t1
    beq     t0, t2, 1b
#endif
#if (TOT_7A_NUM == 2)
    /*node 5 and node 15 HT1_Lo win*/
    li.d	t0, PHYS_TO_UNCACHED(0x50001fe02e00)
    li.d	t1, 0x0000a00000000000
    add.d   t2, t0, t1
1:
    li.d	t1, 0x0000000000000000
    st.d    t1, t0, 0x20
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x60
    li.d	t1, 0x00000000000000f0
    st.d    t1, t0, 0xa0

    li.d	t1, 0x0000001000000000
    st.d    t1, t0, 0x28
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x68
    li.d	t1, 0x00001000000000f0
    st.d    t1, t0, 0xa8

    li.d	t1, 0x0000002000000000
    st.d    t1, t0, 0x30
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x70
    li.d	t1, 0x00002000000000f0
    st.d    t1, t0, 0xb0

    li.d	t1, 0x0000003000000000
    st.d    t1, t0, 0x38
    li.d	t1, 0x00003f3000000000
    st.d    t1, t0, 0x78
    li.d	t1, 0x00003000000000f0
    st.d    t1, t0, 0xb8

#ifdef	LS7A_2WAY_CONNECT
    li.d	t1, 0x0000a00000000000
    add.d   t0, t0, t1
    beq     t0, t2, 1b
#endif
#endif
#endif

/*
 * need configure slave device id 0 1 2 3 a b c d e f
 */
 //map HT: PCI IO :  0xefd_fc000000 <-- 0x18000000
 //map               0xefd_fd000000 <-- 0x19000000
 //map HT: PCI CFG:  0xefd_fe000000 <-- 0x1a000000
 //map HT: PCI CFG:  0xefd_ff000000 <-- 0x1b000000
 //map HT: PCI MEM:  0xe00_10000000 <-- 0x10000000 ~ 0x17ffffff
 //map HT: PCI MEM:  0xe00_40000000 <-- 0x40000000 ~ 0x7fffffff
#define SET_HT_ADDR_TRANS(node, destnode, destport) \
    li.d	t0, PHYS_TO_UNCACHED(0x1fe02000) | node<<44; \
    li.w	t4, 0x1000;   \
    add.d   t2, t0, t4; \
    \
1:  \
    li.d	t1, 0x0000000018000000; \
    st.d    t1, t0,0x0; \
    li.d	t1, 0xfffffffffc000000; \
    st.d    t1, t0, 0x40; \
    li.d	t1, 0x00000efdfc000080 | destnode<<44 | destport; \
    st.d    t1, t0,0x80; \
    \
    li.d	t1, 0x0000000010000000; \
    st.d    t1, t0,0x8; \
    li.d	t1, 0xfffffffff8000000; \
    st.d    t1, t0,0x48; \
    li.d	t1, 0x00000e0010000080 | destnode<<44 | destport; \
    st.d    t1, t0,0x88; \
    \
    li.d	t1, 0x0000000040000000; \
    st.d    t1, t0,0x10; \
    li.d	t1, 0xffffffffc0000000; \
    st.d    t1, t0,0x50; \
    li.d	t1, 0x00000e0040000080 | destnode<<44 | destport; \
    st.d    t1, t0,0x90; \
    \
    li.d	t1, 0x00000e0000000000; \
    st.d    t1, t0,0x18; \
    li.d	t1, 0xffffff0000000000; \
    st.d    t1, t0,0x58; \
    li.d	t1, 0x00000e0000000080 | destnode<<44 | destport; \
    st.d    t1, t0,0x98; \
    \
    addi.d  t0, t0, 0x100; \
    li.d	t3, PHYS_TO_UNCACHED(0x1fe02400) | node<<44; \
    bne     t0, t3, 2f; \
    addi.d  t0, t0, 0x600; \
2: \
    bne     t0, t2, 1b;

#if	(TOT_NODE_NUM == 2)
#ifdef  LS7A_2WAY_CONNECT
    SET_HT_ADDR_TRANS(1, 1, 0xe);
#else
    SET_HT_ADDR_TRANS(1, 0, 0xa);
#endif

#elif	(TOT_NODE_NUM == 4)
    SET_HT_ADDR_TRANS(2, 0, 0xa);
#ifdef  LS7A_2WAY_CONNECT
    SET_HT_ADDR_TRANS(1, 1, 0xe);
    SET_HT_ADDR_TRANS(3, 1, 0xb);
#else
    SET_HT_ADDR_TRANS(1, 0, 0xb);
#ifdef DISABLE_X_LINK
    SET_HT_ADDR_TRANS(3, 0, 0xa);
#else
    SET_HT_ADDR_TRANS(3, 0, 0xf);
#endif
#endif

/*8 ways configured according to 2-way connect*/
#elif	(TOT_NODE_NUM == 8)
    SET_HT_ADDR_TRANS(1, 0, 0xb);
    SET_HT_ADDR_TRANS(2, 0, 0xa);
    SET_HT_ADDR_TRANS(3, 0, 0xf);
#ifdef LS7A_2WAY_CONNECT
    SET_HT_ADDR_TRANS(4, 5, 0xa);
    SET_HT_ADDR_TRANS(5, 5, 0xe);
    SET_HT_ADDR_TRANS(6, 5, 0xf);
    SET_HT_ADDR_TRANS(7, 5, 0xb);
#else
    SET_HT_ADDR_TRANS(4, 0, 0xe);
    SET_HT_ADDR_TRANS(5, 0, 0xb);
    SET_HT_ADDR_TRANS(6, 0, 0xe);
    SET_HT_ADDR_TRANS(7, 0, 0xe);
#endif
#ifdef DISABLE_X_LINK
    SET_HT_ADDR_TRANS(3, 0, 0xa);

#ifdef LS7A_2WAY_CONNECT
    SET_HT_ADDR_TRANS(6, 5, 0xb);
#else
    SET_HT_ADDR_TRANS(6, 0, 0xb);
#endif
#endif

/*16 ways*/
#elif	(TOT_NODE_NUM == 16)
    SET_HT_ADDR_TRANS(1, 0, 0xb);
    SET_HT_ADDR_TRANS(2, 0, 0xa);
    SET_HT_ADDR_TRANS(3, 0, 0xf);

    SET_HT_ADDR_TRANS(4, 0, 0xe);
    SET_HT_ADDR_TRANS(5, 0, 0xb);
    SET_HT_ADDR_TRANS(7, 0, 0xf);
    SET_HT_ADDR_TRANS(12, 0, 0xe);
#ifdef	LS7A_2WAY_CONNECT
    SET_HT_ADDR_TRANS(6, 10, 0xe);

    SET_HT_ADDR_TRANS(8, 10, 0xb);
    SET_HT_ADDR_TRANS(9, 10, 0xf);
    SET_HT_ADDR_TRANS(10, 10, 0xe);
    SET_HT_ADDR_TRANS(11, 10, 0xa);

    SET_HT_ADDR_TRANS(13, 10, 0xb);
    SET_HT_ADDR_TRANS(14, 10, 0xe);
    SET_HT_ADDR_TRANS(15, 10, 0xa);
#else
    SET_HT_ADDR_TRANS(6, 0, 0xa);

    SET_HT_ADDR_TRANS(8, 0, 0xe);
    SET_HT_ADDR_TRANS(9, 0, 0xb);
    SET_HT_ADDR_TRANS(10, 0, 0xa);
    SET_HT_ADDR_TRANS(11, 0, 0xb);

    SET_HT_ADDR_TRANS(13, 0, 0xb);
    SET_HT_ADDR_TRANS(14, 0, 0xa);
    SET_HT_ADDR_TRANS(15, 0, 0xf);
#endif
#endif

