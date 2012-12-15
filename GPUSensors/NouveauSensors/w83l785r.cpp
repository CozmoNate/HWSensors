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
 * W83L785R hardware monitoring
 */
#include <string.h>
#include "nvclock_i2c.h"
//#include "nvclock.h"


/* various defines for register offsets and such are needed */

#define W83L785R_REG_LOCAL_TEMP 0x26
#define W83L785R_REG_LOCAL_TEMP_OFFSET 0x85
#define W83L785R_REG_REMOTE_TEMP 0x27
#define W83L785R_REG_REMOTE_TEMP_OFFSET 0x86

#define W83L785R_REG_FAN1_COUNT 0x28
#define W83L785R_REG_FAN2_COUNT 0x29
#define W83L785R_REG_FAN_DIVISOR 0x47 /* bit 2-0 for fan1; bit 6-4 for fan2 */

#define W83L785R_REG_MAN_ID_L 0x4c
#define W83L785R_REG_MAN_ID_H 0x4d
#define W83L785R_REG_CHIP_ID 0x4e

#define W83L785R_REG_FAN1_PWM 0x81
#define W83L785R_REG_FAN2_PWM 0x83

/* This function should return the chip type .. */
int w83l785r_detect(I2CDevPtr dev)
{
	I2CByte man_id_l, man_id_h, chip_id;

	xf86I2CReadByte(dev, W83L785R_REG_MAN_ID_L, &man_id_l); 
	xf86I2CReadByte(dev, W83L785R_REG_MAN_ID_H, &man_id_h); 
	xf86I2CReadByte(dev, W83L785R_REG_CHIP_ID, &chip_id); 

	/* Winbond chip */  
	if((man_id_l == 0xa3) && (man_id_h == 0x5c))
	{
		if((chip_id & 0xfe) == 0x60)
		{
			dev->chip_id = W83L785R;
			dev->chip_name = (char*)STRDUP("Winbond W83L785R", sizeof("Winbond W83L785R"));
			return 1;
		}
	}
  
	return 0;
}

int w83l785r_get_board_temp(nouveau_device *device)
{
	I2CByte temp, offset;
	xf86I2CReadByte(device->nvclock_i2c_sensor, W83L785R_REG_LOCAL_TEMP, &temp);
	xf86I2CReadByte(device->nvclock_i2c_sensor, W83L785R_REG_LOCAL_TEMP_OFFSET, &offset);
	return temp + offset;
}

int w83l785r_get_gpu_temp(nouveau_device *device)
{
	I2CByte temp, offset;
	xf86I2CReadByte(device->nvclock_i2c_sensor, W83L785R_REG_REMOTE_TEMP, &temp);
	xf86I2CReadByte(device->nvclock_i2c_sensor, W83L785R_REG_REMOTE_TEMP_OFFSET, &offset);
	return temp + offset;
}

int w83l785r_get_fanspeed_rpm(nouveau_device *device)
{
	I2CByte count, divisor;

	xf86I2CReadByte(device->nvclock_i2c_sensor, W83L785R_REG_FAN1_COUNT, &count);
	xf86I2CReadByte(device->nvclock_i2c_sensor, W83L785R_REG_FAN_DIVISOR, &divisor);
	divisor &= 0x7;

	/* By default count useally is 153, it seems that a value of 255 means that something is wrong.
	/  For example it retuns this value on boards on which the fan is replaced with a heatpipe and because
	/  of that the fan was removed.
	*/
	if(count == 0xff)
		return 0;

	return 1350000/(count * (1<<divisor));
}

int w83l785r_get_fanspeed_pwm(nouveau_device *device)
{
	I2CByte value;

	xf86I2CReadByte(device->nvclock_i2c_sensor, W83L785R_REG_FAN1_PWM, &value);
	return (float)(0xff-value)*100/256;
}

int w83l785r_set_fanspeed_pwm(I2CDevPtr dev, float speed)
{
	/* We have 8 bits to adjust the duty cycle, to set a speed of 100% we need to write a value of 0
	/  as the register is inverted. (0 means 100% and 0xff means 0)
	/
	/  Below we scale the speed (in %) with 255/100 to turn '100%' into the max value 255.
	*/
	I2CByte value = (100 - (int)speed) * 255/100;

	xf86I2CWriteByte(dev, W83L785R_REG_FAN1_PWM, value);
	xf86I2CReadByte(dev, W83L785R_REG_FAN1_PWM, &value);
	return 1;
}
