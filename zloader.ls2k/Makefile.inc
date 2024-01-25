try-run = $(shell set -e;		\
	TMP="$(TMPOUT).$$$$.tmp";	\
	if ($(1)) >/dev/null 2>&1;	\
	then echo "$(2)";		\
	else echo "$(3)";		\
	fi;				\
	rm -f "$$TMP")


# cc-option
# Usage: cflags-y += $(call cc-option,-march=winchip-c6,-march=i586)

cc-option = $(call try-run,\
	$(CC) $(1) -c -xc /dev/null -o "$$TMP",$(1),$(2))


ifndef PMONCC
CC=$(CROSS_COMPILE)gcc -mabi=lp64
CPP=$(CROSS_COMPILE)cpp -mabi=lp64
CROSS_COMPILE ?= loongarch64-linux-gnu-
else
CC=${PMONCC}
endif

ifndef S
S:=$(shell cd ../; pwd)
endif

CACHED_MEMORY_ADDR=0x9000000000000000
UNCACHED_MEMORY_ADDR=0x8000000000000000

export CROSS_COMPILE

export CFLAGS_ALIGN:=$(call cc-option,-mstrict-align,)
export CC += ${MYCC}  -mabi=lp64 -march=loongarch64 $(CFLAGS_ALIGN) -fno-delayed-branch  -G 0 -mno-memcpy -fno-builtin -c -fno-pic -DCACHED_MEMORY_ADDR=${CACHED_MEMORY_ADDR} -DUNCACHED_MEMORY_ADDR=${UNCACHED_MEMORY_ADDR}
export LD
export MKDEP=makedepend

LD		= $(CROSS_COMPILE)ld  -m elf64loongarch -G 0 -static -n -nostdlib
OUT_FORMAT="elf64-loongarch"

GZROMSTARTADDR?=${CACHED_MEMORY_ADDR}+0x90000000
ROMSTARTADDR?=${CACHED_MEMORY_ADDR}+0xf010000
RAMSTARTADDR?=${CACHED_MEMORY_ADDR}+0x8000000

gencode=./genrom

all: ${tgt}
ifeq (${TARGET},ls3a5000_7a)
rom: clean acpi ${START} zloader.o
else
rom: clean ${START} zloader.o
endif
	gcc  -I./sys/arch/loongarch/include/ -DSTARTADDR=${GZROMSTARTADDR} -DOUT_FORMAT=\"${OUT_FORMAT}\" -DOUT_ARCH=loongarch -Uloongarch -E -P ld.script.S  > ld.script
	${LD} -T ld.script -e start -o gzrom ${START} zloader.o cache_stage.o ${TARGET_LIB}
	${CROSS_COMPILE}objcopy -O binary gzrom gzrom.bin

${START}:
	rm -f ../Targets/${TARGET}/compile/${TARGETEL}/${START} 
	rm -f ../Targets/${TARGET}/compile/${TARGETEL}/cache_stage.o
	gcc  -DSTARTADDR=${ROMSTARTADDR} -DOUT_FORMAT=\"${OUT_FORMAT}\" -DOUT_ARCH=loongarch -Uloongarch -E -P ld.script.S  > ../Targets/${TARGET}/conf/ld.script
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/
	cp ../Targets/${TARGET}/compile/${TARGETEL}/${START} .
	cp ../Targets/${TARGET}/compile/${TARGETEL}/cache_stage.o .

zloader.o: zloader.c inflate.c  malloc.c  memop.c  pmon.bin.c init_loongarch.c
	$(CC) -c zloader.c ${ZLOADER_OPTIONS} -DMEMSIZE=${MEMSIZE}

init_loongarch.c:  ../Targets/${TARGET}/compile/${TARGETEL}/pmon
	${gencode} $< > init_loongarch.c

pmon.bin.c:  ../Targets/${TARGET}/compile/${TARGETEL}/pmon.bin
	gzip $< -c > pmon.bin.gz
	./bin2c pmon.bin.gz pmon.bin.c biosdata

dtb:
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/ DTB_O=`pwd`/${TARGET}.dtb.i DTB_I=`pwd`/../Targets/${TARGET}/conf/${TARGETEL}.dts dtb
	./dtc -I dts -O dtb -o ${TARGET}.dtb  ${TARGET}.dtb.i
	( echo  "#include <include/load_dtb.h>";echo NVRAM_OFFS; echo DTB_OFFS; )| make -C ../Targets/${TARGET}/compile/${TARGETEL}/ DTB_O=`pwd`/dtbinfo.txt DTB_I=- dtb
	[ -f gzrom.bin ] && cp gzrom.bin gzrom-dtb.bin && python ../tools/pmonenv.py -O $$((`tail -n 1 dtbinfo.txt`)) -o $$((`tail -n 2 dtbinfo.txt|head -n 1`)) -f gzrom-dtb.bin -d ${TARGET}.dtb -w  al=\(usb0,0\)/boot/vmlinuz al1=\(wd0,0\)/boot/vmlinuz append="'console=ttyS0,115200 console=tty initcall_debug=1 loglevel=20'" FR=1
cleanall: clean
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/ clean
clean:
	rm -rf *.o zlib_gzip zloader pmon.bin.c gzrom gzrom.bin gzram  init_loongarch.c pmon.bin.gz zlib_deflate/*.o zlib_inflate/*.o zlib_gzrom zlib_gzrom.bin ${TARGET_LIB}
	rm $(ASL_DIR)/dsdt.c $(ASL_DIR)/*.tmp $(ASL_DIR)/*.aml -rf

cfg:
	# DO NOT DELETE
	perl -i -ne 'print;exit if(/^# DO NOT DELETE/);' ../lib/libc/Makefile
	perl -i -ne 'print;exit if(/^# DO NOT DELETE/);' ../lib/libm/Makefile
	perl -i -ne 'print;exit if(/^# DO NOT DELETE/);' ../lib/libz/Makefile
	mkdir -p ../Targets/${TARGET}/compile
	cd ../Targets/${TARGET}/conf/;../../../tools/pmoncfg/pmoncfg ${TARGETEL}
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/ depend clean
	${TARGET_CFG}
