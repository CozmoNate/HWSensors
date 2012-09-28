//
//  nouveau_temp.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nouveau_temp_h
#define HWSensors_nouveau_temp_h

struct nvbios_therm_sensor {
	/* diode */
	s16 slope_mult;
	s16 slope_div;
	s16 offset_num;
	s16 offset_den;
	s8 offset_constant;
};

void nouveau_therm_init(struct nouveau_device *device);
int nouveau_therm_fan_get(struct nouveau_device *device);
int nouveau_therm_fan_sense(struct nouveau_device *device);

int nouveau_fan_pwm_get(struct nouveau_device *device);
int nouveau_fan_rpm_get(struct nouveau_device *device);

#endif
