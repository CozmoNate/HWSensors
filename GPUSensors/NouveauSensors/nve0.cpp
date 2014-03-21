//
//  nve0.c
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
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
#include "nv50.h"
#include "nva3.h"
#include "nv84.h"
#include "nvc0.h"
#include "nve0.h"
#include "nvd0.h"
#include "nouveau_therm.h"

bool nve0_identify(struct nouveau_device *device)
{
	switch (device->chipset) {
        case 0xe4:
            device->cname = "GK104";
            break;
        case 0xe6:
            device->cname = "GK106";
            break;
        case 0xe7:
            device->cname = "GK107";
            break;
        case 0xf0:
            device->cname = "GK110";
            break;
        case 0xf1:
            device->cname = "GK110B";
            break;
        case 0x108:
            device->cname = "GK208";
            break;
        default:
            nv_fatal(device, "unknown Kepler chipset 0x%x\n", device->chipset);
            return false;
	}
    
	return true;
}

static int nve0_clock_read(struct nouveau_device *device, u8 source);

void nve0_init(struct nouveau_device *device)
{
    nvd0_therm_init(device);
    

    device->gpio_find = nouveau_gpio_find;
    device->gpio_get = nouveau_gpio_get;
    device->gpio_sense = nvd0_gpio_sense;
    device->temp_get = nv84_temp_get;
    device->clocks_get = nve0_clock_read;
//    device->voltage_get = nouveau_voltage_get;
    device->pwm_get = nvd0_fan_pwm_get;
    device->fan_pwm_get = nouveau_therm_fan_pwm_get;
    device->fan_rpm_get = nva3_therm_fan_sense;
}

static u32 read_div(struct nouveau_device *, int, u32, u32);
static u32 read_pll(struct nouveau_device *, u32);

static u32 read_vco(struct nouveau_device *device, u32 dsrc)
{
	u32 ssrc = nv_rd32(device, dsrc);
	if (!(ssrc & 0x00000100))
		return read_pll(device, 0x00e800);
	return read_pll(device, 0x00e820);
}

static u32 read_pll(struct nouveau_device *device, u32 pll)
{
	u32 ctrl = nv_rd32(device, pll + 0x00);
	u32 coef = nv_rd32(device, pll + 0x04);
	u32 P = (coef & 0x003f0000) >> 16;
	u32 N = (coef & 0x0000ff00) >> 8;
	u32 M = (coef & 0x000000ff) >> 0;
	u32 sclk;
	u16 fN = 0xf000;
    
	if (!(ctrl & 0x00000001))
		return 0;
    
	switch (pll) {
        case 0x00e800:
        case 0x00e820:
            sclk = device->crystal;
            P = 1;
            break;
        case 0x132000:
            sclk = read_pll(device, 0x132020);
            P = (coef & 0x10000000) ? 2 : 1;
            break;
        case 0x132020:
            sclk = read_div(device, 0, 0x137320, 0x137330);
            fN   = nv_rd32(device, pll + 0x10) >> 16;
            break;
        case 0x137000:
        case 0x137020:
        case 0x137040:
        case 0x1370e0:
            sclk = read_div(device, (pll & 0xff) / 0x20, 0x137120, 0x137140);
            break;
        default:
            return 0;
	}
    
	if (P == 0)
		P = 1;
    
	sclk = (sclk * N) + (((u16)(fN + 4096) * sclk) >> 13);
	return sclk / (M * P);
}

static u32 read_div(struct nouveau_device *device, int doff, u32 dsrc, u32 dctl)
{
	u32 ssrc = nv_rd32(device, dsrc + (doff * 4));
	u32 sctl = nv_rd32(device, dctl + (doff * 4));
    
	switch (ssrc & 0x00000003) {
        case 0:
            if ((ssrc & 0x00030000) != 0x00030000)
                return device->crystal;
            return 108000;
        case 2:
            return 100000;
        case 3:
            if (sctl & 0x80000000) {
                u32 sclk = read_vco(device, dsrc + (doff * 4));
                u32 sdiv = (sctl & 0x0000003f) + 2;
                return (sclk * 2) / sdiv;
            }
            
            return read_vco(device, dsrc + (doff * 4));
        default:
            return 0;
	}
}

static u32 read_mem(struct nouveau_device *device)
{
	switch (nv_rd32(device, 0x1373f4) & 0x0000000f) {
        case 1: return read_pll(device, 0x132020);
        case 2: return read_pll(device, 0x132000);
        default:
            return 0;
	}
}

static u32 read_clk(struct nouveau_device *device, int clk)
{
	u32 sctl = nv_rd32(device, 0x137250 + (clk * 4));
	u32 sclk, sdiv;
    
	if (clk < 7) {
		u32 ssel = nv_rd32(device, 0x137100);
		if (ssel & (1 << clk)) {
			sclk = read_pll(device, 0x137000 + (clk * 0x20));
			sdiv = 1;
		} else {
			sclk = read_div(device, clk, 0x137160, 0x1371d0);
			sdiv = 0;
		}
	} else {
		u32 ssrc = nv_rd32(device, 0x137160 + (clk * 0x04));
		if ((ssrc & 0x00000003) == 0x00000003) {
			sclk = read_div(device, clk, 0x137160, 0x1371d0);
			if (ssrc & 0x00000100) {
				if (ssrc & 0x40000000)
					sclk = read_pll(device, 0x1370e0);
				sdiv = 1;
			} else {
				sdiv = 0;
			}
		} else {
			sclk = read_div(device, clk, 0x137160, 0x1371d0);
			sdiv = 0;
		}
	}
    
	if (sctl & 0x80000000) {
		if (sdiv)
			sdiv = ((sctl & 0x00003f00) >> 8) + 2;
		else
			sdiv = ((sctl & 0x0000003f) >> 0) + 2;
		return (sclk * 2) / sdiv;
	}
    
	return sclk;
}

static int nve0_clock_read(struct nouveau_device *device, u8 source)
{
	switch (source) {
        case nouveau_clock_memory:
            return read_mem(device) * 2;
        case nouveau_clock_core:
            return read_clk(device, 0x00) / 2;
//        case nouveau_clock_rop:
//            return read_clk(device, 0x01) / 2;
//        case nv_clk_src_hubk07:
//            return read_clk(device, 0x02);
//        case nv_clk_src_hubk06:
//            return read_clk(device, 0x07);
//        case nv_clk_src_hubk01:
//            return read_clk(device, 0x08);
//        case nv_clk_src_daemon:
//            return read_clk(device, 0x0c);
//        case nv_clk_src_vdec:
//            return read_clk(device, 0x0e);
	}
    
    return 0;
}