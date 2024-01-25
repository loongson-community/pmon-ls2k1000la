#if 1
#include <sys/param.h>
#include <sys/syslog.h>
#include <machine/endian.h>
#include <sys/device.h>
#include <machine/cpu.h>
#include <machine/pio.h>
#include <machine/intr.h>
#include <dev/pci/pcivar.h>
#endif
#include <sys/types.h>
#include <termio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <dev/ic/mc146818reg.h>
#include <linux/io.h>

#include <autoconf.h>

#include "pflash.h"
#include "dev/pflash_tgt.h"

#include "target/bonito.h"
#include "target/ls2k1000.h"
#include "target/board.h"
#include <pmon/dev/gt64240reg.h>
#include <pmon/dev/ns16550.h>

#include <pmon.h>

#include "mod_x86emu_int10.h"
#include "mod_x86emu.h"
#include "mod_vgacon.h"
#include "mod_framebuffer.h"

typedef unsigned char         u8;
typedef unsigned short          u16;
typedef unsigned int         u32;
typedef unsigned char         UINT8;
typedef unsigned short          UINT16;
typedef unsigned int         UINT32;

u8     I2CADR = 0x2d; //0x5a
void i2c_init(int speed,  int slaveaddr);
int i2c_read(u8 chip, uint addr, int alen, u8 *buffer, int len);
int i2c_write(u8 chip, uint addr, int alen, u8 *buffer, int len);

#define  gpioi2c_write(chip, reg, data)  i2c_write(chip, reg, 1, data, 1);
#define  gpioi2c_read(chip, reg, data) i2c_read(chip, reg, 1, data, 1);
typedef enum
{ 
  Bit_RESET = 0,
  Bit_SET = !Bit_RESET
}bool;


typedef enum LT9211_OUTPUTMODE_ENUM
{
    OUTPUT_RGB888=0,
    OUTPUT_BT656_8BIT=1,
    OUTPUT_BT1120_16BIT=2,
    OUTPUT_LVDS_2_PORT=3,
    OUTPUT_LVDS_1_PORT=4,
    OUTPUT_YCbCr444=5,
    OUTPUT_YCbCr422_16BIT
};
#define LT9211_OutPutModde  OUTPUT_LVDS_1_PORT

typedef enum VIDEO_INPUTMODE_ENUM
{
    Input_RGB888,
    Input_YCbCr444,
    Input_YCbCr422_16BIT
}
_Video_Input_Mode;

#define Video_Input_Mode  Input_RGB888


//#define lvds_format_JEIDA

//#define lvds_sync_de_only


//////////option for debug///////////


typedef struct video_timing{
    u16 hfp;
    u16 hs;
    u16 hbp;
    u16 hact;
    u16 htotal;
    u16 vfp;
    u16 vs;
    u16 vbp;
    u16 vact;
    u16 vtotal;
    u32 pclk_khz;
};

typedef enum VideoFormat
{
    video_384x292_60Hz_vic,
    video_640x480_60Hz_vic,
    video_1280x720_60Hz_vic,
    video_1366x768_60Hz_vic,
    video_1280x1024_60Hz_vic,
    video_1920x1080_60Hz_vic,
    video_1920x1200_60Hz_vic,
    video_1920x1080_25Hz_vic,
    video_none
};

typedef struct Lane_No{
u8	swing_high_byte;
u8	swing_low_byte;
u8	emph_high_byte;
u8	emph_low_byte;
};
u16 hact, vact;
u16 hs, vs;
u16 hbp, vbp;
u16 htotal, vtotal;
u16 hfp, vfp;
u8 VideoFormat=0;

enum VideoFormat Video_Format;
struct video_timing TimingStr;

