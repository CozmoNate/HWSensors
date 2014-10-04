/*
 *  Radeon.h
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *  Copyright 2013 kozlek. All rights reserved.
 *
 */

#include "GPUSensors.h"
#include "radeon.h"

class RadeonSensors : public GPUSensors
{
    OSDeclareDefaultStructors(RadeonSensors)    
	
private:
    radeon_device       card;
    
protected:	
    virtual bool        willReadSensorValue(FakeSMCSensor *sensor, float* value);
    virtual bool        shouldWaitForAccelerator();
    virtual bool        probIsAcceleratorAlreadyLoaded();
	virtual bool        managedStart(IOService *provider);
    
public:
    virtual void		stop(IOService *provider);
};
