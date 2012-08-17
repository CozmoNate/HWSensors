//
//  nouveau_temp.c
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#include "nouveau.h"
#include "nouveau_bios.h"

#include "nouveau_temp.h"

static void nouveau_temp_vbios_parse(struct nouveau_device *device, u8 *temp)
{
	struct nouveau_pm_temp_sensor_constants *sensor = &device->sensor_constants;
	//struct nouveau_pm_threshold_temp *temps = &pm->threshold_temp;
	int i, headerlen, recordlen, entries;
    
	if (!temp) {
		nv_debug(device, "temperature table pointer invalid\n");
		return;
	}
    
	/* Set the default sensor's contants */
	sensor->offset_constant = 0;
	sensor->offset_mult = 0;
	sensor->offset_div = 1;
	sensor->slope_mult = 1;
	sensor->slope_div = 1;
    
	/* Set the default temperature thresholds */
//	temps->critical = 110;
//	temps->down_clock = 100;
//	temps->fan_boost = 90;
    
	/* Set the default range for the pwm fan */
//	pm->fan.min_duty = 30;
//	pm->fan.max_duty = 100;
    
	/* Set the known default values to setup the temperature sensor */
	if (device->card_type >= NV_40) {
		switch (device->chipset) {
            case 0x43:
                sensor->offset_mult = 32060;
                sensor->offset_div = 1000;
                sensor->slope_mult = 792;
                sensor->slope_div = 1000;
                break;
                
            case 0x44:
            case 0x47:
            case 0x4a:
                sensor->offset_mult = 27839;
                sensor->offset_div = 1000;
                sensor->slope_mult = 780;
                sensor->slope_div = 1000;
                break;
                
            case 0x46:
                sensor->offset_mult = -24775;
                sensor->offset_div = 100;
                sensor->slope_mult = 467;
                sensor->slope_div = 10000;
                break;
                
            case 0x49:
                sensor->offset_mult = -25051;
                sensor->offset_div = 100;
                sensor->slope_mult = 458;
                sensor->slope_div = 10000;
                break;
                
            case 0x4b:
                sensor->offset_mult = -24088;
                sensor->offset_div = 100;
                sensor->slope_mult = 442;
                sensor->slope_div = 10000;
                break;
                
            case 0x50:
                sensor->offset_mult = -22749;
                sensor->offset_div = 100;
                sensor->slope_mult = 431;
                sensor->slope_div = 10000;
                break;
                
            case 0x67:
                sensor->offset_mult = -26149;
                sensor->offset_div = 100;
                sensor->slope_mult = 484;
                sensor->slope_div = 10000;
                break;
		}
	}
    
	headerlen = temp[1];
	recordlen = temp[2];
	entries = temp[3];
	temp = temp + headerlen;
    
	/* Read the entries from the table */
	for (i = 0; i < entries; i++) {
		s16 value = ROM16(temp[1]);
        
		switch (temp[0]) {
            case 0x01:
                if ((value & 0x8f) == 0)
                    sensor->offset_constant = (value >> 9) & 0x7f;
                break;
                
            case 0x04:
//                if ((value & 0xf00f) == 0xa000) /* core */
//                    temps->critical = (value&0x0ff0) >> 4;
                break;
                
            case 0x07:
//                if ((value & 0xf00f) == 0xa000) /* core */
//                    temps->down_clock = (value&0x0ff0) >> 4;
                break;
                
            case 0x08:
//                if ((value & 0xf00f) == 0xa000) /* core */
//                    temps->fan_boost = (value&0x0ff0) >> 4;
                break;
                
            case 0x10:
                sensor->offset_mult = value;
                break;
                
            case 0x11:
                sensor->offset_div = value;
                break;
                
            case 0x12:
                sensor->slope_mult = value;
                break;
                
            case 0x13:
                sensor->slope_div = value;
                break;
            case 0x22:
//                pm->fan.min_duty = value & 0xff;
//                pm->fan.max_duty = (value & 0xff00) >> 8;
                break;
            case 0x26:
//                pm->fan.pwm_freq = value;
                break;
		}
		temp += recordlen;
	}
    
	//nouveau_temp_safety_checks(device);
    
	/* check the fan min/max settings */
//	if (pm->fan.min_duty < 10)
//		pm->fan.min_duty = 10;
//	if (pm->fan.max_duty > 100)
//		pm->fan.max_duty = 100;
//	if (pm->fan.max_duty < pm->fan.min_duty)
//		pm->fan.max_duty = pm->fan.min_duty;
}

static int nv40_sensor_setup(struct nouveau_device *device)
{
	//struct nouveau_pm *pm = nouveau_pm(dev);
	struct nouveau_pm_temp_sensor_constants *sensor = &device->sensor_constants;
	s32 offset = sensor->offset_mult / sensor->offset_div;
	s32 sensor_calibration;
    
	/* set up the sensors */
	sensor_calibration = 120 - offset - sensor->offset_constant;
	sensor_calibration = sensor_calibration * sensor->slope_div /
    sensor->slope_mult;
    
	if (device->chipset >= 0x46)
		sensor_calibration |= 0x80000000;
	else
		sensor_calibration |= 0x10000000;
    
	nv_wr32(device, 0x0015b0, sensor_calibration);
    
	/* Wait for the sensor to update */
	//msleep(5);
    IOSleep(5);
    
	/* read */
	return nv_rd32(device, 0x0015b4) & 0x1fff;
}

int nv40_diode_temp_get(struct nouveau_device *device)
{
	//struct nouveau_pm *pm = nouveau_pm(dev);
	struct nouveau_pm_temp_sensor_constants *sensor = &device->sensor_constants;
	int offset = sensor->offset_mult / sensor->offset_div;
	int core_temp;
    
	if (device->card_type >= NV_50) {
		core_temp = nv_rd32(device, 0x20008);
	} else {
		core_temp = nv_rd32(device, 0x0015b4) & 0x1fff;
		/* Setup the sensor if the temperature is 0 */
		if (core_temp == 0)
			core_temp = nv40_sensor_setup(device);
	}
    
	core_temp = core_temp * sensor->slope_mult / sensor->slope_div;
	core_temp = core_temp + offset + sensor->offset_constant;
    
	return core_temp;
}

int nv84_diode_temp_get(struct nouveau_device *device)
{
	return nv_rd32(device, 0x20400);
}

void nouveau_temp_init(struct nouveau_device *device)
{
	struct nvbios *bios = &device->vbios;
	struct bit_entry P;
	u8 *temp = NULL;
    
	if (bios->type == NVBIOS_BIT) {
		if (nouveau_bit_table(device, 'P', &P))
			return;
        
		if (P.version == 1)
			temp = ROMPTR(device, P.data[12]);
		else if (P.version == 2)
			temp = ROMPTR(device, P.data[16]);
		else
			nv_warn(device, "unknown temp for BIT P %d\n", P.version);
        
		nouveau_temp_vbios_parse(device, temp);
	}
}