#define print printf
//hfp, hs, hbp,hact,htotal,vfp, vs, vbp, vact,vtotal,
struct video_timing video_640x480_60Hz     ={ 8, 96,  40, 640,   800, 33,  2,  10, 480,   525,  25000};
struct video_timing video_720x480_60Hz     ={16, 62,  60, 720,   858,  9,  6,  30, 480,   525,  27000};
struct video_timing video_1280x720_60Hz    ={110,40, 220,1280,  1650,  5,  5,  20, 720,   750,  74250};
struct video_timing video_1280x720_30Hz    ={110,40, 220,1280,  1650,  5,  5,  20, 720,   750,  37125};
struct video_timing video_1366x768_60Hz    ={26, 110,110,1366,  1592,  13, 6,  13, 768,   800,  81000};
struct video_timing video_1920x1080_30Hz   ={88, 44, 148,1920,  2200,  4,  5,  36, 1080, 1125,  74250};
struct video_timing video_1920x1080_60Hz   ={88, 44, 148,1920,  2200,  4,  5,  36, 1080, 1125, 148500};
struct video_timing video_3840x1080_60Hz   ={176,88, 296,3840,  4400,  4,  5,  36, 1080, 1125, 297000};
struct video_timing video_1920x1200_60Hz   ={48, 32,  80,1920,  2080,  3,  6,  26, 1200, 1235, 154000};
struct video_timing video_3840x2160_30Hz   ={176,88, 296,3840,  4400,  8,  10, 72, 2160, 2250, 297000};
struct video_timing video_3840x2160_60Hz   ={176,88, 296,3840,  4400,  8,  10, 72, 2160, 2250, 594000};

struct video_timing video_1920x1080_25Hz   ={528, 44, 148,1920,  2640,  4,  5,  36, 1080, 1125,  74250};

void Timer0_Delay1ms(UINT32 u32CNT)
{
        delay(1000*u32CNT);                      	
}
u8 HDMI_ReadI2C_Byte(u8 RegAddr)
{
	u8  p_data=0;
	gpioi2c_read(I2CADR,RegAddr,&p_data);
	return p_data;
}
bool HDMI_WriteI2C_Byte(u8 RegAddr, u8 d)
{
gpioi2c_write(I2CADR, RegAddr,&d);
return TRUE;
}
/** video chk soft rst **/
void lt9211_vid_chk_rst(void)       
{
    HDMI_WriteI2C_Byte(0xff,0x81);	  
    HDMI_WriteI2C_Byte(0x10,0xbe); 
    Timer0_Delay1ms(10);
    HDMI_WriteI2C_Byte(0x10,0xfe); 
}

void LT9211_ChipID(void)
{
    HDMI_WriteI2C_Byte(0xff,0x81);//register bank
    print("\r\nLT9211 Chip ID:%x,",HDMI_ReadI2C_Byte(0x00));
    print("%x, ",HDMI_ReadI2C_Byte(0x01));
    print("%x, ",HDMI_ReadI2C_Byte(0x02));
}

void LT9211_SystemInt(void)
{
    /* system clock init */		   
    HDMI_WriteI2C_Byte(0xff,0x82);
    HDMI_WriteI2C_Byte(0x01,0x18);
	
    HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x06,0x61); 	
    HDMI_WriteI2C_Byte(0x07,0xa8); //fm for sys_clk
	  
    HDMI_WriteI2C_Byte(0xff,0x87); //??? txpll ???????????
    HDMI_WriteI2C_Byte(0x14,0x08); //default value
    HDMI_WriteI2C_Byte(0x15,0x00); //default value
    HDMI_WriteI2C_Byte(0x18,0x0f);
    HDMI_WriteI2C_Byte(0x22,0x08); //default value
    HDMI_WriteI2C_Byte(0x23,0x00); //default value
    HDMI_WriteI2C_Byte(0x26,0x0f); 
}

void LT9211_ClockCheckDebug(void)
{
#ifdef _uart_debug_
    u32 fm_value;

	HDMI_WriteI2C_Byte(0xff,0x86);
	HDMI_WriteI2C_Byte(0x00,0x0a);
	Timer0_Delay1ms(300);
    fm_value = 0;
	fm_value = (HDMI_ReadI2C_Byte(0x08) &(0x0f));
    fm_value = (fm_value<<8) ;
	fm_value = fm_value + HDMI_ReadI2C_Byte(0x09);
	fm_value = (fm_value<<8) ;
	fm_value = fm_value + HDMI_ReadI2C_Byte(0x0a);
	print("\r\ndessc pixel clock: ");
	printdec_u32(fm_value);
#endif
}

