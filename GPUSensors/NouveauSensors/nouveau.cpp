//
//  nouveau.c
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

/*
 * Copyright 2010 Red Hat Inc.
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
#include "nv50.h"
#include "nva3.h"
#include "nvc0.h"
#include "nve0.h"
#include "gm100.h"
#include "gp100.h"
#include "nouveau_therm.h"
#include "nouveau_volt.h"

bool nouveau_identify(struct nouveau_device *device)
{
    /* identify the chipset */
    
    nv_debug(device, "identifying the chipset\n");
    
    /* read boot0 and strapping information */
    UInt32 boot0 = nv_rd32(device, 0x000000);
    UInt32 strap = nv_rd32(device, 0x101000);
    
    /* determine chipset and derive architecture from it */
    if ((boot0 & 0x1f000000) > 0) {
        device->chipset = (boot0 & 0x1ff00000) >> 20;
        switch (device->chipset & 0x1f0) {
			case 0x040:
			case 0x060: device->card_type = NV_40; break;
			case 0x050:
			case 0x080:
			case 0x090:
			case 0x0a0: device->card_type = NV_50; break;
			case 0x0c0: device->card_type = NV_C0; break;
			case 0x0d0: device->card_type = NV_D0; break;
			case 0x0e0:
            case 0x0f0:
            case 0x100: device->card_type = NV_E0; break;
            case 0x110:
            case 0x120: device->card_type = GM100; break;
            case 0x130: device->card_type = GP100; break;
			default:
				break;
        }
    }
    
    bool ret = FALSE;
    
    switch (device->card_type) {
		case NV_40: ret = nv40_identify(device); break;
		case NV_50: ret = nv50_identify(device); break;
		case NV_C0:
		case NV_D0: ret = nvc0_identify(device); break;
		case NV_E0: ret = nve0_identify(device); break;
        case GM100: ret = gm100_identify(device); break;
        case GP100: ret = gp100_identify(device); break;
        default: break;
    }
    
    if (!ret) {
        nv_fatal(device, "unsupported chipset, 0x%08x\n", boot0);
        return false;
    }
    
    nv_debug(device, "BOOT0  : 0x%08x\n", boot0);
    nv_debug(device, "chipset: %s (NV%02X) family: NV%02X\n",
			device->cname, device->chipset, device->card_type);
    /* determine frequency of timing crystal */
    if ( device->chipset < 0x17 ||
        (device->chipset >= 0x20 && device->chipset <= 0x25))
        strap &= 0x00000040;
    else
        strap &= 0x00400040;
    
    switch (strap) {
		case 0x00000000: device->crystal = 13500; break;
		case 0x00000040: device->crystal = 14318; break;
		case 0x00400000: device->crystal = 27000; break;
		case 0x00400040: device->crystal = 25000; break;
    }
    
    nv_debug(device, "crystal freq: %dKHz\n", device->crystal);
    
    return true;
}

bool nouveau_init(struct nouveau_device *device)
{
	int ret;
    
    nv_debug(device, "initializing monitoring driver\n");
    
    switch (device->card_type) {
		case NV_40: nv40_init(device); break;
		case NV_50: nv50_init(device); break;
		case NV_C0:
		case NV_D0: nvc0_init(device); break;
		case NV_E0: nve0_init(device); break;
        case GM100: gm100_init(device); break;
        case GP100: gp100_init(device); break;
        default: break;
    }
    
    // attempt to locate a drivable fan
    ret = device->gpio_find(device, 0, DCB_GPIO_FAN, 0xff, &device->fan_pwm);
    if (ret < 0)
		device->fan_pwm.func = DCB_GPIO_UNUSED;
    
    /* attempt to detect a tachometer connection */
	ret = device->gpio_find(device, 0, DCB_GPIO_FAN_SENSE, 0xff, &device->fan_tach);
	if (ret < 0)
		device->fan_tach.func = DCB_GPIO_UNUSED;
    
    /*if (device->gpio_init)
        device->gpio_init(device);*/
    
	/* parse aux tables from vbios */
	//nouveau_volt_init(device);
    nouveau_volt_create(device);
    nouveau_therm_init(device);
    
    return true;
}
