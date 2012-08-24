//
//  INT340E.cpp
//  HWSensors
//
//  Created by Kozlek on 24.08.12.
//
//

#include "INT340EMonitor.h"

#include "FakeSMCDefinitions.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(INT340EMonitor, FakeSMCPlugin)


float INT340EMonitor::getSensorValue(FakeSMCSensor *sensor)
{
    UInt64 value;
    
//    switch(sensor->getGroup()) {
//        case kFakeSMCTemperatureSensor:
//            if (kIOReturnSuccess == acpiDevice->evaluateInteger(OSDynamicCast(OSString, temperatures->getObject(sensor->getKey()))->getCStringNoCopy(), &value))
//                return (float)value;
//            
//            break;
//        case kFakeSMCVoltageSensor:
//            if (kIOReturnSuccess == acpiDevice->evaluateInteger(OSDynamicCast(OSString, voltages->getObject(sensor->getKey()))->getCStringNoCopy(), &value))
//                // all voltage values returned from ACPI should be
//                // in millivolts
//                return (float)value * 0.001f;
//            
//            break;
//        case kFakeSMCTachometerSensor: {
//            char key[5];
//            
//            snprintf(key, 5, ACPI_NAME_FORMAT_TACHOMETER, (unsigned int)sensor->getIndex());
//            
//            if (kIOReturnSuccess == acpiDevice->evaluateInteger(key, &value))
//                return (float)value;
//            
//            break;
//        }
//    }
    
    return 0;
}

bool INT340EMonitor::start(IOService * provider)
{
	if (!super::start(provider))
        return false;
    
	acpiDevice = (IOACPIPlatformDevice *)provider;
	
	if (!acpiDevice) {
        HWSensorsWarningLog("ACPI device not ready");
        return false;
    }
    
    registerService();
    
    if (kIOReturnSuccess == acpiDevice->evaluateInteger("IVER", &version))
        setProperty("version", version, 64);
        
    
    OSObject *object;
    
    if (kIOReturnSuccess == acpiDevice->evaluateObject("TSDL", &object))
        setProperty("Temperatures", object);
    
    HWSensorsInfoLog("started");
    
	return true;
}