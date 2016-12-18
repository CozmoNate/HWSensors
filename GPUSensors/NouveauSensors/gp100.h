//
//  gp100.h
//  HWSensors
//
//  Created by Natan Zalkin on 17/12/2016.
//
//

#ifndef HWSensors_gp100_hpp
#define HWSensors_gp100_hpp

bool gp100_identify(struct nouveau_device *device);
void gp100_init(struct nouveau_device *device);

#endif /* HWSensors_gp100_hpp */
