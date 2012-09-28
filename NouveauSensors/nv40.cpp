//
//  nv40.c
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
#include "nv40.h"
#include "nv10.h"
#include "nouveau_therm.h"

bool nv40_identify(struct nouveau_device *device)
{
	switch (device->chipset) {
        case 0x40:
            device->cname = "NV40";
            break;
        case 0x41:
            device->cname = "NV41";
            break;
        case 0x42:
            device->cname = "NV42";
            break;
        case 0x43:
            device->cname = "NV43";
            break;
        case 0x45:
            device->cname = "NV45";
            break;
        case 0x47:
            device->cname = "G70";
            break;
        case 0x49:
            device->cname = "G71";
            break;
        case 0x4b:
            device->cname = "G73";
            break;
        case 0x44:
            device->cname = "NV44";
            break;
        case 0x46:
            device->cname = "G72";
            break;
        case 0x4a:
            device->cname = "NV44A";
            break;
        case 0x4c:
            device->cname = "C61";
            break;
        case 0x4e:
            device->cname = "C51";
            break;
        case 0x63:
            device->cname = "C73";
            break;
        case 0x67:
            device->cname = "C67";
            break;
        case 0x68:
            device->cname = "C68";
            break;
        default:
            nv_fatal(device, "unknown Curie chipset\n");
            return false;
	}
    
	return true;
}

void nv40_init(struct nouveau_device *device)
{
    device->gpio_sense = nv10_gpio_sense;
    device->gpio_find = nouveau_gpio_find;
    device->gpio_get = nouveau_gpio_get;
    
    device->temp_get = nv40_temp_get;
    device->clocks_get = nv40_clocks_get;
    device->pwm_get = nv40_fan_pwm_get;
    device->voltage_get = nouveau_voltage_get;
    device->fan_pwm_get = nouveau_therm_fan_get;
    device->fan_rpm_get = nouveau_therm_fan_sense;
}

static u32 read_pll_1(struct nouveau_device *device, u32 reg)
{
	u32 ctrl = nv_rd32(device, reg + 0x00);
	int P = (ctrl & 0x00070000) >> 16;
	int N = (ctrl & 0x0000ff00) >> 8;
	int M = (ctrl & 0x000000ff) >> 0;
	u32 ref = 27000, clk = 0;
    
	if (ctrl & 0x80000000)
		clk = ref * N / M;
    
	return clk >> P;
}

static u32 read_pll_2(struct nouveau_device *device, u32 reg)
{
	u32 ctrl = nv_rd32(device, reg + 0x00);
	u32 coef = nv_rd32(device, reg + 0x04);
	int N2 = (coef & 0xff000000) >> 24;
	int M2 = (coef & 0x00ff0000) >> 16;
	int N1 = (coef & 0x0000ff00) >> 8;
	int M1 = (coef & 0x000000ff) >> 0;
	int P = (ctrl & 0x00070000) >> 16;
	u32 ref = 27000, clk = 0;
    
	if ((ctrl & 0x80000000) && M1) {
		clk = ref * N1 / M1;
		if ((ctrl & 0x40000100) == 0x40000000) {
			if (M2)
				clk = clk * N2 / M2;
			else
				clk = 0;
		}
	}
    
	return clk >> P;
}

static u32 read_clk(struct nouveau_device *device, u32 src)
{
	switch (src) {
        case 3:
            return read_pll_2(device, 0x004000);
        case 2:
            return read_pll_1(device, 0x004008);
        default:
            break;
	}
    
	return 0;
}

int nv40_clocks_get(struct nouveau_device *device, u8 source)
{
	u32 ctrl = nv_rd32(device, 0x00c040);
    
    switch (source) {
        case nouveau_clock_core:
            return read_clk(device, (ctrl & 0x00000003) >> 0);
            
        case nouveau_clock_shader:
            return read_clk(device, (ctrl & 0x00000030) >> 4);
            
        case  nouveau_clock_memory:
            return read_pll_2(device, 0x4020);
            
        default:
            return 0;
    }
}

static int nv40_sensor_setup(struct nouveau_device *device)
{
	/* enable ADC readout and disable the ALARM threshold */
	if (device->chipset >= 0x46) {
		nv_mask(device, 0x15b8, 0x80000000, 0);
		nv_wr32(device, 0x15b0, 0x80003fff);
		return nv_rd32(device, 0x15b4) & 0x3fff;
	} else {
		nv_wr32(device, 0x15b0, 0xff);
		return nv_rd32(device, 0x15b4) & 0xff;
	}
}

int nv40_temp_get(struct nouveau_device *device)
{
	//struct nouveau_pm *pm = nouveau_pm(dev);
	struct nouveau_pm_temp_sensor_constants *sensor = &device->sensor_constants;
    int core_temp;
    
	if (device->chipset >= 0x46) {
		nv_wr32(device, 0x15b0, 0x80003fff);
		core_temp = nv_rd32(device, 0x15b4) & 0x3fff;
	} else {
		nv_wr32(device, 0x15b0, 0xff);
		core_temp = nv_rd32(device, 0x15b4) & 0xff;
	}
    
	/* Setup the sensor if the temperature is 0 */
	if (core_temp == 0)
		core_temp = nv40_sensor_setup(device);
    
	if (sensor->slope_div == 0)
		sensor->slope_div = 1;
	if (sensor->offset_div == 0)
		sensor->offset_div = 1;
	if (sensor->slope_mult < 1)
		sensor->slope_mult = 1;
    
	core_temp = core_temp * sensor->slope_mult / sensor->slope_div;
	core_temp = core_temp + sensor->offset_mult / sensor->offset_div;
	core_temp = core_temp + sensor->offset_constant - 8;
    
	return core_temp;
}

int nv40_fan_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty)
{
	if (line == 2) {
		u32 reg = nv_rd32(device, 0x0010f0);
		if (reg & 0x80000000) {
			*duty = (reg & 0x7fff0000) >> 16;
			*divs = (reg & 0x00007fff);
			return 0;
		}
	} else
        if (line == 9) {
            u32 reg = nv_rd32(device, 0x0015f4);
            if (reg & 0x80000000) {
                *divs = nv_rd32(device, 0x0015f8);
                *duty = (reg & 0x7fffffff);
                return 0;
            }
        } else {
            nv_error(device, "unknown pwm ctrl for gpio %d\n", line);
            return -ENODEV;
        }
    
	return -ENODEV;
}
