#include <sys/types.h>

#define max(x,y) ({			\
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })

extern void tgt_putchar (int);
extern uint8_t tgt_getchar (void);
static uint32_t strlen(const char *p)
{
	int n;

	if (!p)
		return (0);
	for (n = 0; *p; p++)
		n++;
	return (n);
}

static char * strcpy (char *dstp, const char *srcp)
{
	char *dp = dstp;

	if (!dstp)
		return (0);
	*dp = 0;
	if (!srcp)
		return (dstp);

	while ((*dp++ = *srcp++) != 0);
	return (dstp);
}

int printbase(long v, int w, int base, int sign)
{
	int i, j;
	int c;
	char buf[64];
	unsigned long value;
	if (sign && v < 0) {
		value = -v;
		tgt_putchar('-');
	} else {
		value = v;
	}
	for (i = 0; value; i++) {
		buf[i] = value % base;
		value = value / base;
	}
	for (j = max(w, i); j > 0; j--) {
		c = j > i ? 0 : buf[j - 1];
		tgt_putchar((c <= 9) ? c + '0' : c - 0xa + 'a');
	}
	return 0;
}

int puts(char *s)
{
	char c;
	while ((c = *s))
	{
		if (c == '\n')
			tgt_putchar('\r');
		tgt_putchar(c);
		s++;
	}
	return 0;
}

int early_printf(const char *fmt, ...)
{
	int i, longlong = 0;
	char c;
	void **arg;
	void *ap;
	int w;
	__builtin_va_start(ap, fmt);
	arg = ap;
	for (i = 0; fmt[i]; i++) {
		c = fmt[i];
		if (c == '%') {
			w = 1;
again:
			switch(fmt[i + 1]) {
			case 'l':
				i++;
				if (sizeof(long) == sizeof(long long))
					longlong = 1;
				if (fmt[i + 1] == 'l') {
					longlong = 1;
					i++;
				}
				goto again;
				break;
			case 's':
				puts(*arg);
				arg++;
				i++;
				break;
			case 'c':
				tgt_putchar((long)*arg);
				arg++;
				i++;
				break;
			case 'u':
				printbase((long)*arg, w, 10, 0);
				arg++;
				i++;
				break;
			case 'd':
				printbase((long)*arg, w, 10, 1);
				arg++;
				i++;
				break;
			case 'o':
				printbase((long)*arg, w, 8, 0);
				arg++;
				i++;
				break;
			case 'b':
				printbase((long)*arg, w, 2, 0);
				arg++;
				i++;
				break;
			case 'p':
			case 'x':
			case 'X':
				if (longlong)
					printbase((long long)*arg, w, 16, 0);
				else
					printbase((long)*arg, w, 16, 0);
				arg++;
				i++;
				break;
			case '%':
				tgt_putchar('%');
				i++;
				break;
			case '0':
				i++;
				for (w = 0; fmt[i + 1] > '0' && fmt[i + 1] <= '9'; i++)
					w = w * 10 + (fmt[i + 1] - '0');
				goto again;
				break;
			case '1' ... '9':
				for (w = 0; fmt[i + 1] > '0' && fmt[i + 1] <= '9'; i++)
					w = w * 10 + (fmt[i + 1] - '0');
				goto again;
				break;
			default:
				tgt_putchar('%');
				break;
			}
		} else {
			if (c == '\n')
				tgt_putchar('\r');
			tgt_putchar(c);
		}
	}
	return 0;
}

#define  pr_info(fmt, args...) early_printf(fmt, ##args)

char* early_puts(char *dst, char *s)
{
	int32_t len = strlen(s);
	strcpy(dst, s);
	return dst + len;
}

char* early_putchar(char *dst, char chr)
{
	dst[0] = chr;
	return ++dst;
}

char* early_printbase(char *dst, long v, int w, int base, int sign)
{
	int i, j;
	int c;
	char buf[64];
	unsigned long value;
	if (sign && v < 0) {
		value = -v;
		dst = early_putchar(dst, '-');
	} else {
		value = v;
	}
	for (i = 0; value; i++) {
		buf[i] = value % base;
		value = value / base;
	}
	for (j = max(w, i); j > 0; j--) {
		c = j > i ? 0 : buf[j - 1];
		dst = early_putchar(dst, (c <= 9) ? c + '0' : c - 0xa + 'a');
	}
	return dst;
}
#if 0
int early_vsprintf(char *str, const char *format, __builtin_va_list ap)
{
	int i, longlong = 0;
	char c;
	int w;
	char *str_old;
	str_old = str;
	for(i = 0; format[i]; i++) {
		c = format[i];
		if(c == '%') {
			w = 1;
again:
			switch(format[i + 1]) {
			case 'l':
				i++;
				longlong = 1;
				goto again;
				break;
			case 's':
				str = early_puts(str, __builtin_va_arg(ap, char*));
				i++;
				break;
			case 'c':
				str = early_putchar(str, (char)__builtin_va_arg(ap, int));
				i++;
				break;
			case 'u':
				str = early_printbase(str, __builtin_va_arg(ap, long), w, 10, 0);
				i++;
				break;
			case 'd':
				str = early_printbase(str, __builtin_va_arg(ap, long), w, 10, 1);
				i++;
				break;
			case 'o':
				str = early_printbase(str, __builtin_va_arg(ap, long), w, 8, 0);
				i++;
				break;
			case 'b':
				str = early_printbase(str, __builtin_va_arg(ap, long), w, 2, 0);
				i++;
				break;
			case 'p':
			case 'x':
			case 'X':
				if(longlong)
					str = early_printbase(str, __builtin_va_arg(ap, long long), w, 16, 0);
				else
					str = early_printbase(str, __builtin_va_arg(ap, long), w, 16, 0);
				i++;
				break;
			case '%':
				str = early_putchar(str, '%');
				i++;
				break;
			case '0':
				i++;
				for(w = 0; format[i + 1] > '0' && format[i + 1] <= '9'; i++)
					w = w * 10 + (format[i + 1] - '0');
				goto again;
				break;
			case '1' ... '9':
				for(w = 0;format[i + 1] > '0' && format[i + 1] <= '9';i++)
					w = w * 10 + (format[i + 1] -'0');
				goto again;
				break;
			default:
				str = early_putchar(str, '%');
				break;
			}

		} else {
			if(c == '\n')
				str = early_putchar(str, '\r');
			str = early_putchar(str, c);
		}
	}
	str = early_putchar(str, '\0');
	return (int)(str - str_old);
}
#endif

uint64_t get_hex(void)
{
	uint8_t c, cnt = 16;
	uint64_t val = 0;
	while (1) {
		c = tgt_getchar();

		switch (c) {
		case 'q':
		case ' ':
		case '\r':
			/* abort in advance */
			return val;
		case '\b':
			if(cnt < 16) {
				cnt++;
				val = val >> 4;
				tgt_putchar('\b');
				tgt_putchar(' ');
				tgt_putchar('\b');
			}
		default:
			if (!cnt)
				continue;
			if (c >= '0' && c <= '9') {
				val = val << 4 | (c - '0');
			} else if (c >= 'a' && c <= 'f') {
				val = val << 4 | (c - 'a' + 10);
			} else if (c >= 'A' && c <= 'F') {
				val = val << 4 | (c - 'A' + 10);
			} else {
				continue;
			}
			tgt_putchar(c);
			cnt--;
		}
	}
}
