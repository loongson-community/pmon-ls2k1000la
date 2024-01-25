#include <pmon.h>
#include <string.h>
#include <linux/types.h>


void i2c_write_bypass( uint addr, u8 * buffer)
{
	i2c_write(0x2f, addr, 1, buffer, 1);
}

#define EEPROM_BYPASS

#ifdef EEPROM_BYPASS
void i2c_write_eeprom( uint addr, u8 * buffer)
{
	i2c_write(0x57, addr, 1, buffer, 1);
}
#else
#define i2c_write_eeprom(a,b)
#endif

static int post_on()
{
	uint addr=0x20,addr1=0x01;
	u8 buffer[]={0x38};

	ls2k_i2c_init(0, 0x800000001fe21800);
	i2c_write_bypass( addr, buffer);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1,buffer);
#endif	
	return 0;
}
static int go_s5()
{
	uint addr=0x20,addr1=0x01;
	u8 buffer[]={0x89};

	ls2k_i2c_init(0, 0x800000001fe21800);
	i2c_write_bypass( addr, buffer);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1,buffer);
#endif	
	return 0;
}

void linkstart()
{
    unsigned char i,tmp;

    tmp = 0x55;
	ls2k_i2c_init(0, 0x800000001fe21800);
    for(i=0;i<5;i++)
    {
        i2c_write_bypass( 0x18, &tmp);
	    delay1(200);
    }
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(0x0d,&tmp);
#endif
}

void netbypass_init()
{
    unsigned char addr,addr1;
	unsigned char bypass;
    unsigned char offbypass;
	char *d = getenv ("netbypass");
	if(!d || !atob (&bypass, d, 10) || bypass < 0 || bypass > 256) 
    {
        bypass = 0x00;
    }
	ls2k_i2c_init(0, 0x800000001fe21800);
    addr = 0x21,addr1=0x03;
	i2c_write_bypass( addr, &bypass);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1, &bypass);
#endif
	printf("netbypass = %d\n", bypass);
	
	char *t = getenv ("offbypass");
	if(!t || !atob (&offbypass, t, 10) || offbypass < 0 || offbypass > 256) 
    {
        offbypass = 0x00;
    }
	ls2k_i2c_init(0, 0x800000001fe21800);
    addr = 0x22,addr1=0x05;
	i2c_write_bypass( addr, &offbypass);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1, &offbypass);
#endif
	printf("offbypass = %d\n", offbypass);
}

static int power_on_bypass(int argc,char **argv)
{
	unsigned char addr,addr1;
	unsigned char buffer;
	unsigned char group;

	if(argc<3) 
    {
	    printf("power_on_bypass on/off group\n");
		goto err;
    }
	group=strtoul(argv[2],0,0);

    if(group <= 0 || group > 8)
    {
	    printf("power_on_bypass on/off [1...8]\n");
		goto err;
    }

	//printf("group=%d\n", group);

    addr=0x21,addr1=0x03;

	group = (1<<(group-1));

    unsigned int bypass;
	char *d = getenv ("netbypass");
	//printf("netbypass = %s\n", d);

	if(!d || !atob (&bypass, d, 10) || bypass < 0 || bypass > 256) 
    {
        bypass = 0x00;
    }

    if(!strcmp(argv[1],"on")) {
	    buffer = bypass | group;
    } else if (!strcmp(argv[1],"off")) {
	    buffer = bypass & (~group);
    } else {
	    printf("power_on_bypass ['on' or 'off'] group\n");
		goto err;
    }
    unsigned char t[5];
    btoa(t,buffer,10);
	setenv("netbypass", t);

	//printf("newnetbypass = %s\n", t);

	ls2k_i2c_init(0, 0x800000001fe21800);

	i2c_write_bypass( addr, &buffer);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1,&buffer);
#endif	
	return 0;
err:
	return -1;
}
static int power_off_bypass(int argc,char **argv)
{
	unsigned char addr,addr1;
	unsigned char buffer;
	unsigned char group;

	if(argc<3) 
    {
	    printf("power_off_bypass on/off group\n");
		goto err;
    }
	group=strtoul(argv[2],0,0);

    if(group <= 0 || group > 8)
    {
	    printf("power_off_bypass on/off [1...8]\n");
		goto err;
    }
	//printf("group=%d\n", group);

    addr=0x22,addr1=0x05;

	group = (1<<(group-1));

    unsigned int bypass;
	char *d = getenv ("offbypass");
	//printf("offbypass = %s\n", d);

	if(!d || !atob (&bypass, d, 10) || bypass < 0 || bypass > 256) 
    {
        bypass = 0x00;
    }

    if(!strcmp(argv[1],"on")) {
	    buffer = bypass | group;
    } else if (!strcmp(argv[1],"off")) {
	    buffer = bypass & (~group);
    } else {
	    printf("power_off_bypass ['on' or 'off'] group\n");
		goto err;
    }
    unsigned char t[5];
    btoa(t,buffer,10);
	setenv("offbypass", t);

	//printf("newoffbypass = %s\n", t);

	ls2k_i2c_init(0, 0x800000001fe21800);

	i2c_write_bypass( addr, &buffer);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1,&buffer);