void LT9211_TxDigital(void)
{
    print("\r\nLT9211 OUTPUT_MODE: ");
	if( LT9211_OutPutModde == OUTPUT_RGB888 )
	 {
        print("\rLT9211 set to OUTPUT_RGB888");
        HDMI_WriteI2C_Byte(0xff,0x85);
        HDMI_WriteI2C_Byte(0x88,0x90);
		HDMI_WriteI2C_Byte(0x60,0x00);
		HDMI_WriteI2C_Byte(0x6d,0x03);//0x07
		HDMI_WriteI2C_Byte(0x6E,0x00);
		HDMI_WriteI2C_Byte(0xff,0x81);
		HDMI_WriteI2C_Byte(0x36,0xc0); //bit7:ttltx_pixclk_en;bit6:ttltx_BT_clk_en
	 }
	else if( LT9211_OutPutModde == OUTPUT_BT656_8BIT )
	 {
		print("\rLT9211 set to OUTPUT_BT656_8BIT");
	 	HDMI_WriteI2C_Byte(0xff,0x85);
        HDMI_WriteI2C_Byte(0x88,0x90);
		HDMI_WriteI2C_Byte(0x60,0x34);
		HDMI_WriteI2C_Byte(0x6d,0x00);//0x08 YC SWAP
		HDMI_WriteI2C_Byte(0x6e,0x07);//low 16BIT
		
		HDMI_WriteI2C_Byte(0xff,0x81);
		HDMI_WriteI2C_Byte(0x0d,0xfd);
		HDMI_WriteI2C_Byte(0x0d,0xff);
		HDMI_WriteI2C_Byte(0xff,0x81);
		HDMI_WriteI2C_Byte(0x36,0xc0); //bit7:ttltx_pixclk_en;bit6:ttltx_BT_clk_en
	 }
	else if( LT9211_OutPutModde == OUTPUT_BT1120_16BIT )
	{ 
        print("\rLT9211 set to OUTPUT_BT1120_16BIT");		
		HDMI_WriteI2C_Byte(0xff,0x85);
        HDMI_WriteI2C_Byte(0x88,0x90);
		HDMI_WriteI2C_Byte(0x60,0x33);
		HDMI_WriteI2C_Byte(0x6d,0x08);//0x08 YC SWAP
		HDMI_WriteI2C_Byte(0x6e,0x06);//HIGH 16BIT
		
		HDMI_WriteI2C_Byte(0xff,0x81);
		HDMI_WriteI2C_Byte(0x0d,0xfd);
		HDMI_WriteI2C_Byte(0x0d,0xff);		
	}
	else if( (LT9211_OutPutModde == OUTPUT_LVDS_2_PORT) || (LT9211_OutPutModde == OUTPUT_LVDS_1_PORT) ) 
	{
		print("\rLT9211 set to OUTPUT_LVDS");
		HDMI_WriteI2C_Byte(0xff,0x85); /* lvds tx controller */
		HDMI_WriteI2C_Byte(0x59,0x50); 	
		HDMI_WriteI2C_Byte(0x5a,0xaa); //0xaa
		HDMI_WriteI2C_Byte(0x5b,0xaa);
		if( LT9211_OutPutModde == OUTPUT_LVDS_2_PORT )
		{
			HDMI_WriteI2C_Byte(0x5c,0x01);	//lvdstx port sel 01:dual;00:single
		}
		else
		{
			HDMI_WriteI2C_Byte(0x5c,0x00);
		}
		HDMI_WriteI2C_Byte(0x88,0x80);	
		HDMI_WriteI2C_Byte(0xa1,0x77); 	
		HDMI_WriteI2C_Byte(0xff,0x86);	
		HDMI_WriteI2C_Byte(0x40,0x40); //tx_src_sel
		/*port src sel*/
		HDMI_WriteI2C_Byte(0x41,0x34);	
		HDMI_WriteI2C_Byte(0x42,0x10);
		HDMI_WriteI2C_Byte(0x43,0x23); //pt0_tx_src_sel
		HDMI_WriteI2C_Byte(0x44,0x41);
		HDMI_WriteI2C_Byte(0x45,0x02); //pt1_tx_src_scl

#ifdef lvds_format_JEIDA
        HDMI_WriteI2C_Byte(0xff,0x85);
		HDMI_WriteI2C_Byte(0x59,0xd0); 	
		HDMI_WriteI2C_Byte(0xff,0xd8);
		HDMI_WriteI2C_Byte(0x11,0x40);
#endif	
	}  		
}

