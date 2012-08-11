//
//  nouveau_volt.h
//  HWSensors
//
//  Created by Kozlek on 10.08.12.
//
//

#ifndef HWSensors_nouveau_volt_h
#define HWSensors_nouveau_volt_h

#include "nouveau_definitions.h"

struct nouveau_pm_voltage_level {
	u32 voltage; /* microvolts */
	u8  vid;
};

struct nouveau_pm_voltage {
	bool supported;
	u8 version;
	u8 vid_mask;
    
	struct nouveau_pm_voltage_level *level;
	int nr_level;
};

void nouveau_volt_init(struct nouveau_device *);
int nouveau_voltage_gpio_get(struct nouveau_device *device);

#endif
