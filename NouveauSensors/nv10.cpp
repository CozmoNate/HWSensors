//
//  nv10.cpp
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
//
//

#include "nv10.h"

#include "nouveau_definitions.h"
#include "nouveau.h"

void nv10_gpio_init(struct nouveau_device *device)
{
	nv_wr32(device, 0x001140, 0x00000000);
	nv_wr32(device, 0x001100, 0xffffffff);
	nv_wr32(device, 0x001144, 0x00000000);
	nv_wr32(device, 0x001104, 0xffffffff);
}

int nv10_gpio_sense(struct nouveau_device *device, int line)
{
	if (line < 2) {
		line = line * 16;
		line = nv_rd32(device, 0x00600818) >> line;
		return !!(line & 0x0100);
	} else
        if (line < 10) {
            line = (line - 2) * 4;
            line = nv_rd32(device, 0x0060081c) >> line;
            return !!(line & 0x04);
        } else
            if (line < 14) {
                line = (line - 10) * 4;
                line = nv_rd32(device, 0x00600850) >> line;
                return !!(line & 0x04);
            }
    
	return false;
}
