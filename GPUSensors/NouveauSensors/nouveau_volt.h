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

struct nouveau_volt {
    
	int (*vid_get)(struct nouveau_device *);
	int (*get)(struct nouveau_device *);
    
	u8 vid_mask;
	u8 vid_nr;
	struct {
		u32 uv;
		u8 vid;
	} vid[256];
};

struct nvbios_volt {
	u8  vidmask;
	u32 min;
	u32 max;
	u32 base;
	s16 step;
};

struct nvbios_volt_entry {
	u32 voltage;
	u8  vid;
};

int nouveau_volt_create(struct nouveau_device *device);

#endif