void LT9211_TTLRxPhy(void)
{
	HDMI_WriteI2C_Byte(0xff,0x82);
	HDMI_WriteI2C_Byte(0x28,0x40);
	HDMI_WriteI2C_Byte(0x61,0x09);
 
	//Data mapping
	HDMI_WriteI2C_Byte(0xff,0x85);
	HDMI_WriteI2C_Byte(0x88,0x80);
	HDMI_WriteI2C_Byte(0x45,0x00);	//RGB swap 0x30
	//HDMI_WriteI2C_Byte(0x48,0x58); 	//8BIT
}

void LT9211_RXCSC(void)
{
    HDMI_WriteI2C_Byte(0xff,0xf9);
    if( LT9211_OutPutModde == OUTPUT_RGB888 )
	{
        if( Video_Input_Mode == Input_RGB888 )
        {
            HDMI_WriteI2C_Byte(0x86,0x00);
            HDMI_WriteI2C_Byte(0x87,0x00);
        }
        else if ( Video_Input_Mode == Input_YCbCr444 )
        {
            HDMI_WriteI2C_Byte(0x86,0x0f);
            HDMI_WriteI2C_Byte(0x87,0x00);
        }
        else if ( Video_Input_Mode == Input_YCbCr422_16BIT )
        {
            HDMI_WriteI2C_Byte(0x86,0x00);
            HDMI_WriteI2C_Byte(0x87,0x03);				
        }
    }
    else if( (LT9211_OutPutModde == OUTPUT_BT656_8BIT) || (LT9211_OutPutModde ==OUTPUT_BT1120_16BIT) || (LT9211_OutPutModde ==OUTPUT_YCbCr422_16BIT) )
    {  
        if( Video_Input_Mode == Input_RGB888 )
        {
            HDMI_WriteI2C_Byte(0x86,0x0f);
            HDMI_WriteI2C_Byte(0x87,0x30);
        }
        else if ( Video_Input_Mode == Input_YCbCr444 )
        {
            HDMI_WriteI2C_Byte(0x86,0x00);
            HDMI_WriteI2C_Byte(0x87,0x30);
        }
        else if ( Video_Input_Mode == Input_YCbCr422_16BIT )
        {
            HDMI_WriteI2C_Byte(0x86,0x00);
            HDMI_WriteI2C_Byte(0x87,0x00);				
        }	 
    }
    else if( LT9211_OutPutModde == OUTPUT_YCbCr444 )
	{
        if( Video_Input_Mode == Input_RGB888 )
        {
            HDMI_WriteI2C_Byte(0x86,0x0f);
            HDMI_WriteI2C_Byte(0x87,0x00);
        }
        else if ( Video_Input_Mode == Input_YCbCr444 )
        {
            HDMI_WriteI2C_Byte(0x86,0x00);
            HDMI_WriteI2C_Byte(0x87,0x00);
        }
        else if ( Video_Input_Mode == Input_YCbCr422_16BIT )
        {
            HDMI_WriteI2C_Byte(0x86,0x00);
            HDMI_WriteI2C_Byte(0x87,0x03);				
        }
    }
}

