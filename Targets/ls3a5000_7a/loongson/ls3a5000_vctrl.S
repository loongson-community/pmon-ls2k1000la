
/*************************
 * this file i2c address use 64 bit access
 * this code used for control voltage
*************************/
#if	(TOT_NODE_NUM >= 8) && !defined(BOARD_3C5000L_2W_V01)
#define MPS_I2C_REG_BASE		LS3A5000_I2C1_REG_BASE
#else
#define MPS_I2C_REG_BASE		LS3A5000_I2C0_REG_BASE
#endif

#define PRER_LO_REG                     (0x0)
#define PRER_HI_REG                     (0x1)
#define CTR_REG                         (0x2)
#define TXR_REG                         (0x3)
#define RXR_REG                         (0x3)
#define CR_REG                                  (0x4)
#define SR_REG                                  (0x4)

#define CR_START                                0x80
#define CR_STOP                                 0x40
#define CR_READ                                 0x20
#define CR_WRITE                                0x10
#define CR_ACK                                  0x8
#define CR_IACK                                 0x1

#define SR_NOACK                                0x80
#define SR_BUSY                                 0x40
#define SR_AL                                   0x20
#define SR_TIP                                  0x2
#define SR_IF                                   0x1

/*
 * use register:
 *      a0, a3
 *      input: a3
 *      a3: i2c register base address
 */
LEAF(ls_v_i2cinit)
    //LPB clock_a,SCL clock_s,prescale = clock_a / (4 * clock_s);
    ld.b    a0, a3, CTR_REG
    andi    a0, a0, 0x7f /* ~(1 << 7) */
    st.b    a0, a3, CTR_REG

#ifdef BONITO_100M
    li.d	a0, 0x53 //100M
#else
    li.d	a0, 0x63 //25M
#endif
    st.b    a0, a3, PRER_LO_REG

#ifdef BONITO_100M
#if (TOT_NODE_NUM == 4)
    li.d	a0, 0x4 //
#else
    li.d	a0, 2 //
#endif
#else
    li.d	a0, 0x0 //25M
#endif
    st.b    a0, a3, 0x1

    ld.b    a0, a3, CTR_REG
    ori     a0, a0, (1 << 7)
    st.b    a0, a3, CTR_REG

    jirl    zero, ra, 0
   
END(ls_v_i2cinit)

/*
 * use register:
 *      a0, a1, a3, a4
 *      input: a0,a1,a2
 *      a0: device ID will change a0 to return value
 *      a1: register offset/command, [16-31] store write count now 1 mean 1 byte default 0/2  byte 2
 *      a2: configure value
 *      a3: i2c register base address
 */

LEAF(ls_v_i2cwrite)
/*i2c_send_addr*/
    /* load device address */
    ori     a4, a0, 0
    st.b    a4, a3, TXR_REG

    /* send start frame */
    li.d	a4, CR_START | CR_WRITE
    st.b    a4, a3, CR_REG

    /* wait send finished */
//      i2c_wait_tip
1:
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_TIP
    bnez    a4, 1b

    //check ACK
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_NOACK
    bnez    a4, 3f

    /* load data(offset/command) to be send */
    ori     a4, a1, 0
    st.b    a4, a3, TXR_REG

    /* send data frame */
    li.d	a4, CR_WRITE
    st.b    a4, a3, CR_REG

    /* wait send finished */
//      i2c_wait_tip
1:
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_TIP
    bnez    a4, 1b

    //check ACK
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_NOACK
    bnez    a4, 3f
/*i2c_send_addr*/

/* i2c write max data is word*/
/*i2c tx byte*/
    /* load configure value */
    ori     a4, a2, 0
    st.b    a4, a3, TXR_REG

    /* send start frame */
    li.d	a4, CR_WRITE
    st.b    a4, a3, CR_REG

    /* wait send finished */
//      i2c_wait_tip
1:
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_TIP
    bnez    a4, 1b

    //check ACK
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_NOACK
    bnez    a4, 3f

    /*get count*/
    li.d	a0, (1 << 16)
    and     a4, a1, a0
    bnez    a4, 3f

