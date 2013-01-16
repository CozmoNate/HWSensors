//
//  rv770.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 07.01.13.
//
//

/*
 * Copyright 2008 Advanced Micro Devices, Inc.
 * Copyright 2008 Red Hat Inc.
 * Copyright 2009 Jerome Glisse.
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
 * Authors: Dave Airlie
 *          Alex Deucher
 *          Jerome Glisse
 */

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
