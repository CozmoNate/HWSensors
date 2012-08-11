//
//  nv40.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nv40_h
#define HWSensors_nv40_h

bool nv40_identify(struct nouveau_device *device);
int nv40_pm_clocks_get(struct nouveau_device *device, u8 source);
bool nv40_pm_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty);

#endif
