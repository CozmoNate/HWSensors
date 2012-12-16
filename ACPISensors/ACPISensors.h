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
    
    OSDictionary            *temperatures;
    OSDictionary            *voltages;
    OSDictionary            *tachometers;
    
    bool                    addSensorToList(OSDictionary *list, OSString *configKey, OSString *acpiMethod, const char *refName, const char* smcKey, const char *type, UInt8 size, UInt32 group, UInt32 index);
    
protected:
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual bool			start(IOService *provider);
};
