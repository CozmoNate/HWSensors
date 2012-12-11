//
//  nouveau_gpio.h
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
//
//

#ifndef __HWSensors__nouveau_gpio__
#define __HWSensors__nouveau_gpio__

#include "nouveau_definitions.h"

enum dcb_gpio_func_name {
	DCB_GPIO_PANEL_POWER = 0x01,
	DCB_GPIO_TVDAC0 = 0x0c,
	DCB_GPIO_TVDAC1 = 0x2d,
	DCB_GPIO_FAN = 0x09,
	DCB_GPIO_FAN_SENSE = 0x3d,
	DCB_GPIO_UNUSED = 0xff
};

struct dcb_gpio_func {
	u8 func;
	u8 line;
	u8 log[2];
    
	/* so far, "param" seems to only have an influence on PWM-related
	 * GPIOs such as FAN_CONTROL and PANEL_BACKLIGHT_LEVEL.
	 * if param equals 1, hardware PWM is available
	 * if param equals 0, the host should toggle the GPIO itself
	 */
	u8 param;
};

u16 dcb_gpio_table(struct nouveau_device *device, u8 *ver, u8 *hdr, u8 *cnt, u8 *len);

int nouveau_gpio_find(struct nouveau_device *device, int idx, u8 tag, u8 line, struct dcb_gpio_func *func);
int nouveau_gpio_get(struct nouveau_device *device, int idx, u8 tag, u8 line);

#endif /* defined(__HWSensors__nouveau_gpio__) */
