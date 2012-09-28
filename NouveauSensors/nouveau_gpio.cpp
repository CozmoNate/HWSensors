//
//  nouveau_gpio.cpp
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

#include "nouveau_gpio.h"
#include "nouveau.h"
#include "nouveau_bios.h"

static u16 dcb_gpio_table(struct nouveau_device *device)
{
	u8  ver, hdr, cnt, len;
	u16 dcb = nouveau_dcb_table(device, &ver, &hdr, &cnt, &len);
	if (dcb) {
		if (ver >= 0x30 && hdr >= 0x0c)
			return nv_ro16(device, dcb + 0x0a);
		if (ver >= 0x22 && nv_ro08(device, dcb - 1) >= 0x13)
			return nv_ro16(device, dcb - 0x0f);
	}
	return 0x0000;
}

static u16 dcb_gpio_entry(struct nouveau_device *device, int idx, int ent, u8 *ver)
{
	u16 gpio = dcb_gpio_table(device);
	if (gpio) {
		*ver = nv_ro08(device, gpio);
		if (*ver < 0x30 && ent < nv_ro08(device, gpio + 2))
			return gpio + 3 + (ent * nv_ro08(device, gpio + 1));
		else if (ent < nv_ro08(device, gpio + 2))
			return gpio + nv_ro08(device, gpio + 1) +
            (ent * nv_ro08(device, gpio + 3));
	}
	return 0x0000;
}

static int dcb_gpio_parse(struct nouveau_device *device, int idx, u8 func, u8 line, struct dcb_gpio_func *gpio)
{
	u8  ver, hdr, cnt, len;
	u16 entry;
	int i = -1;
    
	while ((entry = dcb_gpio_entry(device, idx, ++i, &ver))) {
		if (ver < 0x40) {
			u16 data = nv_ro16(device, entry);
			(*gpio).line = (data & 0x001f) >> 0;
			(*gpio).func = (data & 0x07e0) >> 5;
            (*gpio).log[0] = (data & 0x1800) >> 11;
            (*gpio).log[1] = (data & 0x6000) >> 13;
		} else
            if (ver < 0x41) {
                u32 data = nv_ro32(device, entry);
                (*gpio).line = (data & 0x0000001f) >> 0;
                (*gpio).func = (data & 0x0000ff00) >> 8;
                (*gpio).log[0] = (data & 0x18000000) >> 27;
                (*gpio).log[1] = (data & 0x60000000) >> 29;
            } else {
                u32 data = nv_ro32(device, entry + 0);
                u8 data1 = nv_ro32(device, entry + 4);
                (*gpio).line = (data & 0x0000003f) >> 0;
                (*gpio).func = (data & 0x0000ff00) >> 8;
                (*gpio).log[0] = (data1 & 0x30) >> 4;
                (*gpio).log[1] = (data1 & 0xc0) >> 6;
            }
        
		if ((line == 0xff || line == gpio->line) &&
		    (func == 0xff || func == gpio->func))
			return 0;
	}
    
	/* DCB 2.2, fixed TVDAC GPIO data */
	if ((entry = nouveau_dcb_table(device, &ver, &hdr, &cnt, &len)) && ver >= 0x22) {
		if (func == DCB_GPIO_TVDAC0) {
            (*gpio).func = DCB_GPIO_TVDAC0;
            (*gpio).line = nv_ro08(device, entry - 4) >> 4;
            (*gpio).log[0] = !!(nv_ro08(device, entry - 5) & 2);
            (*gpio).log[1] =  !(nv_ro08(device, entry - 5) & 2);
			return 0;
		}
	}
    
	return -EINVAL;
}

int nouveau_gpio_sense(struct nouveau_device *device, int idx, int line)
{
    if (!device->gpio_sense) {
        nv_debug(device, "hardware GPIO sense function not set\n");
        return -EINVAL;
    }
    
	return device->gpio_sense(device, line);
}

int nouveau_gpio_find(struct nouveau_device *device, int idx, u8 tag, u8 line, struct dcb_gpio_func *func)
{
	if (line == 0xff && tag == 0xff)
		return -EINVAL;
    
	if (!dcb_gpio_parse(device, idx, tag, line, func))
		return 0;
    
//	/* Apple iMac G4 NV18 */
//	if (nv_device_match(nv_object(gpio), 0x0189, 0x10de, 0x0010)) {
//		if (tag == DCB_GPIO_TVDAC0) {
//			*func = (struct dcb_gpio_func) {
//				.func = DCB_GPIO_TVDAC0,
//				.line = 4,
//				.log[0] = 0,
//				.log[1] = 1,
//			};
//			return 0;
//		}
//	}
    
	return -EINVAL;
}

int nouveau_gpio_get(struct nouveau_device *device, int idx, u8 tag, u8 line)
{
	struct dcb_gpio_func func;
	int ret = 0;
    
    ret = nouveau_gpio_find(device, idx, tag, line, &func);
	if (ret == 0) {
		ret = nouveau_gpio_sense(device, idx, func.line);
		if (ret >= 0)
			ret = (ret == (func.log[1] & 1));
        else
            nv_debug(device, "insane GPIO value\n");
	}
    else nv_debug(device, "GPIO not found\n");
    
	return ret;
}
