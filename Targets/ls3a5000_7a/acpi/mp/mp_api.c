#include <include/stdio.h>
#include <timer.h>
#include "cpump.h"
#include <regdef.h>
#include "cpu.h"

int get_apic_id(void)
{
	return read_apic_id() & 0x3ff;
}
/**
 *Get the Application Processors state.
 *@param[in]  cput_ap_datf The pointer to struct cpu_ap_data of specified AP
 *@return  The AP status
 **/
int get_ap_state (struct cpu_ap_data *cpu_data)
{
	return cpu_data->state;
}

/**
 *This function will be called from AP reset code if BSP uses wake_up_ap.

*@param[in] ExchangeInfo Pointer to the MP exchange info buffer*@param[in] Apindex Number of current executing AP
 *@param[in] Apindex Number of current executing AP
 */
void ap_wakeup_function (unsigned int ap_index, struct cpu_mp_data *cpu_mp_data)
{
	unsigned int tmp;

	interlocked_increment ((unsigned int *)&cpu_mp_data->cpu_count);
	tmp = ap_index;

	initialize_ap_data (cpu_mp_data, tmp, 0);

	interlocked_increment ((unsigned int *) &cpu_mp_data->finished_count);
	asm_wait_for_kernel();
}


void wakeup_ap (unsigned long func, void *option)
{
	/*A1_OFF*/
	*(volatile unsigned long *)PHYS_TO_UNCACHED(0x1fe01038) = (u64)option;
	/*FN_OFF*/
	*(volatile unsigned long *)PHYS_TO_UNCACHED(0x1fe01020) = func;
}


/*
 *Calculate timeout value and return the current performance counter value.

 *Calculate the number of performance counter ticks required for a timeout.
 *If timeout_microseconds is 0, return value is also 0, which is recognized
 *as infinity.

 *@param[in]  timeout_microseconds timeout value in microseconds.
 *@param[out] current_time Returns the current value of the performance counter.

 *@return Expected time stamp counter for timeout.
 *If timeout_microseconds is 0, return value is also 0, which is recognized
 *as infinity.

 */
u64 calculate_timeout(unsigned int timeout_microseconds, u64 *current_time)
{
	u64 timeout_in_seconds = 0ULL;
	u64 timestamp_counter_freq = 0ULL;

	//
	// Read the current value of the performance counter
	//
	*current_time = CPU_GetCOUNT();
	//
	// If timeout_microseconds is 0, return value is also 0, which is recognized
	// as infinity.
	//
	if (timeout_microseconds == 0) {
		return 0;
	}

	//
	// get_performance_freq () returns the timestamp counter's frequency
	// in Hz.
	//
	timestamp_counter_freq = get_performance_freq(NULL, NULL);

	//
	// Check the potential overflow before calculate the number of ticks for the timeout value.
	//
	if (div_u64x32 (MAX_UINT64, timeout_microseconds) < timestamp_counter_freq) {
		//
		// Convert microseconds into seconds if direct multiplication overflows
		//
		timeout_in_seconds =  (timeout_microseconds / 1000000);
		//
		// Assertion if the final tick count exceeds MAX_UINT64
		//
		ASSERT (div_u64x32 (MAX_UINT64, timeout_in_seconds) >= timestamp_counter_freq);
		return mult_u64x64 (timestamp_counter_freq, timeout_in_seconds);
	} else {
		//
		// No overflow case, multiply the return value with timeout_microseconds and then divide
		// it by 1,000,000, to get the number of ticks for the timeout value.
		//
		return div_u64x32 (
				mult_u64x32 (
					timestamp_counter_freq,
					timeout_microseconds
					),
				1000000
				);
	}
}

/*
 *Checks whether timeout expires.

 *Check whether the number of elapsed performance counter ticks required for
 *a timeout condition has been reached.
 *If timeout is zero, which means infinity, return value is always FALSE.

 *@param[in, out] previous_time On input, the value of the performance counter
 *when it was last read.
 *On output, the current value of the performance
 *counter
 *@param[in] total_time The total amount of elapsed time in performance
 *counter ticks.
 *@param[in] timeout The number of performance counter ticks required
 *to reach a timeout condition.

 *@retval TRUE A timeout condition has been reached.
 *@retval FALSE A timeout condition has not been reached.

 **/
unsigned char check_timeout(u64 *previous_time, u64 *total_time, u64 timeout)
{
	u64 start;
	u64 end;
	u64 current_time;
	long delta;
	long cycle;

	if (timeout == 0) {
		return FALSE;
	}
	get_performance_freq(&start, &end);
	cycle = end - start;
	if (cycle < 0) {
		cycle = -cycle;
	}
	cycle++;
	current_time = CPU_GetCOUNT();
	delta = (long) (current_time - *previous_time);
	if (start > end) {
		delta = -delta;
	}
	if (delta < 0) {
		delta += cycle;
	}
	*total_time += delta;
	*previous_time = current_time;
	if (*total_time > timeout) {
		return TRUE;
	}
	return FALSE;
}
/*
 *Helper function that waits until the finished AP count reaches the specified
 *limit, or the specified timeout elapses (whichever comes first).

 *@param[in] cpu_mp_data Pointer to CPU MP Data.
 *@param[in] finished_ap_limit The number of finished APs to wait for.
 *@param[in] time_limit The number of microseconds to wait for.
 */
