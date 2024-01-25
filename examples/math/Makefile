#
#  Makefile to build hello.c
#

CROSS_COMPILE 	=loongisa-sde-elf-

#
# Include the make variables (CC, etc...)
#

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
SIZE		= $(CROSS_COMPILE)size

hello:	hello.o
	#${CC} -G 0 -O2 -EL -mcpu=r4000 -Ttext=0x80200000 -N -o hello hello.o -nostdlib -e main libm.a libgcc.a
	${CC} -G 0 -O2 -EL -mcpu=r4000 -Ttext=0x80200000 -N -o hello hello.o -nostdlib -e main libm.a libgcc.a
	cp hello /tftpboot
.S.o:
	#loongarch-linux-gcc -G 0 -c $< -mno-abicalls -fno-pic 
	loongisa-sde-elf-gcc  -G 0 -c $< -fno-pic 
.c.o:
	#loongarch-linux-gcc -EL -c $< -G 0 -mno-abicalls -fno-pic
	loongisa-sde-elf-gcc  -EL -c $< -G 0 -fno-pic

clean:
	rm -f hello *.o
