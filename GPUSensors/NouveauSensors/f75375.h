/* Fintek F75375 sensor module
/  Copyright(C) 2005, Matt Wright
*/
//#include "nvclock.h"
#include "nvclock_i2c.h"

int debug;
typedef struct _fan_vtemp
{
	int temp[4];
	int speed[5];
} fan_vtemp;


#define dbg_printf(x) if (debug==1) IOLog(x)

#define MODE_SPEED			0x00
#define MODE_TEMP			0x10
#define MODE_PWM			0x20

int f75375_get_gpu_fanmode(I2CDevPtr dev);
int f75375_set_gpu_fanmode(I2CDevPtr dev, I2CByte mode);
int f75375_set_gpu_tempctl(I2CDevPtr dev, fan_vtemp speeds);
int f75375_get_gpu_tempctl(I2CDevPtr dev, fan_vtemp *speeds);

#define FINTEK_VENDOR1			0x5d
#define FINTEK_VENDOR2			0x5e

#define	ASUS_NV40_CHIPID_H		0x5a
#define	ASUS_NV40_CHIPID_L		0x5b

#define	F75375S_VRAM_VCC		0x10
#define F75375S_VRAM_V1			0x11
#define	F75375S_VRAM_V2			0x12
#define	F75375S_VRAM_V3			0x13
#define F75375S_VRAM_TEMP1		0x14
#define F75375S_VRAM_TEMP2		0x15
#define F75375S_VRAM_FAN1_MSB		0x16
#define	F75375S_VRAM_FAN1_LSB		0x17
#define	F75375S_VRAM_FAN2_MSB		0x18
#define	F75375S_VRAM_FAN2_LSB		0x19

#define F75375S_FAN1_PWM		0x76
#define F75375S_FAN1_COUNT_H		0x16
#define F75375S_FAN1_COUNT_L		0x17
#define F75375S_FAN1_MODE		0x60
#define F75375S_FAN1_EXPECT_H		0x74
#define F75375S_FAN1_EXPECT_L		0x75

#define F75375S_TEMP_GPU		0x14
#define F75375S_TEMP_RAM		0x15

#define F75375S_VT1_B1			0xa0
#define F75375S_VT1_B2			0xa1
#define F75375S_VT1_B3			0xa2
#define F75375S_VT1_B4			0xa3

#define F75375S_VT1_S1_H		0xa4
#define F75375S_VT1_S1_L		0xa5
#define F75375S_VT1_S2_H		0xa6
#define F75375S_VT1_S2_L		0xa7
#define F75375S_VT1_S3_H		0xa8
#define F75375S_VT1_S3_L		0xa9
#define F75375S_VT1_S4_H		0xaa
#define F75375S_VT1_S4_L		0xab
#define F75375S_VT1_S5_H		0xac
#define F75375S_VT1_S5_L		0xad

#define FAN_TO_RPM(msb, lsb) 		(1500000/((msb<<8)+lsb))
#define RPM_TO_FAN(x)			(1500000/x)
#define MERGE_BYTE(msb, lsb) 		((msb<<8)+lsb)