void LT9211_Txpll(void)
{
    u8 loopx;
    if( LT9211_OutPutModde == OUTPUT_BT656_8BIT )
    {
        HDMI_WriteI2C_Byte(0xff,0x82);
        HDMI_WriteI2C_Byte(0x2d,0x40);
        HDMI_WriteI2C_Byte(0x30,0x50);
        HDMI_WriteI2C_Byte(0x33,0x55);
    }
    else if( (LT9211_OutPutModde == OUTPUT_LVDS_2_PORT) || (LT9211_OutPutModde == OUTPUT_LVDS_1_PORT) || (LT9211_OutPutModde == OUTPUT_RGB888) || (LT9211_OutPutModde ==OUTPUT_BT1120_16BIT) )
    {
        HDMI_WriteI2C_Byte(0xff,0x82);
        HDMI_WriteI2C_Byte(0x36,0x01); //b7:txpll_pd
        if( LT9211_OutPutModde == OUTPUT_LVDS_1_PORT )
        {
            HDMI_WriteI2C_Byte(0x37,0x29);
        }
        else
        {
            HDMI_WriteI2C_Byte(0x37,0x2a);
        }
		HDMI_WriteI2C_Byte(0x38,0x06);
		HDMI_WriteI2C_Byte(0x39,0x30);
		HDMI_WriteI2C_Byte(0x3a,0x8e);
		HDMI_WriteI2C_Byte(0xff,0x87);
		HDMI_WriteI2C_Byte(0x37,0x14);
		HDMI_WriteI2C_Byte(0x13,0x00);
		HDMI_WriteI2C_Byte(0x13,0x80);
		Timer0_Delay1ms(100);
		for(loopx = 0; loopx < 10; loopx++) //Check Tx PLL cal
		{
            HDMI_WriteI2C_Byte(0xff,0x87);			
            if(HDMI_ReadI2C_Byte(0x1f)& 0x80)
			{
				if(HDMI_ReadI2C_Byte(0x20)& 0x80)
				{
					print("\r\nLT9211 tx pll lock");
				}
				else
				{
                    print("\r\nLT9211 tx pll unlocked");
                }					
				print("\r\nLT9211 tx pll cal done");
				break;
            }
			else
			{
				print("\r\nLT9211 tx pll unlocked");
			}
		}
    } 
    print("\r\n system success");	 	
}

void LT9211_TxPhy(void)
{		
    HDMI_WriteI2C_Byte(0xff,0x82);
    if( (LT9211_OutPutModde == OUTPUT_RGB888) || (LT9211_OutPutModde ==OUTPUT_BT656_8BIT) || (LT9211_OutPutModde ==OUTPUT_BT1120_16BIT) )
    {
        HDMI_WriteI2C_Byte(0x62,0x01); //ttl output enable
        HDMI_WriteI2C_Byte(0x6b,0xff);
    }
    else if( (LT9211_OutPutModde == OUTPUT_LVDS_2_PORT) || (LT9211_OutPutModde ==OUTPUT_LVDS_1_PORT) )
    {
        /* dual-port lvds tx phy */	
        HDMI_WriteI2C_Byte(0x62,0x00); //ttl output disable
		if(LT9211_OutPutModde == OUTPUT_LVDS_2_PORT)
		{
            HDMI_WriteI2C_Byte(0x3b,0xb8);
		}
		else
		{
            HDMI_WriteI2C_Byte(0x3b,0x38);
		}
        // HDMI_WriteI2C_Byte(0x3b,0xb8); //dual port lvds enable	
		HDMI_WriteI2C_Byte(0x3e,0x92); 
		HDMI_WriteI2C_Byte(0x3f,0x48); 	
		HDMI_WriteI2C_Byte(0x40,0x31); 		
		HDMI_WriteI2C_Byte(0x43,0x80); 		
		HDMI_WriteI2C_Byte(0x44,0x00);
		HDMI_WriteI2C_Byte(0x45,0x00); 		
		HDMI_WriteI2C_Byte(0x49,0x00);
		HDMI_WriteI2C_Byte(0x4a,0x01);
		HDMI_WriteI2C_Byte(0x4e,0x00);		
		HDMI_WriteI2C_Byte(0x4f,0x00);
		HDMI_WriteI2C_Byte(0x50,0x00);
		HDMI_WriteI2C_Byte(0x53,0x00);
		HDMI_WriteI2C_Byte(0x54,0x01);
		HDMI_WriteI2C_Byte(0xff,0x81);
		HDMI_WriteI2C_Byte(0x20,0x7b); 
		HDMI_WriteI2C_Byte(0x20,0xff); //mlrx mltx calib reset
    }
}