/*i2c tx byte*/
/*i2c tx byte*/
    /* load configure value */
    ori     a4, a2, 0
    srli.w  a4, a4, 8
    st.b    a4, a3, TXR_REG

    /* send start frame */
    li.d	a4, CR_WRITE
    st.b    a4, a3, CR_REG

    /* wait send finished */
//      i2c_wait_tip
1:
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_TIP
    bnez    a4, 1b

    //check ACK
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_NOACK
    bnez    a4, 3f

    b       4f

/*i2c tx byte*/
3:
/* i2c_stop */
    /* free i2c bus */
    li.d	a4, CR_STOP
    st.b    a4, a3, CR_REG
1:
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_BUSY
    bnez    a4, 1b

    li.d	a0, 0 //return value check this function
4:
    jirl    zero, ra, 0

END(ls_v_i2cwrite)

LEAF(ls_v_i2cread)
/*
 * use register:
 *      a0, a1, a2, a3
 *      input: a0,a1
 *      a0: device ID -> change to return value
 *      a1: register offset
 *
 */
/*i2c_send_addr*/
/*send device addr*/
    /* load device address */
    ori     a2, a0, 0
    st.b    a2, a3, TXR_REG

    /* send start frame */
    li.d	a2, CR_START | CR_WRITE
    st.b    a2, a3, CR_REG

    /* wait send finished */
//      i2c_wait_tip
1:
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_TIP
    bnez    a2, 1b

    //check ACK
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_NOACK
    bnez    a2, 3f

/*send device addr*/
    /* load data(offset/command) to be send */
    ori     a2, a1, 0
    st.b    a2, a3, TXR_REG

    /* send data frame */
    li.d	a2, CR_WRITE
    st.b    a2, a3, CR_REG

    /* wait send finished */
//      i2c_wait_tip
1:
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_TIP
    bnez    a2, 1b

    //check ACK
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_NOACK
    bnez    a2, 3f

/*i2c_send_addr*/

/* i2c_read_word*/
/*send device addr*/
    /* load device address (dev_addr | 0x1/READ) */
    ori     a2, a0, 0x1
    st.b    a2, a3, TXR_REG

    /* send start frame */
    li.d	a2, CR_START | CR_WRITE
    st.b    a2, a3, CR_REG

    /* wait send finished */
//      i2c_wait_tip
1:
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_TIP
    bnez    a2, 1b


     //check ACK
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_NOACK
    bnez    a2, 3f

/*send device addr*/

    /* receive data from fifo */
    li.d	a2, CR_READ
    st.b    a2, a3, CR_REG

//      i2c_wait_tip
1:
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_TIP
    bnez    a2, 1b

    /* read data from fifo */
    ld.bu   a1, a3, RXR_REG

    /* receive data from fifo */
    li.d	a2, CR_READ | CR_ACK
    st.b    a2, a3, CR_REG

//      i2c_wait_tip
1:
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_TIP
    bnez    a2, 1b

    /* read data from fifo */
    ld.bu   a2, a3, RXR_REG
    slli.w  a2, a2, 0x8
    add.w   a1, a1, a2
3:
/* i2c_stop */
    /* free i2c bus */
    li.d	a2, CR_STOP
    st.b    a2, a3, CR_REG
1:
    ld.bu   a2, a3, SR_REG
    andi    a2, a2, SR_BUSY
    bnez    a2, 1b

    ori     a0, a1, 0

    jirl    zero, ra, 0

END(ls_v_i2cread)

LEAF(v_n_ctrl)
/*
 * use register:
 * a0,a1,a2,a3,a4,a5,a6
 * a0 voltage value
 * a1 node number
 * a0: device ID / return value
 * a1: command
 * a2: command value
 * a5: save ra
 */
    ori     a5, ra, 0
    move    a6, a0

    li.d	a3, MPS_I2C_REG_BASE
    slli.d  a1, a1, 44
    or      a3, a3, a1
    /*this code can not change a3*/
    bl      ls_v_i2cinit

#ifdef TY_MULTI_BOARD /* for TY i2c switch */
    li.d	a0, 0xe0
    li.d	a1, 0x0
    li.d	a2, (0x1 << 16)
    or      a1, a1, a2 //write 1 byte
    li.d	a2, 0x1
    bl      ls_v_i2cwrite
