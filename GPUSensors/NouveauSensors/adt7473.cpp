/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2001-2005 Roderick Colenbrander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * ADT7473 hardware monitoring
 */

#include "nouveau_definitions.h"

#include "nvclock_i2c.h"

/* various defines for register offsets and such are needed */
#define ADT7473_REG_LOCAL_TEMP 0x26
#define ADT7473_REG_LOCAL_TEMP_OFFSET 0x70
#define ADT7473_REG_REMOTE_TEMP 0x25
#define ADT7473_REG_REMOTE_TEMP_OFFSET 0x71

#define ADT7473_REG_PWM1_CFG 0x5c
#define ADT7473_REG_PWM1_DUTYCYCLE 0x30
#define ADT7473_REG_PWM1_MAX_DUTYCYCLE 0x38
#define ADT7473_REG_PWM1_MIN_DUTYCYCLE 0x64

#define ADT7473_REG_TACH1_LB 0x28
#define ADT7473_REG_TACH1_HB 0x29

#define ADT7473_REG_CFG5 0x7c

#define ADT7473_REG_MAN_ID 0x3e
#define AD_MAN_ID 0x41
#define ADT7473_REG_DEVID2 0x3F
#define ADT7473_REG_CHIP_ID 0x3d
#define ADT7473_CHIP_ID 0x73

/* This function should return the chip type .. */
int adt7473_detect(I2CDevPtr dev)
{
	I2CByte man_id, dev_id, chip_id;
	
	xf86I2CReadByte(dev, ADT7473_REG_MAN_ID, &man_id);
    xf86I2CReadByte(dev, ADT7473_REG_DEVID2, &dev_id);
    
    if (man_id != AD_MAN_ID || (dev_id & 0xf8) != 0x68)
		return 0;
    
	xf86I2CReadByte(dev, ADT7473_REG_CHIP_ID, &chip_id);

    if (chip_id == 0x73) {
        dev->chip_id = ADT7473;
		dev->chip_name = (char*)STRDUP("Analog Devices ADT7473", sizeof("Analog Devices ADT7473"));
		return 1;
    }
	else if (chip_id == 0x75 && dev->SlaveAddr / 2 == 0x2e) {
        dev->chip_id = ADT7473;
		dev->chip_name = (char*)STRDUP("Analog Devices ADT7475", sizeof("Analog Devices ADT7475"));
		return 1;
    }
	else if (chip_id == 0x76) {
		dev->chip_id = ADT7473;
		dev->chip_name = (char*)STRDUP("Analog Devices ADT7476", sizeof("Analog Devices ADT7476"));
		return 1;
    }
	else if ((dev_id & 0xfc) == 0x6c) {
        dev->chip_id = ADT7473;
		dev->chip_name = (char*)STRDUP("Analog Devices ADT7490", sizeof("Analog Devices ADT7490"));
		return 1;
    }
	
	return 0;
}

int adt7473_get_board_temp(nouveau_device *device)
{
	I2CByte temp;
	I2CByte cfg;

	xf86I2CReadByte(device->nvclock_i2c_sensor, ADT7473_REG_LOCAL_TEMP, &temp);

	/* Check if the sensor uses 2-complement or offset-64 mode */
	xf86I2CReadByte(device->nvclock_i2c_sensor, ADT7473_REG_CFG5, &cfg);
	if(cfg & 0x1)
		return (int)((char)temp);
	else
		return temp - 64;
}

