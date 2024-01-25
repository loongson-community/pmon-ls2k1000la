/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Loongson Technology Corporation Limited
 */

#ifndef _TABLES_CSUM_H_
#define _TABLES_CSUM_H_

unsigned char checksum8(char *buff, int size)
{
	int sum, cnt;

	for (sum = 0, cnt = 0; cnt < size; cnt++) {
		sum = (char) (sum + *(buff + cnt));
	}

	return (char)(0x100 - sum);
}

#endif
