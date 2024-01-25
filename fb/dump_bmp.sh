#!/bin/bash
if [ "$1" = "" ]||[ "$2" = "" ];then
	echo "usage: ./dump_bmp.sh xxx.bmp xxx.h
	dump xxx.bmp to hex, and write into xxx.h"
else
	echo "dump $1 to hex, and write into $2"
	echo "#ifndef __BMP_LOGO_H__
	#define __BMP_LOGO_H__
	unsigned char logo_data[] = {" > $2
	hexdump -e '16/1 "0x%02X, " "\n"' -v $1 >> $2
	echo "};
	#endif" >> $2
	sed -i 's/ 0x  ,//g' $2
fi
