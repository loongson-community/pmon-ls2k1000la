static int cputest()
{
static volatile float x=1.2342374;
static volatile float y=9.153784;
static volatile float z;
printf("cp0 register read test now noting to do\n");
z=x*y*10000;
if((int)z==112979) printf("cpu float calculation test ok\n");
else printf("cpu float calculation test error\n");
return 0;
}
