//
//  si.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

/*
 * Copyright 2011 Advanced Micro Devices, Inc.
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