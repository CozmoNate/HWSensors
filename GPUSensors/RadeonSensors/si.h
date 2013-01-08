//
//  si.h
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

#ifndef __HWSensors__si__
#define __HWSensors__si__

#include "radeon.h"

#define     CG_MULT_THERMAL_STATUS		0x714
#define		CTF_TEMP(x)					((x) << 9)
#define		CTF_TEMP_MASK				0x0003fe00
#define		CTF_TEMP_SHIFT				9

int si_get_temp(struct radeon_device *rdev);

#endif /* defined(__HWSensors__si__) */
