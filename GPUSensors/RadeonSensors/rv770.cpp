//
//  rv770.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 07.01.13.
//
//

#include "rv770d.h"
#include "rv770.h"
#include "radeon.h"

int rv770_get_temp(struct radeon_device *rdev)
{
	u32 temp = (RREG32(CG_MULT_THERMAL_STATUS) & ASIC_T_MASK) >>
    ASIC_T_SHIFT;
	int actual_temp;
    
	if (temp & 0x400)
		actual_temp = -256;
	else if (temp & 0x200)
		actual_temp = 255;
	else if (temp & 0x100) {
		actual_temp = temp & 0x1ff;
		actual_temp |= ~0x1ff;
	} else
		actual_temp = temp & 0xff;
    
	return (actual_temp /** 1000*/) / 2;
}
