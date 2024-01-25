ifndef S
S:=$(shell cd ../../../../..; pwd)
endif
TARGET=ls3a5000_7a
TARGETEL=loongson

LS7A_TARGETS=ls7a.a

IDENT=$(shell cat ${S}/Targets/${TARGET}/compile/${TARGETEL}/Makefile | grep "IDENT" | sed 's/IDENT=//g')

CC=loongarch64-linux-gnu-gcc -mabi=lp64 -mabi=lp64 -march=loongarch64 -fno-delayed-branch -G 0 -mno-memcpy -fno-builtin -c -fno-pic -Wall -Wstrict-prototypes -Wno-uninitialized -Wno-format -Wno-main -O2 -G 0 -mmemcpy -fno-builtin -nostdinc -fno-strict-aliasing -fno-pic -I$S/ -I$S/include -I$S/sys/arch/loongarch/include -I$S/sys -I$S/Targets/${TARGET}/compile/${TARGETEL} -I$S/Targets/${TARGET}/ -I$S/Targets/${TARGET}/include -DSYSTYPE="\"${TARGETEL}\"" -DTARGETNAME="\"${TARGETEL}\"" -DPMON -D__PMON__ ${IDENT}

AR=loongarch64-linux-gnu-ar Drc

LS7A_SRC=$(wildcard *.c)

LS7A_OBJS=${LS7A_SRC:.c=.o}

all: ${LS7A_OBJS} $(LS7A_TARGETS)
	cp $(LS7A_TARGETS) ${S}/Targets/${TARGET}/compile/${TARGETEL}/
	cp $(LS7A_TARGETS) $S/zloader/

${LS7A_OBJS}: ${LS7A_SRC}
ifneq ($(wildcard ${LS7A_SRC}), )
	$(CC) -c ${LS7A_SRC}
	rm -rf *.gch
endif

$(LS7A_TARGETS):${LS7A_OBJS}
ifneq ($(wildcard ${LS7A_SRC}), )
	${AR} ${LS7A_TARGETS} ${LS7A_OBJS}
	rm ${LS7A_OBJS}
endif

clean:
ifneq ($(wildcard ${LS7A_SRC}), )
	rm -f *.o
	rm -f $S/Targets/ls3a5000_7a/compile/loongson/$(LS7A_TARGETS)
	rm -f $S/zloader/$(LS7A_TARGETS)
endif