void timed_wait_for_ap_finish(struct cpu_mp_data *cpu_mp_data, unsigned int finished_ap_limit, unsigned int time_limit)
{
	//
	// calculate_timeout() and check_timeout() consider a time_limit of 0
	// "infinity", so check for (time_limit == 0) explicitly.
	//
	if (time_limit == 0) {
		return;
	}

	cpu_mp_data->expected_time = calculate_timeout (
			time_limit,
			&cpu_mp_data->current_time
			);
	while (cpu_mp_data->finished_count < finished_ap_limit &&
			!check_timeout (
				&cpu_mp_data->current_time,
				&cpu_mp_data->total_time,
				cpu_mp_data->expected_time
				)) {
		CPU_PAUSE ();
	}

	if (cpu_mp_data->finished_count >= finished_ap_limit) {
		tgt_printf("Reached finished_ap_limit=%llx in %llx microseconds\n",
				finished_ap_limit,
				div_u64x64_remainder (mult_u64x32 (cpu_mp_data->total_time, 1000000),
					get_performance_freq(NULL, NULL),
					NULL
					)
			  );
	}
}

/*
 *@param[in] cpu_mp_data Pointer to CPU MP Data
 * @param[in] Broadcast TRUE: Send broadcast IPI to all APs
 *FALSE: Send IPI to AP by apic_id
 *@param[in] core_num The handle number of specified processor
 *@param[in] Procedure  The function to be invoked by AP
 *@param[in] ProcedureArgument The argument to be passed into AP function
 */
void wake_up_ap(struct cpu_mp_data *cpu_mp_data)
{

	cpu_mp_data->finished_count = 0;

	wakeup_ap(ap_wakeup_function, cpu_mp_data);
	timed_wait_for_ap_finish (
			cpu_mp_data,
			MAX_PROCESSOR_NUMBER - 1,
			APINIT_TIME_OUT
			);
}

/**
 *This function will get CPU count in the system.

 *@param[in] cpu_mp_data Pointer to PEI CPU MP Data

 *@return  CPU count detected
 */
unsigned int collect_processor_count(struct cpu_mp_data *cpu_mp_data)
{
	//
	// Send 1st broadcast IPI to APs to wakeup APs
	// write core0 mailbox with initialization function address
	wake_up_ap(cpu_mp_data);
	ASSERT (cpu_mp_data->cpu_count <= (MAX_PROCESSOR_NUMBER));

	// Wait for all APs finished the initialization
	//
	while (cpu_mp_data->finished_count < (cpu_mp_data->cpu_count - 1))
		CPU_PAUSE ();

	tgt_printf("BootCore ID: %d Collect AP: %d Total Core: %d\n", \
			BOOTCORE_ID, cpu_mp_data->finished_count, cpu_mp_data->cpu_count);
	return cpu_mp_data->cpu_count;
}
/*
 *Retrieves the number of logical processor in the platform and the number of
 *those logical processors that are enabled on this boot. This service may only
 *be called from the BSP.
 */
int get_proc_num(unsigned int *enabled)
{
	struct cpu_mp_data *cpu_mp_data;
	unsigned int index;
	unsigned int tmp = 0;

	if (enabled == NULL)
		return 1;

	cpu_mp_data = get_cpu_mp_data();
	if (cpu_mp_data == NULL)
		return 1;

	if (get_apic_id() != cpu_mp_data->bsp_number) {
		return 1;
	}
	for (index = 0; index < TOT_NODE_NUM * CORES_PER_NODE; index++) {
		if (get_ap_state (&cpu_mp_data->cpu_ap_data[index]) != CPU_STATE_DISABLED) {
			tmp++;
		}
	}

	*enabled = tmp;
	return 0;
}

int get_processor_info(unsigned int core_num, struct processor_info *p_info)
{
	struct cpu_mp_data *cpu_mp_data;
	struct cpu_info *cpu_info_in;
	unsigned int caller;

	cpu_mp_data = get_cpu_mp_data();
	if (cpu_mp_data == NULL)
		return 1;
	cpu_info_in = cpu_mp_data->cpu_info_in;

	if (get_apic_id() != cpu_mp_data->bsp_number)
		return 1;

	if (p_info == NULL)
		return 1;

	p_info->p_id = cpu_info_in[core_num].apic_id;
	p_info->flag  = 0;
	if (core_num == cpu_mp_data->bsp_number)
		p_info->flag |= PROCESSOR_AS_BSP_BIT;

	if (cpu_mp_data->cpu_ap_data[core_num].cpu_healthy)
		p_info->flag |= PROCESSOR_HEALTH_STATUS_BIT;

	if (get_ap_state (&cpu_mp_data->cpu_ap_data[core_num]) == CPU_STATE_DISABLED) {
		p_info->flag &= ~PROCESSOR_ENABLED_BIT;
	} else {
		p_info->flag |= PROCESSOR_ENABLED_BIT;
	}

	return 0;
}
