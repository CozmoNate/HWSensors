//
//  si.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

#include "si.h"

/* get temperature in millidegrees */
int si_get_temp(struct radeon_device *rdev)
{
	u32 temp;
	int actual_temp = 0;
    
	temp = (RREG32(CG_MULT_THERMAL_STATUS) & CTF_TEMP_MASK) >> CTF_TEMP_SHIFT;
    
	if (temp & 0x200)
		actual_temp = 255;
	else
		actual_temp = temp & 0x1ff;
    
	//actual_temp = (actual_temp * 1000);
    
	return actual_temp;
}