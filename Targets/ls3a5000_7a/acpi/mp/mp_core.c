#include <string.h>
#include "cpump.h"

extern void *pmalloc __P((size_t));
extern void pfree(void *ap);
extern void bzero(void *dst, size_t length);
extern volatile unsigned int *initialize_spinlock (volatile unsigned int *spinlock);
extern void set_ap_state (struct cpu_ap_data *cpu_data, cpu_state state);

volatile unsigned int spin_lock_timeout	= 10000000;
unsigned int m_number_of_processors		= 1;
struct cpu_mp_data	*m_cpu_mp_data = NULL;

/*
 *
 *Get the pointer to CPU MP Data structure.
 *@return  The pointer to CPU MP Data structure.
 *
 */
struct cpu_mp_data *get_cpu_mp_data (void)
{
	return m_cpu_mp_data;
}
/*
 *Initialize CPU AP Data when AP is wakeup at the first time.
 *
 *@param[in, out] cpu_mp_data Pointer to PEI CPU MP Data
 *@param[in]	processor_number The handle number of processor
 *@param[in]	bist_data Processor BIST data
 *
 */
void initialize_ap_data (struct cpu_mp_data *mp_data, unsigned int processor_number, unsigned int bist_data)
{
	struct cpu_info *cpu_info_in = mp_data->cpu_info_in;

	cpu_info_in[processor_number].apic_id	= get_apic_id();
	cpu_info_in[processor_number].health	= bist_data;

	mp_data->cpu_ap_data[processor_number].waiting		= FALSE;
	mp_data->cpu_ap_data[processor_number].cpu_healthy	= (bist_data == 0) ? TRUE : FALSE;

	initialize_spinlock(&mp_data->cpu_ap_data[processor_number].ap_lock);
	set_ap_state (&mp_data->cpu_ap_data[processor_number], CPU_STATE_IDLE);
}

void mp_init(void)
{
	struct cpu_mp_data		*mp_data;
	struct cpu_info			*cpu_info_in;
	void					*mp_buffer;

	unsigned int			max_lp_n = MAX_PROCESSOR_NUMBER;
	unsigned int			buffersize;
	unsigned int			index;

	buffersize  = sizeof (struct cpu_mp_data);
	buffersize += (sizeof (struct cpu_ap_data) + sizeof (struct cpu_info)) * max_lp_n;
	mp_buffer   = pmalloc(buffersize);
	if (!mp_buffer) {
		tgt_printf("malloc failed, address= 0x%x\n", mp_buffer);
		return -1;
	}

	mp_data = (struct cpu_mp_data *)mp_buffer;

	m_cpu_mp_data = mp_data;

	mp_data->cpu_count		= 1;
	mp_data->bsp_number		= BOOTCORE_ID;
	mp_data->cpu_ap_data	= (struct cpu_ap_data *) (mp_data + 1);
	mp_data->cpu_info_in	= (mp_data->cpu_ap_data + max_lp_n);

	initialize_spinlock(&mp_data->mp_lock);
	//
	// Set BSP basic information
	//
	initialize_ap_data (mp_data, BOOTCORE_ID, 0);

	for (index = 0; index < TOT_NODE_NUM * CORES_PER_NODE; index++) {
		if (index == BOOTCORE_ID)
			continue;
		mp_data->cpu_ap_data[index].state = CPU_STATE_DISABLED;
		mp_data->cpu_ap_data[index].cpu_healthy = FALSE;
	}

	mp_data->total_time		= 0ULL;
	mp_data->current_time	= 0ULL;
	mp_data->expected_time	= 0ULL;

	//
	// Wakeup all APs and calculate the processor count in system
	//
	collect_processor_count (mp_data);

	return 0;
}
