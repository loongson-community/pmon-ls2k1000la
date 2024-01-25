#include <linux/libata.h>
#include "cpump.h"

u64 math_mult_u64x64(u64 multiplicand, u64 multiplier)
{
	return multiplicand * multiplier;
}

u64 math_mult_u64x32(u64 multiplicand, u32 multiplier)
{
	return multiplicand * multiplier;
}

u64 math_div_u64x32(u64 dividend, u32 divisor)
{
	return dividend / divisor;
}

u64 div_u64x32(u64 dividend, u32 divisor)
{
	return math_div_u64x32 (dividend, divisor);
}

u64 mult_u64x32(u64 multiplicand, u32 multiplier)
{
	u64 result;
	result = math_mult_u64x32 (multiplicand, multiplier);

	return result;
}

u64 mult_u64x64(u64 multiplicand, u64 multiplier)
{
	u64 result;
	result = math_mult_u64x64(multiplicand, multiplier);

	return result;
}

u64 internal_math_div_rem_u64x64(u64 dividend, u64 divisor, u64 *remainder)
{
	if (remainder != NULL) {
		*remainder = dividend % divisor;
	}
	return dividend / divisor;
}

u64 div_u64x64_remainder(u64 dividend, u64 divisor, u64 *remainder)
{
	return internal_math_div_rem_u64x64 (dividend, divisor, remainder);
}


u32 internal_sync_compare_exchange_32(volatile u32 *value,
		u32 compare_value, u32 exchange_value)
{
	return *value != compare_value ? *value : \
					 ((*value = exchange_value), compare_value);
}

u32 internal_sync_compare_exchange_64(volatile u64 *value,
		u64 compare_value, u64 exchange_value)
{
	return *value != compare_value ? *value : \
					 ((*value = exchange_value), compare_value);
}


u32 interlocked_compare_exchange_32(volatile u32 *value,
		u32 compare_value, u32 exchange_value)
{
	return internal_sync_compare_exchange_32 (value,
			compare_value, exchange_value);
}
u64 interlocked_compare_exchange_64(volatile u64 *value,
		u64 compare_value, u64 exchange_value)
{
	return internal_sync_compare_exchange_64 (value,
			compare_value, exchange_value);
}


void *locked_compare(void * volatile *value, void *compare_value, void *exchange_value)
{
	unsigned char size_of_value;

	size_of_value = sizeof (*value);

	switch (size_of_value) {
	case sizeof (u32):
		return (void *)(u64)interlocked_compare_exchange_32 (
				(volatile u32 *)value,
				(u32)(u64)compare_value,
				(u32)(u64)exchange_value
				);
	case sizeof (u64):
		return (void *)(u64)interlocked_compare_exchange_64 (
				(volatile u64 *)value,
				(u64)(u64)compare_value,
				(u64)(u64)exchange_value
				);
	default:
		return NULL;
	}
}