void LT9211_VideoCheckDebug(void)
{
#ifdef _uart_debug_
	u8 sync_polarity;

	HDMI_WriteI2C_Byte(0xff,0x86);
    HDMI_WriteI2C_Byte(0x20,0x00);
    Timer0_Delay1ms(100);
	sync_polarity = HDMI_ReadI2C_Byte(0x70);
	vs = HDMI_ReadI2C_Byte(0x71);

	hs = HDMI_ReadI2C_Byte(0x72);
    hs = (hs<<8) + HDMI_ReadI2C_Byte(0x73);
	
	vbp = HDMI_ReadI2C_Byte(0x74);
    vfp = HDMI_ReadI2C_Byte(0x75);

	hbp = HDMI_ReadI2C_Byte(0x76);
	hbp = (hbp<<8) + HDMI_ReadI2C_Byte(0x77);

	hfp = HDMI_ReadI2C_Byte(0x78);
	hfp = (hfp<<8) + HDMI_ReadI2C_Byte(0x79);

	vtotal = HDMI_ReadI2C_Byte(0x7A);
	vtotal = (vtotal<<8) + HDMI_ReadI2C_Byte(0x7B);

	htotal = HDMI_ReadI2C_Byte(0x7C);
	htotal = (htotal<<8) + HDMI_ReadI2C_Byte(0x7D);

	vact = HDMI_ReadI2C_Byte(0x7E);
	vact = (vact<<8)+ HDMI_ReadI2C_Byte(0x7F);

	hact = HDMI_ReadI2C_Byte(0x80);
	hact = (hact<<8) + HDMI_ReadI2C_Byte(0x81);

	print("\r\nsync_polarity = %x", sync_polarity);

    print("\r\nhfp, hs, hbp, hact, htotal = ");
	printdec_u32(hfp);
	printdec_u32(hs);
	printdec_u32(hbp);
	printdec_u32(hact);
	printdec_u32(htotal);

	print("\r\nvfp, vs, vbp, vact, vtotal = ");
	printdec_u32(vfp);
	printdec_u32(vs);
	printdec_u32(vbp);
	printdec_u32(vact);
	printdec_u32(vtotal);
#endif
}

void LT9211_BT_Set(void)
{
	 u16 tmp_data;
	if( (LT9211_OutPutModde == OUTPUT_BT1120_16BIT) || (LT9211_OutPutModde == OUTPUT_BT656_8BIT) )
	{
		tmp_data = hs+hbp;
		HDMI_WriteI2C_Byte(0xff,0x85);
		HDMI_WriteI2C_Byte(0x61,(u8)(tmp_data>>8));
		HDMI_WriteI2C_Byte(0x62,(u8)tmp_data);
		HDMI_WriteI2C_Byte(0x63,(u8)(hact>>8));
		HDMI_WriteI2C_Byte(0x64,(u8)hact);
		HDMI_WriteI2C_Byte(0x65,(u8)(htotal>>8));
		HDMI_WriteI2C_Byte(0x66,(u8)htotal);		
		tmp_data = vs+vbp;
		HDMI_WriteI2C_Byte(0x67,(u8)tmp_data);
		HDMI_WriteI2C_Byte(0x68,0x00);
		HDMI_WriteI2C_Byte(0x69,(u8)(vact>>8));
		HDMI_WriteI2C_Byte(0x6a,(u8)vact);
		HDMI_WriteI2C_Byte(0x6b,(u8)(vtotal>>8));
		HDMI_WriteI2C_Byte(0x6c,(u8)vtotal);		
	}
}

void LT9211_SetTimingPara(void)
{
    //Htotal
	HDMI_WriteI2C_Byte(0xff,0x85);
	HDMI_WriteI2C_Byte(0x20, (u8) ((TimingStr.hact*2) >> 8));
	HDMI_WriteI2C_Byte(0x21, (u8) (TimingStr.hact*2));
	//HFP
	HDMI_WriteI2C_Byte(0x22, (u8) ((TimingStr.hfp*2) >> 8));
	HDMI_WriteI2C_Byte(0x23, (u8) (TimingStr.hfp*2));
	//HSW
	HDMI_WriteI2C_Byte(0x24, (u8) ((TimingStr.hs*2) >> 8));
	HDMI_WriteI2C_Byte(0x25, (u8) (TimingStr.hs*2));
	//VFP
	HDMI_WriteI2C_Byte(0x38, (u8) (TimingStr.vfp >> 8));
	HDMI_WriteI2C_Byte(0x39, (u8) (TimingStr.vfp));
	//VSW
	HDMI_WriteI2C_Byte(0x3C, (u8) (TimingStr.vs >> 8));
	HDMI_WriteI2C_Byte(0x3D, (u8) (TimingStr.vs));
}

