//
//  nvd0.cpp
//  HWSensors
//
//  Created by Kozlek on 11.08.12.
//
//

#include "nvd0.h"

#include "nouveau_definitions.h"
#include "nouveau.h"

int nvd0_gpio_sense(struct nouveau_device *device, int line)
{
	return !!(nv_rd32(device, 0x00d610 + (line * 4)) & 0x00004000);
}

