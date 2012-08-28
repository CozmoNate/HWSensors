/*
 *  ACPIMonitor.h
 *  HWSensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include "IOKit/acpi/IOACPIPlatformDevice.h"

#include "FakeSMCPlugin.h"

#define ACPI_NAME_CPU_HEATSINK          "TCPU"
#define ACPI_NAME_PCH_HEATSINK          "TPCH"
#define ACPI_NAME_AMBIENT_TEMPERATURE   "TAMB"
#define ACPI_NAME_PROXIMITY_TEMPERATURE "TPRX"

#define ACPI_NAME_CPU_VOLTAGE           "VCPU"
#define ACPI_NAME_PCH_VOLTAGE           "VPCH"
#define ACPI_NAME_MEMORY_VOLTAGE        "VMEM"
#define ACPI_NAME_BATTERY_VOLTAGE       "VBAT"

#define ACPI_NAME_FORMAT_TACHOMETER     "RPM%X"

class ACPIMonitor : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(ACPIMonitor)
    
private:
	IOACPIPlatformDevice    *acpiDevice;
    
    OSDictionary            *temperatures;
    OSDictionary            *voltages;
    OSDictionary            *tachometers;
    
protected:
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual bool			start(IOService *provider);
};
