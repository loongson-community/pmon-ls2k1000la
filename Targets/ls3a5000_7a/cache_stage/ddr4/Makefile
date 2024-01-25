ifndef S
S:=$(shell cd ../../../..; pwd)
endif
TARGET=mem_config.a

all: hunt
	cp binary/$(TARGET) ../../compile/loongson/
	cp binary/$(TARGET) $S/zloader/

hunt:
ifneq ($(wildcard source), )
	make all -C source
endif

clean:
ifneq ($(wildcard source), )
	make clean -C source
endif
