/* NVClock Winbond W83781D hardware monitoring
*/
//#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "i2c.h"
#include "nvclock.h"


/* various defines for register offsets and such are needed */

#define W83781D_REG_LOCAL_TEMP 0x27
#define W83781D_REG_REMOTE_TEMP 0x27
#define W83781D_REG_FAN1_COUNT 0x28
#define W83781D_REG_MAN_ID 0x4f
#define ASUS_MAN_ID 0x12
#define W83781D_MAN_ID 0x5c
#define W83781D_REG_CHIP_ID 0x58
#define W83781D_REG_FAN_DIVISOR 0x47

/* This function should return the chip type .. */
int w83781d_detect(I2CDevPtr dev)
{
	I2CByte man_id, chip_id;

	xf86I2CReadByte  (dev, W83781D_REG_MAN_ID, &man_id); 
	xf86I2CReadByte  (dev, W83781D_REG_CHIP_ID, &chip_id); 

	switch(man_id)
	{
		case ASUS_MAN_ID:
		case W83781D_MAN_ID:
		/* We still need a chip_id check (0x11 for w83781d) */
			dev->chip_id = W83781D;
			dev->chip_name = (char*)STRDUP("Winbond W83781D", sizeof("Winbond W83781D"));
			break;
		default:
			IOLog("Uknown Winbond vendor: %x\n", man_id);
			return 0;
	}
	return 1;
}

int w83781d_get_board_temp(I2CDevPtr dev)
{
	I2CByte temp;
	xf86I2CReadByte(dev, W83781D_REG_LOCAL_TEMP, &temp);
	return temp;
}

/* only one temperature exists ... */
int w83781d_get_gpu_temp(I2CDevPtr dev)
{
	I2CByte temp;
	xf86I2CReadByte(dev, W83781D_REG_REMOTE_TEMP, &temp);
	return temp;
}

int w83781d_get_fanspeed_rpm(I2CDevPtr dev)
{
	I2CByte count, divisor;

	xf86I2CReadByte(dev, W83781D_REG_FAN1_COUNT, &count);
	xf86I2CReadByte(dev, W83781D_REG_FAN_DIVISOR, &divisor);
	divisor = 1 << ((divisor >> 4) & 0x3); /* bit 5:4 are for fan1; a value of 0 means a divider of 1, while 2 means 2^3 = 8 */
	
	/* A count of 0xff indicates that something is wrong i.e. no fan is connected */
	if(count == 0xff)
		return 0;
	
	return 1350000/(count * divisor);
}

float w83781d_get_fanspeed_pwm(I2CDevPtr dev)
{
	return 0;
}

int w83781d_set_fanspeed_pwm(I2CDevPtr dev, float speed)
{
	return 0;
}

