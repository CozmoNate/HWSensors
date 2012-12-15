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
void nv50_init(struct nouveau_device *device);

int nv50_gpio_sense(struct nouveau_device *device, int line);

int nv50_clocks_get(struct nouveau_device *device, u8 source);
int nv50_temp_get(struct nouveau_device *device);
int nv50_fan_pwm_get(struct nouveau_device *device, int line, u32 *divs, u32 *duty);

#endif
