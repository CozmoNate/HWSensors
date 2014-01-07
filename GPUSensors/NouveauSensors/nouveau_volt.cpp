//
//  nouveau_volt.c
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
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

#include "nouveau_volt.h"
#include "nouveau.h"
#include "nouveau_bios.h"

static u16 nvbios_volt_table(struct nouveau_device *device, u8 *ver, u8 *hdr, u8 *cnt, u8 *len)
{
	struct bit_entry bit_P;
	u16 volt = 0x0000;
    
	if (!nouveau_bit_entry(device, 'P', &bit_P)) {
		if (bit_P.version == 2)
			volt = nv_ro16(device, bit_P.offset + 0x0c);
		else
            if (bit_P.version == 1)
                volt = nv_ro16(device, bit_P.offset + 0x10);
        
		if (volt) {
			*ver = nv_ro08(device, volt + 0);
			switch (*ver) {
                case 0x12:
                    *hdr = 5;
                    *cnt = nv_ro08(device, volt + 2);
                    *len = nv_ro08(device, volt + 1);
                    return volt;
                case 0x20:
                    *hdr = nv_ro08(device, volt + 1);
                    *cnt = nv_ro08(device, volt + 2);
                    *len = nv_ro08(device, volt + 3);
                    return volt;
                case 0x30:
                case 0x40:
                case 0x50:
                    *hdr = nv_ro08(device, volt + 1);
                    *cnt = nv_ro08(device, volt + 3);
                    *len = nv_ro08(device, volt + 2);
                    return volt;
			}
		}
	}
    
	return 0x0000;
}

static u16 nvbios_volt_parse(struct nouveau_device *device, u8 *ver, u8 *hdr, u8 *cnt, u8 *len,
                  struct nvbios_volt *info)
{
	u16 volt = nvbios_volt_table(device, ver, hdr, cnt, len);
	memset(info, 0x00, sizeof(*info));
	switch (!!volt * *ver) {
        case 0x12:
            info->vidmask = nv_ro08(device, volt + 0x04);
            break;
        case 0x20:
            info->vidmask = nv_ro08(device, volt + 0x05);
            break;
        case 0x30:
            info->vidmask = nv_ro08(device, volt + 0x04);
            break;
        case 0x40:
            info->base    = nv_ro32(device, volt + 0x04);
            info->step    = nv_ro16(device, volt + 0x08);
            info->vidmask = nv_ro08(device, volt + 0x0b);
            /*XXX*/
            info->min     = 0;
            info->max     = info->base;
            break;
        case 0x50:
            info->vidmask = nv_ro08(device, volt + 0x06);
            info->min     = nv_ro32(device, volt + 0x0a);
            info->max     = nv_ro32(device, volt + 0x0e);
            info->base    = nv_ro32(device, volt + 0x12) & 0x00ffffff;
            info->step    = nv_ro16(device, volt + 0x16);
            break;
	}
	return volt;
}

static u16 nvbios_volt_entry(struct nouveau_device *device, int idx, u8 *ver, u8 *len)
{
	u8  hdr, cnt;
	u16 volt = nvbios_volt_table(device, ver, &hdr, &cnt, len);
	if (volt && idx < cnt) {
		volt = volt + hdr + (idx * *len);
		return volt;
	}
	return 0x0000;
}

static u16 nvbios_volt_entry_parse(struct nouveau_device *device, int idx, u8 *ver, u8 *len,
                        struct nvbios_volt_entry *info)
{
	u16 volt = nvbios_volt_entry(device, idx, ver, len);
	memset(info, 0x00, sizeof(*info));
	switch (!!volt * *ver) {
        case 0x12:
        case 0x20:
            info->voltage = nv_ro08(device, volt + 0x00) * 10000;
            info->vid     = nv_ro08(device, volt + 0x01);
            break;
        case 0x30:
            info->voltage = nv_ro08(device, volt + 0x00) * 10000;
            info->vid     = nv_ro08(device, volt + 0x01) >> 2;
            break;
        case 0x40:
        case 0x50:
            break;
	}
	return volt;
}

