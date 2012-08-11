//
//  nouveau_temp.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nouveau_temp_h
#define HWSensors_nouveau_temp_h

int nv40_diode_temp_get(struct nouveau_device *device);
int nv84_diode_temp_get(struct nouveau_device *device);
void nouveau_temp_init(struct nouveau_device *device);

#endif
