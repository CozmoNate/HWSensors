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
        default:
            nv_fatal(device, "unknown Kepler chipset\n");
            return false;
	}
    
	return true;
}

void nve0_init(struct nouveau_device *device)
{
    nvd0_therm_init(device);
    
    device->gpio_sense = nvd0_gpio_sense;
    device->gpio_find = nouveau_gpio_find;
    device->gpio_get = nouveau_gpio_get;
    
    device->temp_get = nv50_temp_get;
//    device->clocks_get = nvc0_clocks_get;
//    device->voltage_get = nouveau_voltage_get;
    device->pwm_get = nvd0_fan_pwm_get;
    device->fan_pwm_get = nouveau_therm_fan_pwm_get;
    device->fan_rpm_get = nouveau_therm_fan_rpm_get;
}


