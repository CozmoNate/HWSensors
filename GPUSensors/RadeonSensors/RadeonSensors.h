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
    
    bool                managedStart(IOService *provide);
    
protected:	
    virtual float       getSensorValue(FakeSMCSensor *sensor);
    virtual void        onAcceleratorFound(IOService *provider);
    virtual void        onTimeoutExceeded(IOService *provider);
    
public:
    virtual bool        start(IOService *provider);
    virtual void		stop(IOService *provider);
};
