/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

#ifndef _CPUMP_H
#define _CPUMP_H

#include <linux/libata.h>
#include "../../../../pmon/arch/loongarch/loongarchregs.h"

typedef volatile u32		SPIN_LOCK;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL  ((void *) 0)
#endif


#define CORES_PER_NODE		4
#define MAX_PROCESSOR_NUMBER			(TOT_NODE_NUM * CORES_PER_NODE)
#define APINIT_TIME_OUT				100000
#define MAX_UINT64				((u64)0xFFFFFFFFFFFFFFFF)

#define WAKEUP_AP_SIGNAL (SIGNATURE_32('S', 'T', 'A', 'P'))


#define SPIN_LOCK_RELEASED			((u32) 1)
#define SPIN_LOCK_ACQUIRED			((u32) 2)

#define ASSERT(expression) \
	do {	\
		if (!(expression)) { \
			(void)printf("Assertion %s failed: FILE %s LINE %d\n", #expression, __FILE__, __LINE__); \
		} \
	} while (0)

#define PROCESSOR_AS_BSP_BIT		0x00000001
#define PROCESSOR_ENABLED_BIT		0x00000002
#define PROCESSOR_HEALTH_STATUS_BIT	0x00000004

//
// AP loop state when APs are in idle state
// It's value is the same with PcdCpuApLoopMode
//
typedef enum {
	AP_IN_HLT_LOOP		= 1,
	AP_IN_MWAIT_LOOP	= 2,
	AP_IN_RUN_LOOP		= 3
} ap_loop_mode;

//
// AP initialization state during APs wakeup
//
typedef enum {
	AP_INIT_CONFIG		= 1,
	AP_INIT_RECONFIG	= 2,
	AP_INIT_DONE		= 3
} ap_init_state;

//
// AP state
//
typedef enum {
	CPU_STATE_IDLE,
	CPU_STATE_READY,
	CPU_STATE_BUSY,
	CPU_STATE_FINISHED,
	CPU_STATE_DISABLED
} cpu_state;

//
// AP related data
//
struct cpu_ap_data {
	volatile u32		ap_lock;
	u8					cpu_healthy;
	int					state;
	u8					waiting;
	u8					*finished;
	u64					expected_time;
	u64					current_time;
	u64					total_time;
};

struct cpu_info {
	u32					apic_id;
	u32					health;
} __attribute__((packed));


//
// CPU MP Data save in memory
//
struct cpu_mp_data {
	u32					cpu_count;
	u32					bsp_number;
	u32					mp_lock;
	volatile u32		finished_count;
	u8					*finished;
	u64					expected_time;
	u64					current_time;
	u64					total_time;
	struct cpu_ap_data	*cpu_ap_data;
	struct cpu_info		*cpu_info_in;
};

struct processor_info {
	u32 p_id;
	u32 flag;
};
extern u64 get_performance_counter_properties (u64 *start_value, u64 *end_value);

extern volatile unsigned int pcd_spin_lock_timeout;
extern void *interlocked_compare_exchange_pointer (void * volatile *value, void *compare_value, void *exchange_value);

u64 div_u64x64_remainder(u64 dividend, u64 divisor, u64 *remainder);
u64 div_u64x32(u64 dividend, u32 divisor);
u64 mult_u64x64(u64 multiplicand, u64 multiplier);
u64 mult_u64x32(u64 multiplicand, u32 multiplier);
void CPU_PAUSE(void);
extern void asm_wait_for_kernel(void);
struct cpu_mp_data *get_cpu_mp_data (void);
void delay1(int);
int get_proc_num(u32 *enbled);
int get_processor_info (unsigned int tmp_pnum, struct processor_info *p_info);
#endif
