//
//  nouveau_xpio.h
//  HWSensors
//
//  Created by Kozlek on 08.12.12.
//
//

#ifndef __HWSensors__nouveau_xpio__
#define __HWSensors__nouveau_xpio__

#include "nouveau_definitions.h"

struct nvbios_xpio {
	u8 type;
	u8 addr;
	u8 flags;
};

u16 dcb_xpio_table(struct nouveau_device *device, u8 idx, u8 *ver, u8 *hdr, u8 *cnt, u8 *len);

#endif /* defined(__HWSensors__nouveau_xpio__) */
