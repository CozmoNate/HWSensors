/*
 *  Radeon.h
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *  Copyright 2013 kozlek. All rights reserved.
 *
 */

#include "FakeSMCPlugin.h"
#include "radeon.h"

class RadeonSensors : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(RadeonSensors)    
	
private:
    radeon_device       card;
    int                 startCounter;
    
protected:	
    virtual float       getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual IOService   *probe(IOService *provider, SInt32 *score);
    virtual bool        start(IOService *provider);
    virtual void		stop(IOService *provider);
};
