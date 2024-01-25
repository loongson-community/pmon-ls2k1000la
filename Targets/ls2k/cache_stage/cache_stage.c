#include <sys/types.h>
#include "target/ls2k1000.h"
#include "../../../pmon/arch/loongarch/early_printf.c"

void cache_main(unsigned long long msize)
{
	init_loongarch(msize);
}
