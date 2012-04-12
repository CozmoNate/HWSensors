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
#include <string.h>
#include "i2c.h"
#include "nvclock.h"

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
#define ADT7473_REG_CHIP_ID 0x3d
	#define ADT7473_CHIP_ID 0x73

/* This function should return the chip type .. */
int adt7473_detect(I2CDevPtr dev)
{
	I2CByte man_id, chip_id;
	
	xf86I2CReadByte(dev, ADT7473_REG_MAN_ID, &man_id);
	xf86I2CReadByte(dev, ADT7473_REG_CHIP_ID, &chip_id);

	if((man_id == AD_MAN_ID) && (chip_id == ADT7473_CHIP_ID))
	{
		dev->chip_id = ADT7473;
		dev->chip_name = (char*)STRDUP("Analog Devices ADT7473", sizeof("Analog Devices ADT7473"));
		return 1;
	}
	
	return 0;
}

int adt7473_get_board_temp(I2CDevPtr dev)
{
	I2CByte temp;
	I2CByte cfg;

	xf86I2CReadByte(dev, ADT7473_REG_LOCAL_TEMP, &temp);

	/* Check if the sensor uses 2-complement or offset-64 mode */
	xf86I2CReadByte(dev, ADT7473_REG_CFG5, &cfg);
	if(cfg & 0x1)
		return (int)((char)temp);
	else
		return temp - 64;
}

int adt7473_get_gpu_temp(I2CDevPtr dev)
{
	I2CByte temp;
	I2CByte cfg;
	int offset = 0;

	/* The temperature needs to be corrected using an offset which is stored in the bios.
	/  If no bios has been parsed we fall back to a default value.
	*/
	if(nv_card->bios)
	{
		offset = nv_card->bios->sensor_cfg.temp_correction;
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
		if(dev->arch & NV4X)
			offset = 10;
		else if(dev->arch & NV5X)
			offset = 8;
	}

	xf86I2CReadByte(dev, ADT7473_REG_REMOTE_TEMP, &temp);
	
	/* Check if the sensor uses 2-complement or offset-64 mode */

	xf86I2CReadByte(dev, ADT7473_REG_CFG5, &cfg);
	if(cfg & 0x1)
		return (int)((char)temp + offset);
	else
		return temp - 64 + offset;
}

int adt7473_get_fanspeed_rpm(I2CDevPtr dev)
{
	I2CByte count_lb, count_hb;
	int count;

	xf86I2CReadByte(dev, ADT7473_REG_TACH1_LB, &count_lb);
	xf86I2CReadByte(dev, ADT7473_REG_TACH1_HB, &count_hb);
	count = (count_hb << 8) | count_lb;

	/* GT200 boards seem to use two phases instead of a single, the fan speed is twice as high */
	if(dev->arch & GT200)
		count *= 2;
	
	/* GF100 boards seem to use four phases... */
	if(dev->arch & GF100)
		count *= 4;

	/* RPM = 60*90k pulses / (number of counts that fit in a pulse) */
	return 90000*60/count;
}

float adt7473_get_fanspeed_pwm(I2CDevPtr dev)
{
	I2CByte value;

	xf86I2CReadByte(dev, ADT7473_REG_PWM1_DUTYCYCLE, &value);
	return (float)value*100/255;
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
