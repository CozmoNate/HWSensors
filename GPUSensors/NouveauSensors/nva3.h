//
//  nva3.h
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
//
//

#ifndef HWSensors_nva3_h
#define HWSensors_nva3_h

int nva3_therm_fan_sense(struct nouveau_device *device);
int nva3_therm_init(struct nouveau_device *device);
int nva3_clocks_get(struct nouveau_device *device, u8 source);

#endif
