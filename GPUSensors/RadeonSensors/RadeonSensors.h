/*
 *  Radeon.h
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "FakeSMCPlugin.h"
#include "radeon.h"

enum TempFamilies {
	R5xx,
	R6xx,
	R7xx,
	R8xx
};

class RadeonMonitor : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(RadeonMonitor)    
	
private:
    radeon_device       card;
    
protected:	
    virtual float       getSensorValue(FakeSMCSensor *sensor);
	
public:
    virtual bool		start(IOService *provider);
    
};
