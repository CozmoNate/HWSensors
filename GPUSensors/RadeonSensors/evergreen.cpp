//
//  evergreen.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 07.01.13.
//
//

/*
 * Copyright 2010 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Alex Deucher
 */

#include "evergreend.h"
#include "evergreen.h"
#include "radeon.h"

int evergreen_get_temp(struct radeon_device *rdev)
{
	u32 temp, toffset;
	int actual_temp = 0;
    
	if (rdev->family == CHIP_FAMILY_JUNIPER) {
		toffset = (RREG32(CG_THERMAL_CTRL) & TOFFSET_MASK) >>
        TOFFSET_SHIFT;
		temp = (RREG32(CG_TS0_STATUS) & TS0_ADC_DOUT_MASK) >>
        TS0_ADC_DOUT_SHIFT;
        
		if (toffset & 0x100)
			actual_temp = temp / 2 - (0x200 - toffset);
		else
			actual_temp = temp / 2 + toffset;
        
		//actual_temp = actual_temp * 1000;
        
	} else {
		temp = (RREG32(CG_MULT_THERMAL_STATUS) & ASIC_T_MASK) >>
        ASIC_T_SHIFT;
        
		if (temp & 0x400)
			actual_temp = -256;
		else if (temp & 0x200)
			actual_temp = 255;
		else if (temp & 0x100) {
			actual_temp = temp & 0x1ff;
			actual_temp |= ~0x1ff;
		} else
			actual_temp = temp & 0xff;
        
		actual_temp = /*(*/actual_temp/* * 1000)*/ / 2;
	}
    
	return actual_temp;
}

int sumo_get_temp(struct radeon_device *rdev)
{
	u32 temp = RREG32(CG_THERMAL_STATUS) & 0xff;
	int actual_temp = temp - 49;
    
	return actual_temp/* * 1000*/;
}