static const u8 tags[8] = {
	DCB_GPIO_VID0, DCB_GPIO_VID1, DCB_GPIO_VID2, DCB_GPIO_VID3,
	DCB_GPIO_VID4, DCB_GPIO_VID5, DCB_GPIO_VID6, DCB_GPIO_VID7,
};

static int nouveau_voltgpio_get(struct nouveau_device *device)
{
    struct nouveau_volt *volt = &device->volt;

	u8 vid = 0;
	int i;
    
	for (i = 0; i < 8; i++) {
		if (volt->vid_mask & (1 << i)) {
			int ret = nouveau_gpio_get(device, 0, tags[i], 0xff);
			if (ret < 0)
				return ret;
			vid |= ret << i;
		}
	}
    
	return vid;
}

static int nouveau_voltgpio_init(struct nouveau_device *device)
{
    struct nouveau_volt *volt = &device->volt;
	struct dcb_gpio_func func;
	int i;
    
	/* check we have gpio function info for each vid bit.  on some
	 * boards (ie. nvs295) the vid mask has more bits than there
	 * are valid gpio functions... from traces, nvidia appear to
	 * just touch the existing ones, so let's mask off the invalid
	 * bits and continue with life
	 */
	for (i = 0; i < 8; i++) {
		if (volt->vid_mask & (1 << i)) {
			int ret = nouveau_gpio_find(device, 0, tags[i], 0xff, &func);
			if (ret) {
				if (ret != -ENOENT)
					return ret;
				nv_debug(device, "VID bit %d has no GPIO\n", i);
				volt->vid_mask &= ~(1 << i);
			}
		}
	}
    
	return 0;
}

static int nouveau_volt_get(struct nouveau_device *device)
{
    struct nouveau_volt *volt = &device->volt;
    
	if (volt->vid_get) {
		int ret = volt->vid_get(device), i;
		if (ret >= 0) {
			for (i = 0; i < volt->vid_nr; i++) {
				if (volt->vid[i].vid == ret)
					return volt->vid[i].uv;
			}
			ret = -EINVAL;
		}
		return ret;
	}
	return -ENODEV;
}

int nouveau_volt_create(struct nouveau_device *device)
{
	struct nouveau_volt *volt = &device->volt;
	struct nvbios_volt_entry ivid;
	struct nvbios_volt info;
	u8  ver, hdr, cnt, len;
	u16 data;
	int ret = -EUNKNOWN, i;
    
	volt->get = nouveau_volt_get;
    
	data = nvbios_volt_parse(device, &ver, &hdr, &cnt, &len, &info);
	if (data && info.vidmask && info.base && info.step) {
		for (i = 0; i < info.vidmask + 1; i++) {
			if (info.base >= info.min &&
			    info.base <= info.max) {
				volt->vid[volt->vid_nr].uv = info.base;
				volt->vid[volt->vid_nr].vid = i;
				volt->vid_nr++;
			}
			info.base += info.step;
		}
		volt->vid_mask = info.vidmask;
	} else
        if (data && info.vidmask) {
            for (i = 0; i < cnt; i++) {
                data = nvbios_volt_entry_parse(device, i, &ver, &hdr,
                                               &ivid);
                if (data) {
                    volt->vid[volt->vid_nr].uv = ivid.voltage;
                    volt->vid[volt->vid_nr].vid = ivid.vid;
                    volt->vid_nr++;
                }
            }
            volt->vid_mask = info.vidmask;
        }
    
	if (volt->vid_nr) {
		for (i = 0; i < volt->vid_nr; i++) {
			nv_debug(device, "VID %02x: %duv\n", volt->vid[i].vid, volt->vid[i].uv);
		}
        
		/*XXX: this is an assumption.. there probably exists boards
		 * out there with i2c-connected voltage controllers too..
		 */
		ret = nouveau_voltgpio_init(device);
        
		if (ret == 0) {
			volt->vid_get = nouveau_voltgpio_get;
		}
	}
    
	return ret;
}
