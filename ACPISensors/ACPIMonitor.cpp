/*
 *  ACPIMonitor.cpp
 *  HWSensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "ACPIMonitor.h"

#include "FakeSMCDefinitions.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPIMonitor, FakeSMCPlugin)

float ACPIMonitor::getSensorValue(FakeSMCSensor *sensor)
{
    UInt64 value;
    
    switch(sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            if (kIOReturnSuccess == acpiDevice->evaluateInteger(OSDynamicCast(OSString, temperatures->getObject(sensor->getKey()))->getCStringNoCopy(), &value))
                return (float)value;

            break;
        case kFakeSMCVoltageSensor:
            if (kIOReturnSuccess == acpiDevice->evaluateInteger(OSDynamicCast(OSString, voltages->getObject(sensor->getKey()))->getCStringNoCopy(), &value))
                // all voltage values returned from ACPI should be 
                // in millivolts ?
                return (float)value * 0.001f; 
                
            break;
        case kFakeSMCTachometerSensor: {
            if (kIOReturnSuccess == acpiDevice->evaluateInteger(OSDynamicCast(OSString, tachometers->getObject(sensor->getKey()))->getCStringNoCopy(), &value))
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
        return false;
    }
    
    if (OSDictionary *config = OSDynamicCast(OSDictionary, getProperty("Keys Associations"))) {
        // Temperatures
        if ((temperatures = OSDynamicCast(OSDictionary, config->getObject("Temperatures")))) {
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(temperatures);
            
            iterator->reset();
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, temperatures->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy())) {
                    if (!addSensor(key->getCStringNoCopy(), TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, 0))
                        HWSensorsWarningLog("can't add temperature sensor for method %s with key %s", method->getCStringNoCopy(), key->getCStringNoCopy());
                }
            };
            
           //HWSensorsInfoLog("%d temperature sensor(s) added", count);
        }
        else return false;
        
        
        // Voltages
        if ((voltages = OSDynamicCast(OSDictionary, config->getObject("Voltages")))) {
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(voltages);
            
            iterator->reset();
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, voltages->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy())) {
                    if (!addSensor(key->getCStringNoCopy(), TYPE_FP4C, TYPE_FPXX_SIZE, kFakeSMCVoltageSensor, 0))
                        HWSensorsWarningLog("can't add voltage sensor for method %s with key %s", method->getCStringNoCopy(), key->getCStringNoCopy());
                }
            };
            
            //HWSensorsInfoLog("%d voltage sensor(s) added", count);
        }
        else return false;
        
        
        // Tachometers
        if ((tachometers = OSDynamicCast(OSDictionary, config->getObject("Tachometers")))) {
            UInt16 count = 0;
            OSArray* fanNames = OSDynamicCast(OSArray, getProperty("Fan Names"));
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(tachometers);
            
            iterator->reset();
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, tachometers->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy())) {
                    OSString* name = NULL;
                    
                    if (fanNames)
                        name = OSDynamicCast(OSString, fanNames->getObject(count));
                    
                    if (!addTachometer(count, name ? name->getCStringNoCopy() : 0))
                        HWSensorsWarningLog("Failed to register tachometer sensor %d", count);
                    
                    count++;
                }
            };
            
            //HWSensorsInfoLog("%d tachometer sensor(s) added", count);
        }
        else return false;
    }
    
	registerService();

	return true;	
}