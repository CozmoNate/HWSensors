//
//  INT340E.h
//  HWSensors
//
//  Created by Kozlek on 24.08.12.
//
//

#ifndef __HWSensors__INT340E__
#define __HWSensors__INT340E__

#include <IOKit/IOService.h>
#include "IOKit/acpi/IOACPIPlatformDevice.h"

#include "FakeSMCPlugin.h"

#include <kern/clock.h>

class INT340EMonitor : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(INT340EMonitor)
    
private:
	IOACPIPlatformDevice    *acpiDevice;
    UInt64                  version;
    
    OSArray                 *temperatures;
    mach_timespec_t         temperatureNextUpdate;
    
    bool                    updateTemperatures();
    float                   readTemperature(UInt32 index);
    void                    parseTemperatureName(OSString *name, UInt32 index);
    
protected:
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual bool			start(IOService *provider);
};

#endif /* defined(__HWSensors__INT340E__) */
