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

static const u8 vidtag[] = { 0x04, 0x05, 0x06, 0x1a, 0x73 };
static int nr_vidtag = sizeof(vidtag) / sizeof(vidtag[0]);

void nouveau_volt_init(struct nouveau_device *device)
{
	struct dcb_gpio_func func;
	struct bit_entry P;
	u8 *volt = NULL, *entry;
	int i, headerlen, recordlen, entries, vidmask, vidshift;
    
	if (device->vbios.type == NVBIOS_BIT) {
		if (nouveau_bit_table(device, 'P', &P))
			return;
        
		if (P.version == 1)
			volt = ROMPTR(device, P.data[16]);
		else
            if (P.version == 2)
                volt = ROMPTR(device, P.data[12]);
            else {
                nv_warn(device, "unknown volt for BIT P %d\n", P.version);
            }
	} else {
		if (device->vbios.data[device->vbios.offset + 6] < 0x27) {
			nv_debug(device, "BMP version too old for voltage\n");
			return;
		}
        
		volt = ROMPTR(device, device->vbios.data[device->vbios.offset + 0x98]);
	}
    
	if (!volt) {
		nv_debug(device, "voltage table pointer invalid\n");
		return;
	}
    
	switch (volt[0]) {
        case 0x10:
        case 0x11:
        case 0x12:
            headerlen = 5;
            recordlen = volt[1];
            entries   = volt[2];
            vidshift  = 0;
            vidmask   = volt[4];
            break;
        case 0x20:
            headerlen = volt[1];
            recordlen = volt[3];
            entries   = volt[2];
            vidshift  = 0; /* could be vidshift like 0x30? */
            vidmask   = volt[5];
            break;
        case 0x30:
            headerlen = volt[1];
            recordlen = volt[2];
            entries   = volt[3];
            vidmask   = volt[4];
            /* no longer certain what volt[5] is, if it's related to
             * the vid shift then it's definitely not a function of
             * how many bits are set.
             *
             * after looking at a number of nva3+ vbios images, they
             * all seem likely to have a static shift of 2.. lets
             * go with that for now until proven otherwise.
             */
            vidshift  = 2;
            break;
        case 0x40:
            headerlen = volt[1];
            recordlen = volt[2];
            entries   = volt[3]; /* not a clue what the entries are for.. */
            vidmask   = volt[11]; /* guess.. */
            vidshift  = 0;
            break;
        default:
            nv_warn(device, "voltage table 0x%02x unknown\n", volt[0]);
            return;
	}
    
	/* validate vid mask */
	device->voltage.vid_mask = vidmask;
	if (!device->voltage.vid_mask) {
        nv_debug(device, "voltage vidmask is insane 0x%02x\n", vidmask);
		return;
    }
    
	i = 0;
	while (vidmask) {
		if (i > nr_vidtag) {
			nv_debug(device, "vid bit %d unknown\n", i);
			return;
		}
        
		if (!nouveau_gpio_find(device, 0, vidtag[i], 0xff, &func)) {
			nv_debug(device, "vid bit %d has no gpio tag\n", i);
			return;
		}
        
		vidmask >>= 1;
		i++;
	}
    
	/* parse vbios entries into common format */
	device->voltage.version = volt[0];
	if (device->voltage.version < 0x40) {
		device->voltage.nr_level = entries;
		device->voltage.level = (nouveau_pm_voltage_level*)IOMalloc(device->voltage.nr_level * sizeof(nouveau_pm_voltage_level));
		if (!device->voltage.level) {
            nv_debug(device, "can't allocate voltage structure\n");
			return;
        }
        
		entry = volt + headerlen;
		for (i = 0; i < entries; i++, entry += recordlen) {
			device->voltage.level[i].voltage = entry[0] * 10000;
			device->voltage.level[i].vid     = entry[1] >> vidshift;
		}
	} else {
		u32 volt_uv = ROM32(volt[4]);
		s16 step_uv = ROM16(volt[8]);
		u8 vid;
        
		device->voltage.nr_level = device->voltage.vid_mask + 1;
		device->voltage.level = (nouveau_pm_voltage_level*)IOMalloc(device->voltage.nr_level * sizeof(nouveau_pm_voltage_level));
		if (!device->voltage.level) {
            nv_debug(device, "can't allocate voltage structure\n");
			return;
        }
        
		for (vid = 0; vid <= device->voltage.vid_mask; vid++) {
			device->voltage.level[vid].voltage = volt_uv;
			device->voltage.level[vid].vid = vid;
			volt_uv += step_uv;
		}
	}
    
	device->voltage.supported = true;
}

static int nouveau_volt_lvl_lookup(struct nouveau_device *device, int vid)
{
	int i;
    
	for (i = 0; i < device->voltage.nr_level; i++) {
		if (device->voltage.level[i].vid == vid)
			return device->voltage.level[i].voltage;
	}
    
	return 0;
}

int nouveau_voltage_gpio_get(struct nouveau_device *device)
{
	u8 vid = 0;
	int i;
    
	for (i = 0; i < nr_vidtag; i++) {
		if (!(device->voltage.vid_mask & (1 << i)))
			continue;
        
		vid |= nouveau_gpio_get(device, 0, vidtag[i], 0xff) << i;
	}
    
	return nouveau_volt_lvl_lookup(device, vid);
}