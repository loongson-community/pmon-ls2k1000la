/*
 * Modifications to support Loongson Arch:
 * Copyright (c) 2008  Lemote.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * VESA framebuffer support. Author: Lj.Peng <penglj@lemote.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include <dev/pci/pcivar.h>
#include "xf86int10.h"
#include "xf86x86emu.h"
#include "linux/io.h"

#include "mod_framebuffer.h"
#include "vesa.h"
#include <mod_sisfb.h>
//#include "bonito.h"

extern struct pci_device *vga_dev,*pcie_dev;
int vesa_mode = 1;

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 *  Configure language
 */
#ifdef __ASSEMBLY__
#define _ULCAST_
#else
#define _ULCAST_ (unsigned long)
#endif

/* For vesa mode control */
#define GRAPHIC_MODE_100	0x100	/* 640x400 256*/
#define GRAPHIC_MODE_101	0x101	/* 640x480 256*/
#define GRAPHIC_MODE_102	0x102	/* 800x600 16 */
#define GRAPHIC_MODE_103	0x103	/* 800x600 256*/
#define GRAPHIC_MODE_104	0x104	/* 1024x768 16*/
#define GRAPHIC_MODE_105	0x105	/* 1024x768 256*/
#define GRAPHIC_MODE_106	0x106	/* 1280x1024 16*/
#define GRAPHIC_MODE_107	0x107	/* 1280x1024 256*/
#define GRAPHIC_MODE_10d	0x10d	/* 320x200 32K(1:5:5:5)*/
#define GRAPHIC_MODE_10e	0x10e	/* 320x200 64K(5:6:5)*/
#define GRAPHIC_MODE_10f	0x10f	/* 320x200 16.8M(8:8:8)*/
#define GRAPHIC_MODE_110	0x110	/* 640x480 32K*/
#define GRAPHIC_MODE_111	0x111	/* 640x480 64K*/
#define GRAPHIC_MODE_112	0x112	/* 640x480 16.8M*/
#define GRAPHIC_MODE_113	0x113	/* 800x600 32K*/
#define GRAPHIC_MODE_114	0x114	/* 800x600 64K*/
#define GRAPHIC_MODE_115	0x115	/* 800x600 16.8M*/
#define GRAPHIC_MODE_116	0x116	/* 1024x768 32K*/
#define GRAPHIC_MODE_117	0x117	/* 1024x768 64K*/
#define GRAPHIC_MODE_118	0x118	/* 1024x768 16.8M*/
#define GRAPHIC_MODE_119	0x119	/* 1280x1024 32K*/
#define GRAPHIC_MODE_11a	0x11a	/* 1280x1024 64K*/
#define GRAPHIC_MODE_11b	0x11b	/* 1280x1024 16.8M*/
#define USE_LINEAR_FRAMEBUFFER 0x4000
struct vesamode vesamode[] = {
	{GRAPHIC_MODE_114,800,600,16}, /* default 800x600x16 */
	{GRAPHIC_MODE_100,640,400,8},
	{GRAPHIC_MODE_101,640,480,8},
	{GRAPHIC_MODE_102,800,600,4},
	{GRAPHIC_MODE_103,800,600,8},
	{GRAPHIC_MODE_104,1024,768,16},
	{GRAPHIC_MODE_105,1024,768,8},
	{GRAPHIC_MODE_106,1280,1024,16},
	{GRAPHIC_MODE_107,1280,1024,8},
	{GRAPHIC_MODE_10d,320,200,15}, 
	{GRAPHIC_MODE_10e,320,200,16}, 
	{GRAPHIC_MODE_10f,320,200,24},
	{GRAPHIC_MODE_110,640,480,15},
	{GRAPHIC_MODE_111,640,480,16},
	{GRAPHIC_MODE_112,640,480,24},
	{GRAPHIC_MODE_113,800,600,15},
	{GRAPHIC_MODE_114,800,600,16},
	{GRAPHIC_MODE_115,800,600,24},
	{GRAPHIC_MODE_116,1024,768,15},
	{GRAPHIC_MODE_117,1024,768,16},
	{GRAPHIC_MODE_118,1024,768,24},
	{GRAPHIC_MODE_119,1280,1024,15},
	{GRAPHIC_MODE_11a,1280,1024,16},
	{GRAPHIC_MODE_11b,1280,1024,24},
};

struct vesamode *vesa_mode_head = vesamode;


static u32 io_vaddr;

#define BONITO_REGBASE			0x100
#define BONITO_PCIMAP			BONITO(BONITO_REGBASE + 0x10)
#define BONITO(x)	*(volatile unsigned long *)(0xbfe00000+(x))
#define BONITO_PCIMAP			BONITO(BONITO_REGBASE + 0x10)
#define BONITO_PCIMAP_PCIMAP_LO0	0x0000003f
#define BONITO_PCIMAP_WIN(WIN,ADDR)	((((ADDR)>>26) & BONITO_PCIMAP_PCIMAP_LO0) << ((WIN)*6))

int  vesafb_init(void)
{
	u32 video_mem_size;
	u32 fb_address, io_address;
	u32 tmp;

	if(vga_dev != NULL){
		fb_address  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
		io_address  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);
	}
	if(pcie_dev != NULL){
		fb_address  =_pci_conf_read(pcie_dev->pa.pa_tag,0x10);
		io_address  =_pci_conf_read(pcie_dev->pa.pa_tag,0x18);
	}
	io_vaddr = io_address | BONITO_PCIIO_BASE_VA;

	printf("VESA FB init complete.\n");

	return 0;
}

/* dummy implementation */
void video_set_lut2(int index, int rgb)
{
	return;
}

int GetXRes(void)
{
	return vesamode[vesa_mode].width;
}

int GetYRes(void)
{
	return vesamode[vesa_mode].height;
}

int GetBytesPP(void)
{
	return (vesamode[vesa_mode].bpp+1)/8;
}

void video_set_lut(int index, int r, int g, int b)
{				     
	linux_outb(index, 0x03C8);
	linux_outb(r >> 2, 0x03C9);
	linux_outb(g >> 2, 0x03C9);
	linux_outb(b >> 2, 0x03C9);
}