int adt7473_get_gpu_temp(nouveau_device *device)
{
	I2CByte temp;
	I2CByte cfg;
	int offset = 0;

	/* The temperature needs to be corrected using an offset which is stored in the bios.
	/  If no bios has been parsed we fall back to a default value.
	*/
	if(device->bios.data)
	{
		offset = device->sensor_constants.offset_constant;//nv_card->bios->sensor_cfg.temp_correction;
	}
	else
	{
		/* We add a 10C offset to the temperature though this isn't conform
		/  the ADT7473 datasheet. The reason we add this is to show a temperature
		/  similar to the internal gpu sensor. Right now the board and gpu
		/  temperature as reported by the sensor are about the same (there's 
		/  a difference between the two or 3-4C). Most likely the internal gpu
		/  temperature is a bit higher and assuming the temperature as reported
		/  by the internal sensor is correct adding a 10C offset is a good solution.
		/  Add an offset of 8C for 8*00/GTX2*0 cards but it doesn't seem 100% correct though.
		/  It could be that +7C is more correct for 8800GT cards.
		*/
		if(device->card_type == NV_40)
			offset = 10;
		else if(device->card_type == NV_50)
			offset = 8;
	}

	xf86I2CReadByte(device->nvclock_i2c_sensor, ADT7473_REG_REMOTE_TEMP, &temp);
	
	/* Check if the sensor uses 2-complement or offset-64 mode */

	xf86I2CReadByte(device->nvclock_i2c_sensor, ADT7473_REG_CFG5, &cfg);
	if(cfg & 0x1)
		return (int)((char)temp + offset);
	else
		return temp - 64 + offset;
}

int adt7473_get_fanspeed_rpm(nouveau_device *device)
{
	I2CByte count_lb, count_hb;
	int count;

	xf86I2CReadByte(device->nvclock_i2c_sensor, ADT7473_REG_TACH1_LB, &count_lb);
	xf86I2CReadByte(device->nvclock_i2c_sensor, ADT7473_REG_TACH1_HB, &count_hb);
	count = (count_hb << 8) | count_lb;

	/* GT200 boards seem to use two phases instead of a single, the fan speed is twice as high */
	if((device->chipset & 0x1f0) >= 0xA0)
		count *= 2;
	
	/* GF100 boards seem to use four phases... */
	if(device->card_type == NV_C0)
		count *= 4;

	/* RPM = 60*90k pulses / (number of counts that fit in a pulse) */
	return 90000*60/count;
}

int adt7473_get_fanspeed_pwm(nouveau_device *device)
{
	I2CByte value;

	xf86I2CReadByte(device->nvclock_i2c_sensor, ADT7473_REG_PWM1_DUTYCYCLE, &value);
	return ((float)value*100/255);
}

int adt7473_set_fanspeed_pwm(I2CDevPtr dev, float speed)
{
	I2CByte value = (int)speed * 255/100;
	I2CByte cfg, max_dutycycle;
	
	xf86I2CReadByte(dev, ADT7473_REG_PWM1_CFG, &cfg);
	cfg |= 0xe0; /* Put PWM1 in manual mode; this disables automatic control */
	xf86I2CWriteByte(dev, ADT7473_REG_PWM1_CFG, cfg);
	
	/* If the MAX dutycycle is lower than 0xff (100%), set it to 0xff */
	xf86I2CReadByte(dev, ADT7473_REG_PWM1_MAX_DUTYCYCLE, &max_dutycycle);
	if(max_dutycycle < 0xff)
		xf86I2CWriteByte(dev, ADT7473_REG_PWM1_MAX_DUTYCYCLE, 0xff);
	
	xf86I2CWriteByte(dev, ADT7473_REG_PWM1_DUTYCYCLE, value);
	return 1;
}

int adt7473_get_fanspeed_mode(I2CDevPtr dev) {
	I2CByte cfg;
	xf86I2CReadByte(dev, ADT7473_REG_PWM1_CFG, &cfg);
	
	if(cfg & (0x6 << 5)) return 0; /* auto */
	if(cfg & (0x7 << 5)) return 1; /* manual */
	
	return -1;  /* something went wrong */
}

void adt7473_set_fanspeed_mode(I2CDevPtr dev, int mode) {
	I2CByte cfg;
	xf86I2CReadByte(dev, ADT7473_REG_PWM1_CFG, &cfg);
	
	/* Clear the pwm1 config bits */
	cfg&=~(0xF << 5); 

	if(mode==1)
		cfg|=0x7 << 5; /* manual */
	else
		cfg|=0x6 << 5; /* auto */

	xf86I2CWriteByte(dev, ADT7473_REG_PWM1_CFG, cfg);
}
