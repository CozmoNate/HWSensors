//
//  nouveau_gpio.cpp
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
//
//

#include "nouveau_gpio.h"
#include "nouveau.h"
#include "nouveau_bios.h"

static u16 dcb_gpio_table(struct nouveau_device *device)
{
	u8  ver, hdr, cnt, len;
	u16 dcb = nouveau_dcb_table(device, &ver, &hdr, &cnt, &len);
	if (dcb) {
		if (ver >= 0x30 && hdr >= 0x0c)
			return nv_ro16(device, dcb + 0x0a);
		if (ver >= 0x22 && nv_ro08(device, dcb - 1) >= 0x13)
			return nv_ro16(device, dcb - 0x0f);
	}
	return 0x0000;
}

static u16 dcb_gpio_entry(struct nouveau_device *device, int idx, int ent, u8 *ver)
{
	u16 gpio = dcb_gpio_table(device);
	if (gpio) {
		*ver = nv_ro08(device, gpio);
		if (*ver < 0x30 && ent < nv_ro08(device, gpio + 2))
			return gpio + 3 + (ent * nv_ro08(device, gpio + 1));
		else if (ent < nv_ro08(device, gpio + 2))
			return gpio + nv_ro08(device, gpio + 1) +
            (ent * nv_ro08(device, gpio + 3));
	}
	return 0x0000;
}

static bool dcb_gpio_parse(struct nouveau_device *device, int idx, u8 func, u8 line,
               struct dcb_gpio_func *gpio)
{
	u8  ver, hdr, cnt, len;
	u16 entry;
	int i = -1;
    
	while ((entry = dcb_gpio_entry(device, idx, ++i, &ver))) {
		if (ver < 0x40) {
			u16 data = nv_ro16(device, entry);
			(*gpio).line = (data & 0x001f) >> 0;
			(*gpio).func = (data & 0x07e0) >> 5;
            (*gpio).log[0] = (data & 0x1800) >> 11;
            (*gpio).log[1] = (data & 0x6000) >> 13;
		} else
            if (ver < 0x41) {
                u32 data = nv_ro32(device, entry);
                (*gpio).line = (data & 0x0000001f) >> 0;
                (*gpio).func = (data & 0x0000ff00) >> 8;
                (*gpio).log[0] = (data & 0x18000000) >> 27;
                (*gpio).log[1] = (data & 0x60000000) >> 29;
            } else {
                u32 data = nv_ro32(device, entry + 0);
                u8 data1 = nv_ro32(device, entry + 4);
                (*gpio).line = (data & 0x0000003f) >> 0;
                (*gpio).func = (data & 0x0000ff00) >> 8;
                (*gpio).log[0] = (data1 & 0x30) >> 4;
                (*gpio).log[1] = (data1 & 0xc0) >> 6;
            }
        
		if ((line == 0xff || line == gpio->line) &&
		    (func == 0xff || func == gpio->func))
			return true;
	}
    
	/* DCB 2.2, fixed TVDAC GPIO data */
	if ((entry = nouveau_dcb_table(device, &ver, &hdr, &cnt, &len)) && ver >= 0x22) {
		if (func == DCB_GPIO_TVDAC0) {
            (*gpio).func = DCB_GPIO_TVDAC0;
            (*gpio).line = nv_ro08(device, entry - 4) >> 4;
            (*gpio).log[0] = !!(nv_ro08(device, entry - 5) & 2);
            (*gpio).log[1] =  !(nv_ro08(device, entry - 5) & 2);
			return true;
		}
	}
    
	return false;
}

int nouveau_gpio_sense(struct nouveau_device *device, int idx, int line)
{
    if (!device->gpio_sense) {
        nv_debug(device, "hardware GPIO sense function not set\n");
        return false;
    }
    
	return device->gpio_sense(device, line);
}

bool nouveau_gpio_find(struct nouveau_device *device, int idx, u8 tag, u8 line, struct dcb_gpio_func *func)
{
	if (line == 0xff && tag == 0xff)
		return false;
    
	if (dcb_gpio_parse(device, idx, tag, line, func))
		return true;
    
    nv_debug(device, "can't find GPIO line\n");
    
	return false;
}

int nouveau_gpio_get(struct nouveau_device *device, int idx, u8 tag, u8 line)
{
	struct dcb_gpio_func func;
	int ret = 0;
    
	if (nouveau_gpio_find(device, idx, tag, line, &func)) {
		ret = nouveau_gpio_sense(device, idx, func.line);
		if (ret >= 0)
			ret = (ret == (func.log[1] & 1));
        else
            nv_debug(device, "insane GPIO value\n");
	}
    else nv_debug(device, "GPIO not found\n");
    
	return ret;
}

int nouveau_pwmfan_gpio_get(struct nouveau_device *device)
{
	struct dcb_gpio_func func;
	u32 divs, duty;
    
	if (!device->pwm_get) {
        nv_debug(device, "no hardware pwm_get func specified\n");
	 	return 0;
    }
    
	if (nouveau_gpio_find(device, 0, DCB_GPIO_PWM_FAN, 0xff, &func)) {
        bool ret = device->pwm_get(device, func.line, &divs, &duty);
		
        if (ret && divs) {
			divs = max(divs, duty);
			if (device->card_type <= NV_40 || (func.log[0] & 1))
				duty = divs - duty;
			return (duty * 100) / divs;
		}
        
		return nouveau_gpio_get(device, 0, func.func, func.line) * 100;
	}
    
    nv_debug(device, "can't get pwm value\n");
    
	return 0;
}

#define RPM_SENSE_MSECS 500

int nouveau_rpmfan_gpio_get(struct nouveau_device *device)
{
	struct dcb_gpio_func func;
	u32 cycles, cur, prev;
    
	if (!nouveau_gpio_find(device, 0, DCB_GPIO_FAN_SENSE, 0xff, &func)) {
        nv_debug(device, "rpm fan sense gpio func not found\n");
		return 0;
    }
    
	/* Monitor the GPIO input 0x3b for 250ms.
	 * When the fan spins, it changes the value of GPIO FAN_SENSE.
	 * We get 4 changes (0 -> 1 -> 0 -> 1 -> [...]) per complete rotation.
	 */
	mach_timespec_t end, now;
    
    clock_get_system_nanotime((clock_sec_t*)&end.tv_sec, (clock_nsec_t*)&end.tv_nsec);
    
    now.tv_sec = 0;
    now.tv_nsec = RPM_SENSE_MSECS * USEC_PER_SEC;
    
    ADD_MACH_TIMESPEC(&end, &now);
    
	prev = nouveau_gpio_get(device, 0, func.func, func.line);
	cycles = 0;
	do {
		cur = nouveau_gpio_get(device, 0, func.func, func.line);
		if (prev != cur) {
			cycles++;
			prev = cur;
		}
        
		IODelay(750); /* supports 0 < rpm < 7500 */
        
        //counter++;
        
        clock_get_system_nanotime((clock_sec_t*)&now.tv_sec, (clock_nsec_t*)&now.tv_nsec);
        
	} while (CMP_MACH_TIMESPEC(&end, &now) > 0);
    
	/* interpolate to get rpm */
	return (float)cycles / 4.0f * (1000.0f / RPM_SENSE_MSECS) * 60.0f;
}
