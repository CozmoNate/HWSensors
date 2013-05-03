//
//  nvd0.cpp
//  HWSensors
//
//  Created by Kozlek on 11.08.12.
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

#include "nvd0.h"

#include "nouveau_definitions.h"
#include "nouveau.h"

void nvd0_therm_init(struct nouveau_device *device)
{
    /* enable fan tach, count revolutions per-second */
    nv_mask(device, 0x00e720, 0x00000003, 0x00000002);
    if (device->fan_tach.func != DCB_GPIO_UNUSED) {
        nv_mask(device, 0x00d79c, 0x000000ff, device->fan_tach.line);
        nv_wr32(device, 0x00e724, device->crystal * 1000);
        nv_mask(device, 0x00e720, 0x00000001, 0x00000001);
    }
    nv_mask(device, 0x00e720, 0x00000002, 0x00000000);
}

int nvd0_gpio_sense(struct nouveau_device *device, int line)
{
	return !!(nv_rd32(device, 0x00d610 + (line * 4)) & 0x00004000);
}

static int pwm_info(struct nouveau_device *device, int line)
{
	u32 gpio = nv_rd32(device, 0x00d610 + (line * 0x04));
	switch (gpio & 0x000000c0) {
        case 0x00000000: /* normal mode, possibly pwm forced off by us */
        case 0x00000040: /* nvio special */
            switch (gpio & 0x0000001f) {
                case 0x19: return 1;
                case 0x1c: return 0;
                default:
                    break;
            }
        default:
            break;
	}
    
	nv_debug(device, "GPIO %d unknown PWM: 0x%08x\n", line, gpio);
    
	return -ENODEV;
}

int nvd0_fan_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty)
{
	int indx = pwm_info(device, line);
    
	if (indx < 0)
		return indx;
    
	if (nv_rd32(device, 0x00d610 + (line * 0x04)) & 0x00000040) {
		*divs = nv_rd32(device, 0x00e114 + (indx * 8));
		*duty = nv_rd32(device, 0x00e118 + (indx * 8));
		return 0;
	}
    
	return -EINVAL;
}
