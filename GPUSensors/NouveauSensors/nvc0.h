//
//  nvc0.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nvc0_h
#define HWSensors_nvc0_h

#include "nouveau_definitions.h"

bool nvc0_identify(struct nouveau_device *device);
void nvc0_init(struct nouveau_device *device);

int nvc0_therm_fan_sense(struct nouveau_device *device);
int nvc0_clocks_get(struct nouveau_device *device, u8 source);
int nve0_clocks_get(struct nouveau_device *device, u8 source);

#endif
