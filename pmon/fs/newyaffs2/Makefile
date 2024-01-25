#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#


.SUFFIXES: .o64 .elf64 .bin64
PWD=$(shell pwd)
SOURCES := yaffs_allocator.c yaffs_bitmap.c yaffs_checkptrw.c yaffs_ecc.c yaffs_endian.c yaffs_guts.c yaffs_nameval.c yaffs_nand.c yaffs_packedtags1.c yaffs_packedtags2.c yaffs_summary.c yaffs_tagscompat.c yaffs_tagsmarshall.c yaffs_verify.c yaffs_yaffs1.c yaffs_yaffs2.c yaffsfs.c yaffs_attribs.c yaffs_osglue.c ynand-mtd.c yaffs_hweight.c
EXTRA_CFLAGS := -I $(PWD) -I $(PWD)/include -DCONFIG_YAFFS_DEFINES_TYPES -DCONFIG_YAFFS_DIRECT -DCONFIG_YAFFS_PROVIDE_DEFS -DCONFIG_YAFFSFS_PROVIDE_VALUES $(EXTRA_CFLAGS)

objs := $(patsubst %.c,%.o,$(SOURCES))

clean:
	rm -f *.o *.a *.s *.o64 *.bin64 *.elf64


-include $(TOPDIR)/rules.make
