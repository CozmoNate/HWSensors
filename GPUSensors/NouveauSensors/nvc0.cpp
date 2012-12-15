//
//  nvc0.c
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

#include "nvc0.h"

#include "nouveau.h"
#include "nv50.h"
#include "nva3.h"
#include "nvd0.h"
#include "nouveau_therm.h"

bool nvc0_identify(struct nouveau_device *device)
{
	switch (device->chipset) {
        case 0xc0:
            device->cname = "GF100";
            break;
        case 0xc4:
            device->cname = "GF104";
            break;
        case 0xc3:
            device->cname = "GF106";
            break;
        case 0xce:
            device->cname = "GF114";
            break;
        case 0xcf:
            device->cname = "GF116";
            break;
        case 0xc1:
            device->cname = "GF108";
            break;
        case 0xc8:
            device->cname = "GF110";
            break;
        case 0xd9:
            device->cname = "GF119";
            break;
        default:
            nv_fatal(device, "unknown Fermi chipset\n");
            return false;
    }
    
	return true;
}

void nvc0_init(struct nouveau_device *device)
{
    switch (device->chipset) {
        
        case 0xd9:
            device->gpio_sense = nvd0_gpio_sense;
            device->fan_rpm_get = nouveau_therm_fan_rpm_get;
        default:
            nva3_therm_init(device);
            device->gpio_sense = nv50_gpio_sense;
            device->fan_rpm_get = nva3_therm_fan_sense;
            break;
    }
    
    device->gpio_find = nouveau_gpio_find;
    device->gpio_get = nouveau_gpio_get;

    device->temp_get = nv50_temp_get;
    device->clocks_get = nvc0_clocks_get;
    device->voltage_get = nouveau_voltage_get;
    device->pwm_get = nv50_fan_pwm_get;
    device->fan_pwm_get = nouveau_therm_fan_pwm_get;
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
	u32 ctrl = nv_rd32(device, pll + 0);
	u32 coef = nv_rd32(device, pll + 4);
	u32 P = (coef & 0x003f0000) >> 16;
	u32 N = (coef & 0x0000ff00) >> 8;
	u32 M = (coef & 0x000000ff) >> 0;
	u32 sclk, doff;
    
	if (!(ctrl & 0x00000001))
		return 0;
    
	switch (pll & 0xfff000) {
        case 0x00e000:
            sclk = 27000;
            P = 1;
            break;
        case 0x137000:
            doff = (pll - 0x137000) / 0x20;
            sclk = read_div(device, doff, 0x137120, 0x137140);
            break;
        case 0x132000:
            switch (pll) {
                case 0x132000:
                    sclk = read_pll(device, 0x132020);
                    break;
                case 0x132020:
                    sclk = read_div(device, 0, 0x137320, 0x137330);
                    break;
                default:
                    return 0;
            }
            break;
        default:
            return 0;
	}
    
	return sclk * N / M / P;
}

static u32 read_div(struct nouveau_device *device, int doff, u32 dsrc, u32 dctl)
{
	u32 ssrc = nv_rd32(device, dsrc + (doff * 4));
	u32 sctl = nv_rd32(device, dctl + (doff * 4));
    
	switch (ssrc & 0x00000003) {
        case 0:
            if ((ssrc & 0x00030000) != 0x00030000)
                return 27000;
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
	u32 ssel = nv_rd32(device, 0x1373f0);
	
    if (ssel & 0x00000001)
		return read_div(device, 0, 0x137300, 0x137310);

    if (device->card_type == NV_C0)
        return read_pll(device, 0x132000) / 2.0f;
    else if (device->card_type == NV_E0)
        return read_pll(device, 0x132000) * 2.0f;
    
	return read_pll(device, 0x132000);
}

static u32 read_clk(struct nouveau_device *device, int clk)
{
	u32 sctl = nv_rd32(device, 0x137250 + (clk * 4));
	u32 ssel = nv_rd32(device, 0x137100);
	u32 sclk, sdiv;
    
	if (ssel & (1 << clk)) {
		if (clk < 7)
			sclk = read_pll(device, 0x137000 + (clk * 0x20));
		else
			sclk = read_pll(device, 0x1370e0);
		sdiv = ((sctl & 0x00003f00) >> 8) + 2;
	} else {
		sclk = read_div(device, clk, 0x137160, 0x1371d0);
		sdiv = ((sctl & 0x0000003f) >> 0) + 2;
	}
    
	if (sctl & 0x80000000)
		return (sclk * 2) / sdiv;
	return sclk;
}

int nvc0_clocks_get(struct nouveau_device *device, u8 source)
{
    switch (source) {
        case nouveau_clock_core:
            return read_clk(device, 0x00) / 2;
        case nouveau_clock_shader:
            return read_clk(device, 0x00);
        case nouveau_clock_memory:
            return read_mem(device);
        case nouveau_clock_rop:
            return read_clk(device, 0x01);
        case nouveau_clock_hub01:
            return read_clk(device, 0x08);
        case nouveau_clock_hub06:
            return read_clk(device, 0x07);
        case nouveau_clock_hub07:
            return read_clk(device, 0x02);
        case nouveau_clock_vdec:
            return read_clk(device, 0x0e);
        case nouveau_clock_daemon:
            return read_clk(device, 0x0c);
        case nouveau_clock_copy:
            return read_clk(device, 0x09);
        default:
            return 0;
    }
}
