#include <regdef.h>

/*
 *Requests CPU to pause for a short period of time.

 *Requests CPU to pause for a short period of time. Typically used in MP
 *systems to prevent memory starvation while waiting for a spin lock.
 */
	.globl CPU_PAUSE
CPU_PAUSE:
	dbar 0
	jr ra
//end
