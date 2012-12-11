//
//  nvd0.h
//  HWSensors
//
//  Created by Kozlek on 11.08.12.
//
//

#ifndef __HWSensors__nvd0__
#define __HWSensors__nvd0__

#include "linux_definitions.h"

void nvd0_therm_init(struct nouveau_device *device);
int nvd0_fan_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty);
int nvd0_gpio_sense(struct nouveau_device *device, int line);

#endif /* defined(__HWSensors__nvd0__) */
