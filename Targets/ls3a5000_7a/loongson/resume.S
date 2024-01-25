#ifdef LS_STR
/*
 * 3A7A STR config start
 */
	.org 0x500

	/*set memory controller selfrefresh*/
	/* Enable DDR control register */
	li.d	t0, PHYS_TO_UNCACHED(0x1fe00000)
	ld.w    t1, t0,0x180
	li.w	t2, 0xfffffdef
	and     t1, t1, t2
	li.w	t3, 0x00000420;
	or      t1, t1, t3
	st.w    t1, t0,0x180
	dbar    0 //sync

	/* Set interleave_en is 0 */
	li.d	t0, PHYS_TO_UNCACHED(0x1fe00400)
	ld.w    t1, t0,0x4
	li.w	t2, (1 << 7)
	andn	t1, t1, t2
	st.w    t1, t0,0x4
	dbar    0 //sync

	li.d	t0, PHYS_TO_UNCACHED(0x4000ff00000)
	li.d	t1, 0x1308
	ldx.d   t1, t0,t1
	li.d	t2, 0x00000000000000ff
	or      t1, t1, t2
	li.d	t2, 0x1308
	stx.d   t1, t0,t2
	dbar    0 //sync

	li.d	t0, PHYS_TO_UNCACHED(0x5000ff00000)
	li.d	t1, 0x1308
	ldx.d   t1, t0,t1
	li.d	t2, 0x00000000000000ff
	or      t1, t1, t2
	li.d	t2, 0x1308
	stx.d   t1, t0,t2
	dbar    0 //sync

	/* delay */
	li.w	t0, 0x4000
1:
	addi.w	t0, t0, -1
	bnez	t0, 1b

	li.d	t0, PHYS_TO_UNCACHED(0xe00100d0000)
	/* set key,usb wakeup of reg GPE0_EN */
	ld.w    t1, t0,0x2c
	li.w	t3, (0x1 << 8)|(0x3f<<10)
	or      t1, t1, t3
	st.w    t1, t0,0x2c

	/* clear 0-15 of reg GPE0_STS */
	ld.w    t1, t0,0x28
	li.w	t3, 0x0000ffff
	st.w    t3, t0,0x28

	/* clear 0-15 of reg PM1_STS */
	ld.w    t1, t0,0x0c
	li.w	t3, 0x0000ffff
	st.w    t3, t0,0x0c

	/* set wake on line */
	ld.w    t1, t0,0x4
	li.w	t3, 0x80
	or      t1, t1, t3
	st.w    t1, t0,0x4

	/* set vsb_gat_delay */
	ld.w    t1, t0,0x4
	li.w	t3, 0x5 << 11
	or      t1, t1, t3
	li.d	t3, 0xefff
	and     t1, t1, t3
	st.w    t1, t0,0x4

	li.w	a0,'S'
	bl tgt_putchar
	li.w	a0,'3'
	bl tgt_putchar

	/* set reg PM1_CNT to get into S3*/
	li.w	t3, 0x00003400
	st.w    t3, t0, 0x14

	/* delay */
	li.w	t0, 0x40000
2:
	addi.w  t0, t0, -1
	bnez    t0, 2b

1:
	b  1b
#endif
