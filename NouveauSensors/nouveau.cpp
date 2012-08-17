//
//  nouveau.c
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#include "nouveau.h"
#include "nv40.h"
#include "nv50.h"
#include "nva3.h"
#include "nvc0.h"
#include "nve0.h"
#include "nouveau_temp.h"
#include "nouveau_volt.h"

bool nouveau_identify(struct nouveau_device *device)
{
    /* identify the chipset */
    
    nv_debug(device, "identifying the chipset\n");
    
    /* read boot0 and strapping information */
    UInt32 boot0 = nv_rd32(device, 0x000000);
    UInt32 strap = nv_rd32(device, 0x101000);
    
    /* determine chipset and derive architecture from it */
    if ((boot0 & 0x0f000000) > 0) {
        device->chipset = (boot0 & 0xff00000) >> 20;
        switch (device->chipset & 0xf0) {
			case 0x40:
			case 0x60: device->card_type = NV_40; break;
			case 0x50:
			case 0x80:
			case 0x90:
			case 0xa0: device->card_type = NV_50; break;
			case 0xc0: device->card_type = NV_C0; break;
			case 0xd0: device->card_type = NV_D0; break;
			case 0xe0: device->card_type = NV_E0; break;
			default:
				break;
        }
    }
    
    bool ret = FALSE;
    
    switch (device->card_type) {
		case NV_40: ret = nv40_identify(device); break;
		case NV_50: ret = nv50_identify(device); break;
		case NV_C0:
		case NV_D0: ret = nvc0_identify(device); break;
		case NV_E0: ret = nve0_identify(device); break;
        default: break;
    }
    
    if (!ret) {
        nv_error(device, "unknown chipset, 0x%08x\n", boot0);
        return false;
    }
    
    nv_debug(device, "BOOT0  : 0x%08x\n", boot0);
    nv_info(device, "chipset: %s (NV%02X) family: NV%02X\n",
			device->cname, device->chipset, device->card_type);
    
    /* determine frequency of timing crystal */
    if ( device->chipset < 0x17 ||
        (device->chipset >= 0x20 && device->chipset <= 0x25))
        strap &= 0x00000040;
    else
        strap &= 0x00400040;
    
    switch (strap) {
		case 0x00000000: device->crystal = 13500; break;
		case 0x00000040: device->crystal = 14318; break;
		case 0x00400000: device->crystal = 27000; break;
		case 0x00400040: device->crystal = 25000; break;
    }
    
    nv_debug(device, "crystal freq: %dKHz\n", device->crystal);
    
    return true;
}

bool nouveau_init(struct nouveau_device *device)
{
	//char info[256];
	//int ret, i;
    
    nv_debug(device, "initializing monitoring driver\n");
    
	if (device->card_type < NV_50) {
        device->clocks_get = nv40_pm_clocks_get;
        device->voltage_get = nouveau_voltage_gpio_get;
        device->diode_temp_get = nv40_diode_temp_get;
        device->pwm_get = nv40_pm_pwm_get;
        device->pwm_fan_get = nouveau_pwmfan_gpio_get;
        device->rpm_fan_get = nouveau_rpmfan_gpio_get;
    }
    else if (device->card_type < NV_C0) {
        if (device->chipset <  0xa3 ||
            device->chipset == 0xaa ||
            device->chipset == 0xac) {
            device->clocks_get = nv50_pm_clocks_get;
        }
        else {
            device->clocks_get = nva3_pm_clocks_get;
        }
        device->voltage_get = nouveau_voltage_gpio_get;
        if (device->chipset == 0x50)
            device->diode_temp_get = nv40_diode_temp_get;
        else
            device->diode_temp_get = nv84_diode_temp_get;
        device->pwm_get = nv50_pm_pwm_get;
        device->pwm_fan_get = nouveau_pwmfan_gpio_get;
        device->rpm_fan_get = nouveau_rpmfan_gpio_get;
    }
    else if (device->card_type < NV_E0) {
        device->clocks_get = nvc0_pm_clocks_get;
        device->voltage_get = nouveau_voltage_gpio_get;
        device->diode_temp_get = nv84_diode_temp_get;
        if (device->card_type < NV_D0) {
            device->pwm_get = nv50_pm_pwm_get;
            device->pwm_fan_get = nouveau_pwmfan_gpio_get;
            device->rpm_fan_get = nouveau_rpmfan_gpio_get;
        }
    }
    else if (device->card_type < 0xf0) {
        //device->clocks_get = nvc0_pm_clocks_get;
        //device->voltage_get = nouveau_voltage_gpio_get;
        device->diode_temp_get = nv84_diode_temp_get;
        //device->pwm_get = nv50_pm_pwm_get;
        //device->pwm_fan_get = nouveau_pwmfan_gpio_get;
        //device->rpm_fan_get = nouveau_rpmfan_gpio_get;
    }
    
    /*if (device->gpio_init)
        device->gpio_init(device);*/
    
    //nouveau_i2c_create(device);
    
	/* parse aux tables from vbios */
	nouveau_volt_init(device);
	nouveau_temp_init(device);
    
    return true;
}