#endif	
	return 0;
err:
	return -1;
}
static int bypass_info(int argc,char **argv)
{
	u8 addr,i;
    u8 onbypass,offbypass;
#ifdef EEPROM_BYPASS
	ls2k_i2c_init(0, 0x800000001fe21800);
    addr = 0x03;
    i2c_read(0x57, addr, 1, &onbypass, 1);
	printf("\npower_on_bypass info:\n\r");
    for(i=0;i<8;i++)
    {
	    printf("\tgroup-%d : %s\n\r",i,((onbypass>>i)&0x01)?"on":"off");
    }
    
    addr = 0x05;
    i2c_read(0x57, addr, 1, &offbypass, 1);
	printf("\npower_off_bypass info:\n\r");
    for(i=0;i<8;i++)
    {
	    printf("\tgroup-%d : %s\n\r",i+1,((offbypass>>i)&0x01)?"on":"off");
    }
#else
    printf("\nbypass_info not support!\n\r");
#endif
    return 0;
}

static int wdt_bypass(int argc,char **argv)
{
	uint addr=0x23,addr1=0x07;
	u8 buffer[1];
	unsigned char group;
	if(argc<2) 
		return -1;
	group=strtoul(argv[1],0,0);
	group = (1<<(group-1));
	printf("group=%d\n", group);
	buffer[0]=group;


	ls2k_i2c_init(0, 0x800000001fe21800);
	i2c_write_bypass( addr, buffer);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1,buffer);
#endif	
	return 0;
}


static int wdt_need_bypass()
{
	uint addr=0x35,addr1=0x0b;
	u8 buffer[]={0x5e};

	ls2k_i2c_init(0, 0x800000001fe21800);
	i2c_write_bypass( addr, buffer);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1,buffer);
#endif	
	return 0;
}
static int wdt_need_reboot()
{
	uint addr=0x25,addr1=0x09;
	u8 buffer[]={0xaa};

	ls2k_i2c_init(0, 0x800000001fe21800);
	i2c_write_bypass( addr, buffer);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1,buffer);
#endif
	return 0;
}
static int wdt_bypass_time(int argc,char **argv) 
{
	uint addr=0x35,addr1=0x0b;
	u8 buffer[1];
	unsigned int timeout;
	if(argc<2) 
		return -1;
	timeout=strtoul(argv[1],0,0);
	printf("timeout =%d\n", timeout);
	if( timeout > 256 || timeout <0)
		return -1;
	buffer[0]=timeout;


	ls2k_i2c_init(0, 0x800000001fe21800);
	i2c_write_bypass( addr, buffer);
#ifdef EEPROM_BYPASS
	i2c_write_eeprom(addr1,buffer);
#endif	
	return 0;
}

static int read_bypass(int argc,char **argv)
{
#ifdef EEPROM_BYPASS
	u8 date;
	u8 addr;
	if(argc<2) 
		return -1;
	addr=strtoul(argv[1],0,0);
	ls2k_i2c_init(0, 0x800000001fe21800);
    i2c_read(0x57, addr, 1, &date, 1);
	printf("addr[0x%x] = 0x%x\n",addr,date);
#else
    printf("\nread_bypass not support!\n\r");
#endif
	return 0;
}

static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"post_on","bus chip [alen]", 0, "test i2c", post_on, 0, 99, CMD_REPEAT},
	{"go_s5","bus chip [alen]", 0, "test i2c", go_s5, 0, 99, CMD_REPEAT},
	{"power_on_bypass"," group num", 0, "test i2c", power_on_bypass, 0, 99, CMD_REPEAT},
	{"power_off_bypass","group num", 0, "test i2c", power_off_bypass, 0, 99, CMD_REPEAT},
	{"wdt_bypass_setting","group num", 0, "test i2c", wdt_bypass, 0, 99, CMD_REPEAT},
	{"read_bypass","read_bypass addr", 0, "test i2c", read_bypass, 0, 99, CMD_REPEAT},
	{"bypass_info","bypass_info", 0, "test i2c", bypass_info, 0, 99, CMD_REPEAT},
	{"wdt_need_bypass","no param", 0, "test i2c", wdt_need_bypass, 0, 99, CMD_REPEAT},
	{"wdt_need_reboot","no param", 0, "test i2c", wdt_need_reboot, 0, 99, CMD_REPEAT},
	{"wdt_bypass_time","timeout 0<timeout<256", 0, "test i2c", wdt_bypass_time, 0, 99, CMD_REPEAT},
	{0, 0}
};


static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
     cmdlist_expand(Cmds, 1);
}

