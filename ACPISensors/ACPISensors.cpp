/*
 *  ACPIMonitor.cpp
 *  HWSensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "ACPISensors.h"

#include "FakeSMCDefinitions.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPIMonitor, FakeSMCPlugin)

float ACPIMonitor::getSensorValue(FakeSMCSensor *sensor)
{
#if __LP64__
    UInt64 value;
#else
    UInt32 value;
#endif
    
    switch(sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            if (kIOReturnSuccess == acpiDevice->evaluateInteger(OSDynamicCast(OSString, temperatures->getObject(sensor->getKey()))->getCStringNoCopy(), &value))
                return (float)value;

            break;
        case kFakeSMCVoltageSensor:
            if (kIOReturnSuccess == acpiDevice->evaluateInteger(OSDynamicCast(OSString, voltages->getObject(sensor->getKey()))->getCStringNoCopy(), &value))
                // all voltage values returned from ACPI should be 
                // in millivolts
                return (float)value * 0.001f; 
                
            break;
        case kFakeSMCTachometerSensor: {
            char key[5];
            
            snprintf(key, 5, ACPI_NAME_FORMAT_TACHOMETER, (unsigned int)sensor->getIndex());
            
            if (kIOReturnSuccess == acpiDevice->evaluateInteger(key, &value))
                return (float)value;
            
            break;
        }
    }
    
    return 0;
}

bool ACPIMonitor::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;

	acpiDevice = (IOACPIPlatformDevice *)provider;
	
	if (!acpiDevice) {
        HWSensorsWarningLog("ACPI device not ready");
        return true;
    }
    
    if (OSDictionary *config = OSDynamicCast(OSDictionary, getProperty("Keys Associations"))) {
        // Temperatures
        temperatures = OSDynamicCast(OSDictionary, config->getObject("Temperatures"));
        
        if (temperatures) {
            UInt16 count = 0;
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(temperatures);
            
            iterator->reset();
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, temperatures->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy())) {
                    if (!addSensor(key->getCStringNoCopy(), TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, 0))
                        HWSensorsWarningLog("can't add temperature sensor for method %s with key %s", method->getCStringNoCopy(), key->getCStringNoCopy());
                    else count++;
                }
            };
            
            HWSensorsInfoLog("%d temperature sensor(s) added", count);
        }
        
        
        // Voltages
        voltages = OSDynamicCast(OSDictionary, config->getObject("Voltages"));
        
        if (voltages) {
            UInt16 count = 0;
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(voltages);
            
            iterator->reset();
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, voltages->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy())) 
                    if (!addSensor(key->getCStringNoCopy(), TYPE_FP4C, TYPE_FPXX_SIZE, kFakeSMCVoltageSensor, 0))
                        HWSensorsWarningLog("can't add voltage sensor for method %s with key %s", method->getCStringNoCopy(), key->getCStringNoCopy());
            };
            
            HWSensorsInfoLog("%d voltage sensor(s) added", count);
        }
        
        // Fans
        if (OSArray* fanNames = OSDynamicCast(OSArray, getProperty("Fan Names"))) {
            
            UInt16 count = 0;
            
            for (int i=0; i<10; i++) 
            {
                char key[5];
                
                // Using hardcoded names for ACPI tachometers (with hex sequence number): RPM0, RPM1 ... RPME, RPMF
                snprintf(key, 5, ACPI_NAME_FORMAT_TACHOMETER, i);
                
                if (kIOReturnSuccess == acpiDevice->validateObject(key)) {
                    OSString* name = NULL;
                    
                    if (fanNames)
                        name = OSDynamicCast(OSString, fanNames->getObject(i));
                    
                    if (!addTachometer(i, name ? name->getCStringNoCopy() : 0))
                        HWSensorsWarningLog("Can't add tachometer sensor %d", i);
                    else count++;
                }
            }
            
            HWSensorsInfoLog("%d ttachometer sensor(s) added", count);
        }
    }
    
	registerService();

	return true;	
}