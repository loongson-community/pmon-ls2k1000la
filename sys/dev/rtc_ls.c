#include <pmon.h>
#include <stdio.h>
#include <time.h>
#include <pio.h>

#define TOY_TRIM_REG		(0x0020)
#define TOY_WRITE0_REG		(0x0024)
#define TOY_WRITE1_REG		(0x0028)
#define TOY_READ0_REG		(0x002c)
#define TOY_READ1_REG		(0x0030)
#define TOY_MATCH0_REG		(0x0034)
#define TOY_MATCH1_REG		(0x0038)
#define TOY_MATCH2_REG		(0x003c)
#define RTC_CTRL_REG		(0x0040)
#define RTC_TRIM_REG		(0x0060)
#define RTC_WRITE0_REG		(0x0064)
#define RTC_READ0_REG		(0x0068)
#define RTC_MATCH0_REG		(0x006c)
#define RTC_MATCH1_REG		(0x0070)
#define RTC_MATCH2_REG		(0x0074)

unsigned long long rtc_base = 0;
extern int ls7a_version(void);

char *tran_month(char *c, char *i)
{
	switch (*c++) {
	case  'J':
		if (*c++ == 'a')	/* Jan */
			i = "01";
		else if (*c++ == 'n')	/* June */
			i = "06";
		else			/* July */
			i = "07";
		break;
	case  'F':			/* Feb */
		i = "02";
		break;
	case  'M':
		c++;
		if (*c++ == 'r')	/* Mar */
			i = "03";
		else			/* May */
			i = "05";
		break;
	case  'A':
		if (*c++ == 'p')	/* Apr */
			i = "04";
		else			/* Aug */
			i = "08";
		break;
	case  'S':			/* Sept */
		i = "09";
		break;
	case  'O':			/* Oct */
		i = "10";
		break;
	case  'N':			/* Nov */
		i = "11";
		break;
	case  'D':			/* Dec */
		i = "12";
		break;
	default  :
		i = NULL;
	}

	return i;
}

int get_update(char *p)
{
	char *t, *mp, m[3];

	t = strstr(vers, ":");
	strncpy(p, t + 26, 4);		/* year */
	p[4] = '-';
	mp = tran_month(t + 6, m);	/* month */
	strncpy(p + 5, mp, 2);
	p[7] = '-';
	strncpy(p + 8, t + 10, 2);	/* day */
	p[10] = '\0';

	return 0;
}

time_t tgt_gettime()
{
	struct tm tm;
	time_t t;
	unsigned int val;

#ifdef HAVE_TOD
	val = inl(rtc_base + TOY_READ0_REG);
	tm.tm_sec = (val >> 4) & 0x3f;
	tm.tm_min = (val >> 10) & 0x3f;
	tm.tm_hour = (val >> 16) & 0x1f;
	tm.tm_mday = (val >> 21) & 0x1f;
	tm.tm_mon = ((val >> 26) & 0x3f) - 1;
	tm.tm_year = inl(rtc_base + TOY_READ1_REG);
	if (tm.tm_year < 50)
		tm.tm_year += 100;
	tm.tm_isdst = tm.tm_gmtoff = 0;
	t = gmmktime(&tm);
#endif
	return (t);
}

/*
 *  Set the current time if a TOD clock is present
 */
void tgt_settime(time_t t)
{
	struct tm *tm;
	unsigned int val;

#ifdef HAVE_TOD
	tm = gmtime(&t);
	val = ((tm->tm_mon + 1) << 26) | (tm->tm_mday << 21) |
		(tm->tm_hour << 16) | (tm->tm_min << 10) |
		(tm->tm_sec << 4);
	outl(rtc_base + TOY_WRITE0_REG, val);
	outl(rtc_base + TOY_WRITE1_REG, tm->tm_year);
#endif
}
int get_rtc_sec()
{
	return (inl(rtc_base + TOY_READ0_REG) >> 4) & 0x3f;
}
void init_ls_rtc(unsigned long long base)
{
	int year, month, date, hour, min, sec, val;
	rtc_base = base;
	if(ls7a_version())
		val = (1 << 11) | (1 << 8);
	else
		val = (1 << 13) | (1 << 11) | (1 << 8);

	outl(rtc_base + RTC_CTRL_REG, val);
	outl(rtc_base + TOY_TRIM_REG, 0);
	outl(rtc_base + RTC_TRIM_REG, 0);

	val = inl(rtc_base + TOY_READ0_REG);

	year = inl(rtc_base + TOY_READ1_REG);
	month = (val >> 26) & 0x3f;
	date = (val >> 21) & 0x1f;
	hour = (val >> 16) & 0x1f;
	min = (val >> 10) & 0x3f;
	sec = (val >> 4) & 0x3f;
	if ((year < 0 || year > 138)
		|| (month < 1 || month > 12)
		|| (date < 1 || date > 31)
		|| (hour > 23) || (min > 59)
		|| (sec > 59)) {

		tgt_printf("RTC time invalid, reset to epoch.\n");
		/* 2000-01-01 00:00:00 */
		val = (1 << 26) | (1 << 21);
		outl(rtc_base + TOY_WRITE1_REG, 0x64);
		outl(rtc_base + TOY_WRITE0_REG, val);
	}
}
