/*
 *  ACPISensors.h
 *  HWSensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include "IOKit/acpi/IOACPIPlatformDevice.h"

#include "FakeSMCPlugin.h"

class ACPISensors : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(ACPISensors)
    
private:
	IOACPIPlatformDevice    *acpiDevice;
    OSArray                 *methods;
    
protected:
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual bool			start(IOService *provider);
};
