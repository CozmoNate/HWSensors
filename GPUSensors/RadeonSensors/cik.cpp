//
//  cik.c
//  HWSensors
//
//  Created by Kozlek on 07.12.13.
//
//

#include "cik.h"


#define     CG_MULT_THERMAL_STATUS      0xC0300014
#define		CTF_TEMP_MASK				0x0003fe00
#define		CTF_TEMP_SHIFT				9

/* get temperature in millidegrees */
int ci_get_temp(struct radeon_device *rdev)
{
	u32 temp;
	int actual_temp = 0;
    
	temp = (RREG32_SMC(CG_MULT_THERMAL_STATUS) & CTF_TEMP_MASK) >>
    CTF_TEMP_SHIFT;
    
	if (temp & 0x200)
		actual_temp = 255;
	else
		actual_temp = temp & 0x1ff;
    
	//actual_temp = actual_temp * 1000;
    
	return actual_temp;
}

/* get temperature in millidegrees */
int pl_get_temp(struct radeon_device *rdev)
{
	u32 temp;
	int actual_temp = 0;

	temp = (RREG32_IND(CG_MULT_THERMAL_STATUS) & CTF_TEMP_MASK) >>
    CTF_TEMP_SHIFT;

	if (temp & 0x200)
		actual_temp = 255;
	else
		actual_temp = temp & 0x1ff;

	//actual_temp = actual_temp * 1000;

	return actual_temp;
}

/* get temperature in millidegrees */
int kv_get_temp(struct radeon_device *rdev)
{
	u32 temp;
	int actual_temp = 0;
    
	temp = RREG32_SMC(0xC0300E0C);
    
	if (temp)
		actual_temp = (temp / 8) - 49;
	else
		actual_temp = 0;
    
	//actual_temp = actual_temp * 1000;
    
	return actual_temp;
}
