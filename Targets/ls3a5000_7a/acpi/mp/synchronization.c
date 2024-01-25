#include <timer.h>
#include "cpump.h"
#include <asm.h>
#include <regnum.h>
#include <pte.h>

extern volatile unsigned int spin_lock_timeout;

#define barrier() do {__asm__ __volatile__ ("dbar 0" ::: "memory"); } while (0)

u64 stable_timer_freq;

unsigned char acq_spinlock(SPIN_LOCK *spinlock)
{
	SPIN_LOCK lock_value;

	ASSERT (spinlock != NULL);

	lock_value = *spinlock;
	ASSERT (lock_value == SPIN_LOCK_ACQUIRED || lock_value == SPIN_LOCK_RELEASED);

	return (unsigned char)(
			locked_compare(
				(void **)spinlock,
				(void *)SPIN_LOCK_RELEASED,
				(void *)SPIN_LOCK_ACQUIRED) == (void *)SPIN_LOCK_RELEASED
			);
}

SPIN_LOCK *acquire_spinlock(SPIN_LOCK *spinlock)
{
	unsigned long long current = 0ULL;
	unsigned long long previous = 0ULL;
	unsigned long long total;
	unsigned long long start;
	unsigned long long end;
	unsigned long long timeout;
	long cycle = 0LL;
	long delta = 0LL;

	if (spin_lock_timeout == 0) {
		while (!acq_spinlock(spinlock)) {
			CPU_PAUSE();
		}
	} else if (!acq_spinlock(spinlock)) {
		//
		// Get the current timer value
		//
		current = CPU_GetCOUNT();

		//
		// Initialize local variables
		//
		start = 0ULL;
		end   = 0ULL;
		total = 0ULL;

		/*
		 *Retrieve the performance counter properties and compute the number of performance
		 * counter ticks required to reach the timeout
		 */
		timeout = div_u64x32(mult_u64x32(
					get_performance_freq(&start, &end),
					spin_lock_timeout
					),
				1000000
				);
		cycle = end - start;
		if (cycle < 0) {
			cycle = -cycle;
		}
		cycle++;

		while (!acq_spinlock(spinlock)) {
			CPU_PAUSE ();
			previous = current;
			current  = CPU_GetCOUNT();
			delta = (long) (current - previous);
			if (start > end) {
				delta = -delta;
			}
			if (delta < 0) {
				delta += cycle;
			}
			total += delta;
			ASSERT (total < timeout);
		}
	}
	return spinlock;
}

/*
 *Releases a spin lock.

 *This function places the spin lock specified by spinlock in the release state
 *and returns spinlock.
 *If spinlock is NULL, then ASSERT().
 8If spinlock was not initialized with Initializespinlock(), then ASSERT().

 *@param  spinlock  A pointer to the spin lock to release.
 *@return spinlock released lock.

 */
SPIN_LOCK *release_spinlock(SPIN_LOCK *spinlock)
{
	SPIN_LOCK lock_value;

	ASSERT(spinlock != NULL);

	lock_value = *spinlock;
	ASSERT(lock_value == SPIN_LOCK_ACQUIRED || lock_value == SPIN_LOCK_RELEASED);

	*spinlock = SPIN_LOCK_RELEASED;
	return spinlock;
}



/*
 *Set the Application Processors state.
 *@param[in]	cpu_data The pointer to struct cpu_ap_data of specified AP
 *@param[in]	State The AP status
 */
void set_ap_state(struct cpu_ap_data *cpu_data, cpu_state state)
{
	acquire_spinlock (&cpu_data->ap_lock);
	cpu_data->state = state;
	release_spinlock (&cpu_data->ap_lock);
}


SPIN_LOCK *initialize_spinlock(SPIN_LOCK *spinlock)
{
	ASSERT (spinlock != NULL);
	barrier();
	*spinlock = SPIN_LOCK_RELEASED;
	barrier();
	return spinlock;
}
/**
 *Performs an atomic increment of an 32-bit unsigned integer.

 *Performs an atomic increment of the 32-bit unsigned integer specified by
 *Value and returns the incremented value. The increment operation must be
 *performed using MP safe mechanisms. The state of the return value is not
 *guaranteed to be MP safe.

 *If Value is NULL, then ASSERT().

 *@param  Value A pointer to the 32-bit value to increment.

 *@return The incremented value.

 */
unsigned int interlocked_increment(volatile unsigned int *value)
{
	int temp;
	unsigned int result = *value;

	barrier();
	__asm__ __volatile__(
		"amadd.w %1, %2, %0\n"
		: "+ZB"(*value), "=&r" (result)
		: "r" (1)
		: "memory");

	return *value;
}
