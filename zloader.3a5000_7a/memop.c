void *memcpy(void *s1, const void *s2, size_t n)
{
	const char *f = s2;
	char *t = s1;

	while (n-- > 0)
		*t++ = *f++;

	return s1;
}

void * memset(void * s,char c, size_t count)
{
	char *xs = (char *) s;
	while (count--)
		*xs++ = c;

	return s;
}