void LT9211_Patten(struct video_timing *video_format)
{
    u32 pclk_khz;
    u8 dessc_pll_post_div;
    u32 pcr_m, pcr_k;

    pclk_khz = video_format->pclk_khz;     

    HDMI_WriteI2C_Byte(0xff,0xf9);
	HDMI_WriteI2C_Byte(0x3e,0x80);  

    HDMI_WriteI2C_Byte(0xff,0x85);
	HDMI_WriteI2C_Byte(0x88,0xc0);  

    HDMI_WriteI2C_Byte(0xa1,0x64); 
    HDMI_WriteI2C_Byte(0xa2,0xff); 

	HDMI_WriteI2C_Byte(0xa3,(u8)((video_format->hs+video_format->hbp)/256));
	HDMI_WriteI2C_Byte(0xa4,(u8)((video_format->hs+video_format->hbp)%256));//h_start

	HDMI_WriteI2C_Byte(0xa5,(u8)((video_format->vs+video_format->vbp)%256));//v_start

   	HDMI_WriteI2C_Byte(0xa6,(u8)(video_format->hact/256));
	HDMI_WriteI2C_Byte(0xa7,(u8)(video_format->hact%256)); //hactive

	HDMI_WriteI2C_Byte(0xa8,(u8)(video_format->vact/256));
	HDMI_WriteI2C_Byte(0xa9,(u8)(video_format->vact%256));  //vactive

   	HDMI_WriteI2C_Byte(0xaa,(u8)(video_format->htotal/256));
	HDMI_WriteI2C_Byte(0xab,(u8)(video_format->htotal%256));//htotal

   	HDMI_WriteI2C_Byte(0xac,(u8)(video_format->vtotal/256));
	HDMI_WriteI2C_Byte(0xad,(u8)(video_format->vtotal%256));//vtotal

   	HDMI_WriteI2C_Byte(0xae,(u8)(video_format->hs/256)); 
	HDMI_WriteI2C_Byte(0xaf,(u8)(video_format->hs%256));   //hsa

    HDMI_WriteI2C_Byte(0xb0,(u8)(video_format->vs%256));    //vsa

    //dessc pll to generate pixel clk
	HDMI_WriteI2C_Byte(0xff,0x82); //dessc pll
	HDMI_WriteI2C_Byte(0x2d,0x48); //pll ref select xtal 

	if(pclk_khz < 44000)
	{
	  	HDMI_WriteI2C_Byte(0x35,0x83);
		dessc_pll_post_div = 16;
	}

	else if(pclk_khz < 88000)
	{
	  	HDMI_WriteI2C_Byte(0x35,0x82);
		dessc_pll_post_div = 8;
	}

	else if(pclk_khz < 176000)
	{
	  	HDMI_WriteI2C_Byte(0x35,0x81);
		dessc_pll_post_div = 4;
	}

	else if(pclk_khz < 352000)
	{
	  	HDMI_WriteI2C_Byte(0x35,0x80);
		dessc_pll_post_div = 0;
	}

	pcr_m = (pclk_khz * dessc_pll_post_div) /25;
	pcr_k = pcr_m%1000;
	pcr_m = pcr_m/1000;

	pcr_k <<= 14; 

	//pixel clk
 	HDMI_WriteI2C_Byte(0xff,0xd0); //pcr
	HDMI_WriteI2C_Byte(0x2d,0x7f);
	HDMI_WriteI2C_Byte(0x31,0x00);

	HDMI_WriteI2C_Byte(0x26,0x80|((u8)pcr_m));
	HDMI_WriteI2C_Byte(0x27,(u8)((pcr_k>>16)&0xff)); //K
	HDMI_WriteI2C_Byte(0x28,(u8)((pcr_k>>8)&0xff)); //K
	HDMI_WriteI2C_Byte(0x29,(u8)(pcr_k&0xff)); //K
}
	
void LT9211_Config(void)
{
	ls2k_i2c_init(0, LS2K1000_I2C0_REG_BASE);
    LT9211_ChipID();
    LT9211_SystemInt();
	LT9211_TTLRxPhy();
    LT9211_VideoCheckDebug();
	Timer0_Delay1ms(100);
    
	LT9211_TxDigital();
	LT9211_TxPhy();
	Timer0_Delay1ms(10);
	LT9211_Txpll();
	LT9211_RXCSC();
    LT9211_ClockCheckDebug();
}

