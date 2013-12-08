//
//  nouveau_temp.c
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

/*
 * Copyright 2010 PathScale inc.
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
 * Authors: Martin Peres
 */

#include "nouveau.h"
#include "nouveau_bios.h"

#include "nouveau_therm.h"

#include "timer.h"

static u16 therm_table(struct nouveau_device *device, u8 *ver, u8 *hdr, u8 *len, u8 *cnt)
{
    struct bit_entry bit_P;
    u16 therm = 0;
    
    if (!nouveau_bit_entry(device, 'P', &bit_P)) {
        if (bit_P.version == 1)
            therm = nv_ro16(device, bit_P.offset + 12);
        else if (bit_P.version == 2)
            therm = nv_ro16(device, bit_P.offset + 16);
        else
            nv_error(device,
                     "unknown offset for thermal in BIT P %d\n",
                     bit_P.version);
    }
    
    /* exit now if we haven't found the thermal table */
    if (!therm)
        return 0x0000;
    
    *ver = nv_ro08(device, therm + 0);
    *hdr = nv_ro08(device, therm + 1);
    *len = nv_ro08(device, therm + 2);
    *cnt = nv_ro08(device, therm + 3);
    
    return therm + nv_ro08(device, therm + 1);
}

static u16 nvbios_therm_entry(struct nouveau_device *device, int idx, u8 *ver, u8 *len)
{
    u8 hdr, cnt;
    u16 therm = therm_table(device, ver, &hdr, len, &cnt);
    
    if (therm && idx < cnt)
    	return therm + idx * *len;
    
    return 0x0000;
}

static int nvbios_therm_sensor_parse(struct nouveau_device *device, nvbios_therm_sensor *sensor)
{
	s8 thrs_section, sensor_section, offset;
	u8 ver, len, i;
	u16 entry;
    
    
	/* Read the entries from the table */
	thrs_section = 0;
	sensor_section = -1;
	i = 0;
	while ((entry = nvbios_therm_entry(device, i++, &ver, &len))) {
		s16 value = nv_ro16(device, entry + 1);
        
		switch (nv_ro08(device, entry + 0)) {
            case 0x0:
                thrs_section = value;
                if (value > 0)
                    return 0; /* we do not try to support ambient */
                break;
            case 0x01:
                sensor_section++;
                if (sensor_section == 0) {
                    offset = ((s8) nv_ro08(device, entry + 2)) / 2;
                    sensor->offset_constant = offset;
                }
                break;
                
            case 0x04:
//                if (thrs_section == 0) {
//                    sensor->thrs_critical.temp = (value & 0xff0) >> 4;
//                    sensor->thrs_critical.hysteresis = value & 0xf;
//                }
                break;
                
            case 0x07:
//                if (thrs_section == 0) {
//                    sensor->thrs_down_clock.temp = (value & 0xff0) >> 4;
//                    sensor->thrs_down_clock.hysteresis = value & 0xf;
//                }
                break;
                
            case 0x08:
//                if (thrs_section == 0) {
//                    sensor->thrs_fan_boost.temp = (value & 0xff0) >> 4;
//                    sensor->thrs_fan_boost.hysteresis = value & 0xf;
//                }
                break;
                
            case 0x10:
                if (sensor_section == 0)
                    sensor->offset_num = value;
                break;
                
            case 0x11:
                if (sensor_section == 0)
                    sensor->offset_den = value;
                break;
                
            case 0x12:
                if (sensor_section == 0)
                    sensor->slope_mult = value;
                break;
                
            case 0x13:
                if (sensor_section == 0)
                    sensor->slope_div = value;
                break;
            case 0x32:
//                if (thrs_section == 0) {
//                    sensor->thrs_shutdown.temp = (value & 0xff0) >> 4;
//                    sensor->thrs_shutdown.hysteresis = value & 0xf;
//                }
                break;
		}
	}
    
	return 0;
}

void nouveau_therm_init(struct nouveau_device *device)
{
    nouveau_pm_temp_sensor_constants *sensor = &device->sensor_constants;
    nvbios_therm_sensor bios_sensor;
    
    /* store some safe defaults */
	sensor->offset_constant = 0;
	sensor->offset_num = 0;
	sensor->offset_den = 1;
	sensor->slope_mult = 1;
	sensor->slope_div = 1;
    
	if (!nvbios_therm_sensor_parse(device, &bios_sensor)) {
		sensor->slope_mult = bios_sensor.slope_mult;
		sensor->slope_div = bios_sensor.slope_div;
		sensor->offset_num = bios_sensor.offset_num;
		sensor->offset_den = bios_sensor.offset_den;
		sensor->offset_constant = bios_sensor.offset_constant;
	}
}

int nouveau_therm_fan_pwm_get(struct nouveau_device *device)
{
	u32 divs, duty;
	int ret;
    
    if (device->fan_pwm.func != DCB_GPIO_UNUSED) {
        ret = device->pwm_get(device, device->fan_pwm.line, &divs, &duty);
        if (ret == 0 && divs) {
            divs = max(divs, duty);
            if (device->card_type <= NV_40 || (device->fan_pwm.log[0] & 1))
                duty = divs - duty;
            return (duty * 100) / divs;
        }
        
        return device->gpio_get(device, 0, device->fan_pwm.func, device->fan_pwm.line) * 100;
    }
    
    return -EIO;
}

#define THERM_FAN_SENSE_CYCLES 8

int nouveau_therm_fan_rpm_get(struct nouveau_device *device)
{
	u32 cycles, cur, prev, stop = THERM_FAN_SENSE_CYCLES * 4 + 1;
	u64 start, interval;

	if (device->fan_tach.func != DCB_GPIO_UNUSED) {
        /* Time a complete rotation and extrapolate to RPM:
         * When the fan spins, it changes the value of GPIO FAN_SENSE.
         * We get 4 changes (0 -> 1 -> 0 -> 1) per complete rotation.
         */
        start = ptimer_read();

        prev = device->gpio_get(device, 0, device->fan_tach.func, device->fan_tach.line);
        cycles = 0;
        do {
            IODelay(750); /* supports 0 < rpm < 7500 */
            
            cur = device->gpio_get(device, 0, device->fan_tach.func, device->fan_tach.line);
            if (prev != cur) {
                if (!start)
                    start = ptimer_read();
                cycles++;
                prev = cur;
            }
            
            interval = ptimer_read() - start;
            
        } while (cycles < stop && interval < 500000000);
        
        if (interval) {
            return (int)(((u64)60000000000ULL * (((u64)cycles - 1) / 4)) / interval);
        } else
            return -EIO;
    }
    
    nv_debug(device, "DCB_GPIO_FAN_SENSE func not found\n");
    
    return -EIO;
}
