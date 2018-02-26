//
//  gp100.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 17/12/2016.
//
//

#include "gp100.h"

#include "nouveau.h"
#include "nv84.h"
#include "nva3.h"
#include "nvd0.h"
#include "nve0.h"
#include "nouveau_therm.h"

bool gp100_identify(struct nouveau_device *device)
{
    switch (device->chipset) {
        case 0x130:
            device->cname = "GP100";
            break;
            
        case 0x132:
            device->cname = "GP102";
            break;
            
        case 0x134:
            device->cname = "GP104";
            break;
            
        case 0x136:
            device->cname = "GP106";
            break;
            
        case 0x137:
            device->cname = "GP107";
            break;
            
        case 0x138:
        case 0x13b:
            device->cname = "GP108";
            break;
            
        default:
            nv_fatal(device, "unknown Pascal chipset 0x%x\n", device->chipset);
            return false;
    }
    
    return true;
}

void gp100_init(struct nouveau_device *device)
{
    nvd0_therm_init(device);
    
    device->gpio_find = nouveau_gpio_find;
    device->gpio_get = nouveau_gpio_get;
    device->gpio_sense = nvd0_gpio_sense;
    device->temp_get = nv84_temp_get;
//    device->clocks_get = nve0_clock_read;
//    device->voltage_get = nouveau_voltage_get;
    device->pwm_get = nvd0_fan_pwm_get;
    device->fan_pwm_get = nouveau_therm_fan_pwm_get;
    device->fan_rpm_get = nva3_therm_fan_sense;
}

