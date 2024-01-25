/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */
#ifndef _TIMER_H
#define _TIMER_H

#include <include/stdio.h>
#include <linux/libata.h>

#define LOONGSON_CPUCFG_CONFIG_FIELD2 0x2
#define LOONGSON_CPUCFG_CONFIG_FIELD4 0x4
#define LOONGSON_CPUCFG_CONFIG_FIELD5 0x5

#define  BIT48 0x0001000000000000ULL

extern u64 stable_timer_freq;
#ifndef HZ
#define HZ 100
#endif

#define STABLE_TIMER_FREQ	(100 * 1024 * 1024 * HZ)

/*
 *Retrieves the 64-bit frequency in Hz and the range of performance counter
 *values.

 *If start_value is not NULL, then the value that the performance counter starts
 *with immediately after is it rolls over is returned in start_value. If
 *end_value is not NULL, then the value that the performance counter end with
 *immediately before it rolls over is returned in end_value. The 64-bit
 *frequency of the performance counter in Hz is always returned. If start_value
 *is less than end_value, then the performance counter counts up. If start_value
 *is greater than end_value, then the performance counter counts down. For
 *example, a 64-bit free running counter that counts up would have a start_value
 *of 0 and an end_value of 0xFFFFFFFFFFFFFFFF. A 24-bit free running counter
 *that counts down would have a start_value of 0xFFFFFF and an end_value of 0.

 *@param start_value The value the performance counter starts with when it
 *rolls over.
 *@param end_value The value that the performance counter ends with before
 *it rolls over.

 *@return The frequency in Hz.
 */
static inline u64 get_performance_freq(u64 *start_value, u64 *end_value)
{
	if (start_value != NULL) {
		*start_value = 0;
	}

	if (end_value != NULL) {
		*end_value = BIT48 - 1;
	}

	return STABLE_TIMER_FREQ;
}

#endif
