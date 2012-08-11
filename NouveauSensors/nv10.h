//
//  nv10.h
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
//
//

#ifndef __HWSensors__nv10__
#define __HWSensors__nv10__

void nv10_gpio_init(struct nouveau_device *device);
int nv10_gpio_sense(struct nouveau_device *device, int line);

#endif /* defined(__HWSensors__nv10__) */
