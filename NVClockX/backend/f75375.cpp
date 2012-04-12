/* Fintek F75375 sensor module
/  Copyright(C) 2005, Matt Wright
*/

#include <string.h>
#include "i2c.h"
#include "f75375.h"

int f75375_detect(I2CDevPtr dev)
{
	I2CByte nvl, nvh;

	xf86I2CReadByte(dev, FINTEK_VENDOR1, &nvl);
	xf86I2CReadByte(dev, FINTEK_VENDOR2, &nvh);

	if (MERGE_BYTE(nvh, nvl) != 0x3419)
	{
		return 0;
	}

	xf86I2CReadByte(dev, ASUS_NV40_CHIPID_H, &nvh);
	xf86I2CReadByte(dev, ASUS_NV40_CHIPID_L, &nvl);

	if (MERGE_BYTE(nvh, nvl) == 0x0306)
	{
		dev->chip_id = F75375;
		dev->chip_name = (char*)strdup("Fintek F75375S");
		return 1;
	}
	if (MERGE_BYTE(nvh, nvl) == 0x0204)
	{
		dev->chip_id = F75375;
		dev->chip_name = (char*)strdup("Fintek F75373S");
		return 1;
	}
	return 0;
}


int f75375_get_gpu_temp(I2CDevPtr dev)
{
	I2CByte nvh;

	xf86I2CReadByte(dev, F75375S_TEMP_GPU, &nvh);
	return (int)nvh;
}


int f75375_get_fanspeed_rpm(I2CDevPtr dev)
{
	I2CByte nvh, nvl;
	int rpm;

	xf86I2CReadByte(dev, F75375S_FAN1_COUNT_H, &nvh);
	xf86I2CReadByte(dev, F75375S_FAN1_COUNT_L, &nvl);

	rpm = FAN_TO_RPM(nvh, nvl);

	return rpm;
}


int f75375_get_board_temp(I2CDevPtr dev)
{
	I2CByte nvh;

	xf86I2CReadByte(dev, F75375S_TEMP_RAM, &nvh);
	return (int)nvh;
}


int f75375_set_fanspeed_rpm(I2CDevPtr dev, int desired_rpm)
{
	I2CByte nvh, nvl;
	int desired_count;

	desired_count = RPM_TO_FAN(desired_rpm);

	nvh = (desired_count>>8) & 0x00ff;
	nvl = (desired_count) & 0x00ff;

	xf86I2CWriteByte(dev, F75375S_FAN1_EXPECT_H, nvh);
	xf86I2CWriteByte(dev, F75375S_FAN1_EXPECT_L, nvl);
	return 1;
}


int f75375_set_gpu_tempctl(I2CDevPtr dev, fan_vtemp speeds)
{
	I2CByte nvh, nvl;
	int i, temp_speed;

	for (i=0; i<4; i++)
	{
		xf86I2CWriteByte(dev, F75375S_VT1_B1 + i, speeds.temp[i]);
	}

	for (i=0; i<5; i++)
	{

		temp_speed = RPM_TO_FAN(speeds.speed[i]);

		nvh = (temp_speed >> 8) & 0x00ff;
		nvl = (temp_speed) & 0x00ff;

		xf86I2CWriteByte(dev, F75375S_VT1_S1_H + (i*2), nvh);
		xf86I2CWriteByte(dev, F75375S_VT1_S1_L + (i*2), nvl);
	}

	return 0;
}


int f75375_get_gpu_tempctl(I2CDevPtr dev, fan_vtemp *speeds)
{
	I2CByte nvh, nvl;
	int i;

	for (i=0; i<4; i++)
	{
		xf86I2CReadByte(dev, F75375S_VT1_B1 + i, &nvl);
		speeds->temp[i] = (int)nvl;
	}

	for (i=0; i<5; i++)
	{
		xf86I2CReadByte(dev, F75375S_VT1_S1_H + (i*2), &nvh);
		xf86I2CReadByte(dev, F75375S_VT1_S1_L + (i*2), &nvl);
		speeds->speed[i] = FAN_TO_RPM(nvh, nvl);
	}

	return 0;
}


int f75375_get_gpu_fanmode(I2CDevPtr dev)
{
	I2CByte mode;

	xf86I2CReadByte(dev, F75375S_FAN1_MODE, &mode);
	return (int)mode;
}


int f75375_set_gpu_fanmode(I2CDevPtr dev, I2CByte mode)
{
	xf86I2CWriteByte(dev, F75375S_FAN1_MODE, (I2CByte)mode);
	return 0;
}


float f75375_get_fanspeed_pwm(I2CDevPtr dev)
{
	I2CByte speed;

	xf86I2CReadByte(dev, F75375S_FAN1_PWM, &speed);
	return (float)speed*100/256;
}


int f75375_set_fanspeed_pwm(I2CDevPtr dev, float speed)
{
	I2CByte value = (I2CByte)(speed * 255/100);

	xf86I2CWriteByte(dev, F75375S_FAN1_PWM, value);
	return 0;
}
