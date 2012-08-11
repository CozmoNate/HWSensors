//
//  nv50.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nv50_h
#define HWSensors_nv50_h

bool nv50_identify(struct nouveau_device *device);
int nv50_pm_clocks_get(struct nouveau_device *device, u8 source);

void nv50_gpio_init(struct nouveau_device *device);
int nv50_gpio_sense(struct nouveau_device *device, int line);
bool nv50_pm_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty);

#endif
