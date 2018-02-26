//
//  cik.h
//  HWSensors
//
//  Created by Kozlek on 07.12.13.
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

#ifndef HWSensors_cik_h
#define HWSensors_cik_h

#include "radeon.h"

#define     RREG32_SMC(reg)             tn_smc_rreg(rdev, (reg))
#define     RREG32_IND(reg)             mm_smc_rreg(rdev, (reg))
#define     TN_SMC_IND_INDEX_0          0x200
#define     TN_SMC_IND_DATA_0           0x204
#define     MM_SMC_IND_INDEX_11          (0x1AC * 4)
#define     MM_SMC_IND_DATA_11           (0x1AD * 4)

static inline u32 tn_smc_rreg(struct radeon_device *rdev, u32 reg)
{
	//unsigned long flags;
	u32 r;
    
	//spin_lock_irqsave(&rdev->smc_idx_lock, flags);
	WREG32(TN_SMC_IND_INDEX_0, (reg));
	r = RREG32(TN_SMC_IND_DATA_0);
    
	//spin_unlock_irqrestore(&rdev->smc_idx_lock, flags);
	return r;
}

static inline u32 mm_smc_rreg(struct radeon_device *rdev, u32 reg)
{
	//unsigned long flags;
	u32 r;

	//spin_lock_irqsave(&rdev->smc_idx_lock, flags);
	WREG32(MM_SMC_IND_INDEX_11, (reg));
	r = RREG32(MM_SMC_IND_DATA_11);

	//spin_unlock_irqrestore(&rdev->smc_idx_lock, flags);
	return r;
}


int ci_get_temp(struct radeon_device *rdev);
int pl_get_temp(struct radeon_device *rdev);
int kv_get_temp(struct radeon_device *rdev);

#endif
