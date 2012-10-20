/*
 *  ACPISensors.cpp
 *  HWSensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "ACPISensors.h"

#include "FakeSMCDefinitions.h"
#include "OEMInfo.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPISensors, FakeSMCPlugin)

float ACPISensors::getSensorValue(FakeSMCSensor *sensor)
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

bool ACPISensors::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;

	acpiDevice = (IOACPIPlatformDevice *)provider;
	
	if (!acpiDevice) {
        HWSensorsFatalLog("ACPI device not ready");
        return false;
    }
    
    if (OSDictionary *configuration = getConfigurationNode())
    {
        // Temperatures
        if ((temperatures = OSDynamicCast(OSDictionary, configuration->getObject("Temperatures")))) {
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(temperatures);
            
            iterator->reset();
            
            UInt16 count = 0;
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, temperatures->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy())) {
                    if (!addSensor(key->getCStringNoCopy(), TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, 0))
                        HWSensorsWarningLog("failed to register temperature sensor for method %s with key %s", method->getCStringNoCopy(), key->getCStringNoCopy());
                    else count++;
                }
            };
            
            if (count)
                HWSensorsInfoLog("%d temperature sensor%s added", count, count > 1 ? "s" : "");
        }
        else return false;
        
        
        // Voltages
        if ((voltages = OSDynamicCast(OSDictionary, configuration->getObject("Voltages")))) {
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(voltages);
            
            iterator->reset();
            
            UInt16 count = 0;
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, voltages->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy())) {
                    if (!addSensor(key->getCStringNoCopy(), TYPE_FP4C, TYPE_FPXX_SIZE, kFakeSMCVoltageSensor, 0))
                        HWSensorsWarningLog("failed to register voltage sensor for method %s with key %s", method->getCStringNoCopy(), key->getCStringNoCopy());
                    else count++;
                }
            };
            
            if (count)
                HWSensorsInfoLog("%d voltage sensor%s added", count, count > 1 ? "s" : "");
        }
        else return false;
        
        
        // Tachometers
        if ((tachometers = OSDynamicCast(OSDictionary, configuration->getObject("Tachometers")))) {
            
            OSDictionary* fanNames = OSDynamicCast(OSDictionary, configuration->getObject("Fan Names"));
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(tachometers);
            
            iterator->reset();
            
            UInt16 count = 0;
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, tachometers->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy())) {
                    OSString* name = NULL;
                    
                    if (fanNames)
                        name = OSDynamicCast(OSString, fanNames->getObject(method));
                    
                    if (!addTachometer(count, name ? name->getCStringNoCopy() : 0))
                        HWSensorsWarningLog("Failed to register tachometer sensor for method \"%s\"", method->getCStringNoCopy());
                    else count++;
                }
            };
            
            if (count)
                HWSensorsInfoLog("%d tachometer sensor%s added", count, count > 1 ? "s" : "");
        }
        else return false;
    }
    else HWSensorsWarningLog("no valid configuration provided");
    
	registerService();
    
    HWSensorsInfoLog("started");

	return true;	
}