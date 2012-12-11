//
//  nva3.c
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
//
//

/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */

#include "nouveau.h"
#include "nva3.h"

int nva3_therm_fan_sense(struct nouveau_device *device)
{
	u32 tach = nv_rd32(device, 0x00e728) & 0x0000ffff;
	u32 ctrl = nv_rd32(device, 0x00e720);
	if (ctrl & 0x00000001)
		return tach * 60;
	return -ENODEV;
}

int nva3_therm_init(struct nouveau_device *device)
{
	struct dcb_gpio_func *tach = &device->fan_tach;
    
	/* enable fan tach, count revolutions per-second */
	nv_mask(device, 0x00e720, 0x00000003, 0x00000002);
	if (tach->func != DCB_GPIO_UNUSED) {
		nv_wr32(device, 0x00e724, device->crystal * 1000);
		nv_mask(device, 0x00e720, 0x001f0000, tach->line << 16);
		nv_mask(device, 0x00e720, 0x00000001, 0x00000001);
	}
	nv_mask(device, 0x00e720, 0x00000002, 0x00000000);
    
	return 0;
}

static u32 read_clk(struct nouveau_device *, int, bool);
static u32 read_pll(struct nouveau_device *, int, u32);

static u32 read_vco(struct nouveau_device *device, int clk)
{
	u32 sctl = nv_rd32(device, 0x4120 + (clk * 4));
	if ((sctl & 0x00000030) != 0x00000030)
		return read_pll(device, 0x41, 0x00e820);
	return read_pll(device, 0x42, 0x00e8a0);
}

static u32 read_clk(struct nouveau_device *device, int clk, bool ignore_en)
{
	u32 sctl, sdiv, sclk;
    
	/* refclk for the 0xe8xx plls is a fixed frequency */
	if (clk >= 0x40) {
		if (device->chipset == 0xaf) {
			/* no joke.. seriously.. sigh.. */
			return nv_rd32(device, 0x00471c) * 1000;
		}
        
		return device->crystal;
	}
    
	sctl = nv_rd32(device, 0x4120 + (clk * 4));
	if (!ignore_en && !(sctl & 0x00000100))
		return 0;
    
	switch (sctl & 0x00003000) {
        case 0x00000000:
            return device->crystal;
        case 0x00002000:
            if (sctl & 0x00000040)
                return 108000;
            return 100000;
        case 0x00003000:
            sclk = read_vco(device, clk);
            sdiv = ((sctl & 0x003f0000) >> 16) + 2;
            return (sclk * 2) / sdiv;
        default:
            return 0;
	}
}

static u32 read_pll(struct nouveau_device *device, int clk, u32 pll)
{
	u32 ctrl = nv_rd32(device, pll + 0);
	u32 sclk = 0, P = 1, N = 1, M = 1;
    
	if (!(ctrl & 0x00000008)) {
		if (ctrl & 0x00000001) {
			u32 coef = nv_rd32(device, pll + 4);
			M = (coef & 0x000000ff) >> 0;
			N = (coef & 0x0000ff00) >> 8;
			P = (coef & 0x003f0000) >> 16;
            
			/* no post-divider on these.. */
			if ((pll & 0x00ff00) == 0x00e800)
				P = 1;
            
			sclk = read_clk(device, 0x00 + clk, false);
		}
	} else {
		sclk = read_clk(device, 0x10 + clk, false);
	}
    
	if (M * P)
		return sclk * N / (M * P);
	return 0;
}

int nva3_clocks_get(struct nouveau_device *device, u8 source)
{
    switch (source) {
        case nouveau_clock_core:
            return read_pll(device, 0x00, 0x4200);
        case nouveau_clock_shader:
            return read_pll(device, 0x01, 0x4220);
        case nouveau_clock_memory:
            return read_pll(device, 0x02, 0x4000);
        case nouveau_clock_unka0:
            return read_clk(device, 0x20, false);
        case nouveau_clock_vdec:
            return read_clk(device, 0x21, false);
        case nouveau_clock_daemon:
            return read_clk(device, 0x25, false);
        case nouveau_clock_copy:
            return read_pll(device, 0x00, 0x4200); // core
        default:
            return 0;
    }
}