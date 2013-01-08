//
//  radeon_atombios.h
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

#ifndef __HWSensors__radeon_atombios__
#define __HWSensors__radeon_atombios__

#include "radeon.h"
#include "atombios.h"

bool radeon_atombios_init(struct radeon_device *rdev);
void radeon_atombios_get_power_modes(struct radeon_device *rdev);

#endif /* defined(__HWSensors__radeon_atombios__) */
