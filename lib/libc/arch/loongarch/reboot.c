#include <cpu.h>
void
tgt_reboot()
{

	void (*longreach) (void);
	
	longreach = (void *)PHYS_TO_UNCACHED(0x1c000000);
	(*longreach)();

	while(1);

}
