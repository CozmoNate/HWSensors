//
//  gm200.cpp
//  HWSensors
//
//  Created by Kozlek on 24/10/14.
//
//

#include "gm200.h"

#include "nouveau.h"
#include "nv50.h"
#include "nva3.h"
#include "nv84.h"
#include "nvd0.h"
#include "nve0.h"
#include "nouveau_therm.h"

bool gm200_identify(struct nouveau_device *device)
{
//    switch (device->chipset) {
//        case 0x124:
//            device->cname = "GK204";
//            break;
//
//        default:
//            nv_fatal(device, "unknown Maxwell chipset 0x%x\n", device->chipset);
//            return false;
//    }
//
//    return true;

    return false;
}

void gm200_init(struct nouveau_device *device)
{
//    nvd0_therm_init(device);

//    device->gpio_find = nouveau_gpio_find;
//    device->gpio_get = nouveau_gpio_get;
//    device->gpio_sense = nvd0_gpio_sense;
//    device->temp_get = nv84_temp_get;
//    device->clocks_get = nve0_clock_read;
//    //    device->voltage_get = nouveau_voltage_get;
//    device->pwm_get = nvd0_fan_pwm_get;
//    device->fan_pwm_get = nouveau_therm_fan_pwm_get;
//    device->fan_rpm_get = nva3_therm_fan_sense;
}