#endif
#if 0 /* page select */
    li.d	a0, MPS_ADDR
    li.d	a1, 0x5e
    li.d	a2, 0x210
    bl      ls_v_i2cwrite

    li.d	a4, CR_STOP
    st.b    a4, a3, CR_REG
1:
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_BUSY
    bnez    a4, 1b
#endif

    //li      a2, 31 //0.8v
    //li      a2, 36 //0.85v
    //li      a2, 41 //0.9v
    //li      a2, 46 //0.95v
    //li      a2, 51 //1.0v
    //li      a2, 56 //1.05v
    //li      a2, 61 //1.1v
    move    a2, a6

    li.d	a0, MPS_ADDR
    li.d	a1, 0x21 //VOUT_COMMAND
    bl      ls_v_i2cwrite
    /*now a0 store return value*/
    beqz    a0, 2f


    li.d	a4, CR_STOP
    st.b    a4, a3, CR_REG
1:
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_BUSY
    bnez    a4, 1b

    b 3f
2:
    //TTYDBG("v ctrl err\r\n")
    ori     ra, a5, 0
    jirl    zero, ra, 0
3:
    //TTYDBG("v ctrl end\r\n")
    ori     ra, a5, 0
    jirl    zero, ra, 0

END(v_n_ctrl)

LEAF(v_n_ctrl_read)
/*
 * use register:
 * a0,a1,a2,a3,a6
 * a0: device ID
 * a1: command
 * a3: i2c register base
 * a6: save ra
 */

    ori     a6, ra, 0

    li.d	a3, MPS_I2C_REG_BASE
    slli.d  a0, a0, 44
    or      a3, a3, a0
    bl      ls_v_i2cinit

    li.d	a0, MPS_ADDR
    li.d	a1, 0x8b //READ_VOUT
    bl      ls_v_i2cread
    /* now a0 store return value */
    bl      hexserial

    TTYDBG("\r\n")

    ori     ra, a6, 0
    jirl    zero, ra, 0

END(v_n_ctrl_read)

LEAF(v_p_ctrl)
/*
 * use register:
 * a0 voltage value
 * a1 node number
 * a0,a1,a2,a5,a6
 * a0: device ID / return value
 * a1: command
 * a2: command value
 * a5: save ra
 */
    ori     a5, ra, 0
    move    a6, a0

    li.d	a3, MPS_I2C_REG_BASE
    slli.d  a1, a1, 44
    or      a3, a3, a1
    bl      ls_v_i2cinit

    /*write one byte do not need completed i2c stop code*/
    li.d	a0, MPS_ADDR
    li.d	a1, 0 //page select command
    li.d	a2, (0x1 << 16)
    or      a1, a1, a2 //write 1 byte
    li.d	a2, 1
    bl      ls_v_i2cwrite


    li.d	a0, MPS_ADDR
    li.d	a1, 0x21 //VOUT_COMMAND
    //li      a2, 31 //0.8v
    //li      a2, 36 //0.85v
    //li      a2, 41 //0.9v
    //li      a2, 46 //0.95v
    //li      a2, 51 //1.0v
    //li      a2, 56 //1.05v
    //li      a2, 61 //1.1v
    move    a2, a6
    bl      ls_v_i2cwrite

    li.d	a4, CR_STOP
    st.b    a4, a3, CR_REG
1:
    ld.bu   a4, a3, SR_REG
    andi    a4, a4, SR_BUSY
    bnez    a4, 1b

    li.d	a0, MPS_ADDR
    li.d	a1, 0 //page select command
    li.d	a2, (0x1 << 16)
    or      a1, a1, a2 //write 1 byte
    li.d	a2, 0
    bl      ls_v_i2cwrite

    /*mps chip control one byte do not check return value*/

    TTYDBG("io ctrl end\r\n")
    ori     ra, a5, 0
    jirl    zero, ra, 0

2:
    TTYDBG("io ctrl err\r\n")
    ori     ra, a5, 0
    jirl    zero, ra, 0

END(v_p_ctrl)
