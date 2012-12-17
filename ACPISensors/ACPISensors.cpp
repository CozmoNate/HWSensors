/*
 *  ACPISensors.cpp
 *  HWSensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "ACPISensors.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPISensors, FakeSMCPlugin)

float ACPISensors::getSensorValue(FakeSMCSensor *sensor)
{
    UInt32 value;
    
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

bool ACPISensors::addSensorToList(OSDictionary *list, OSString *configKey, OSString *acpiMethod, const char *refName, const char* smcKey, const char *type, UInt8 size, UInt32 group, UInt32 index)
{
    if (configKey->isEqualTo(refName)) {
        if (addSensor(smcKey, type, size, group, index)) {
            list->setObject(smcKey, acpiMethod);
            return true;
        }
    }
    
    HWSensorsWarningLog("failed to register sensor for key %s", configKey->getCStringNoCopy());
    
    return false;
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
        if (OSDictionary *temps = OSDynamicCast(OSDictionary, configuration->getObject("Temperatures"))) {
            
            temperatures = OSDictionary::withCapacity(0);
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(temps);
            UInt16 count = 0;
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, temps->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                    for (int i = 0; i < FakeSMCTemperatureCount; i++) {
                        if (addSensorToList(temperatures, key, method, FakeSMCTemperature[i].name, FakeSMCTemperature[i].key, FakeSMCTemperature[i].type, FakeSMCTemperature[i].size, kFakeSMCTemperatureSensor, count)) {
                            count++;
                            break;
                        }
                        else HWSensorsErrorLog("Failed to register temperature sensor \"%s\" for method \"%s\"", key->getCStringNoCopy(), method->getCStringNoCopy());
                    }
                }
            };
            
            if (count)
                HWSensorsInfoLog("%d temperature sensor%s added", count, count > 1 ? "s" : "");
        }
        else return false;
        
        
        // Voltages
        if (OSDictionary *volts = OSDynamicCast(OSDictionary, configuration->getObject("Voltages"))) {
            
            voltages = OSDictionary::withCapacity(0);
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(volts);
            UInt16 count = 0;
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, volts->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                    for (int i = 0; i < FakeSMCVolatgeCount; i++) {
                        if (addSensorToList(voltages, key, method, FakeSMCVolatge[i].name, FakeSMCVolatge[i].key, FakeSMCVolatge[i].type, FakeSMCVolatge[i].size, kFakeSMCVoltageSensor, count)) {
                            count++;
                            break;
                        }
                        else HWSensorsErrorLog("Failed to register voltage sensor \"%s\" for method \"%s\"", key->getCStringNoCopy(), method->getCStringNoCopy());
                    }
                }
            };
            
            if (count)
                HWSensorsInfoLog("%d voltage sensor%s added", count, count > 1 ? "s" : "");
        }
        else return false;
        
        
        // Tachometers
        if (OSDictionary *fans = OSDynamicCast(OSDictionary, configuration->getObject("Tachometers"))) {
            tachometers = OSDictionary::withCapacity(0);
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(fans);
            UInt16 count = 0;
            
            OSDictionary* fanNames = OSDynamicCast(OSDictionary, configuration->getObject("Fan Names"));
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, fans->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                    OSString* name = NULL;

                    if (fanNames)
                        name = OSDynamicCast(OSString, fanNames->getObject(key));
                    
                    if (FakeSMCSensor *sensor = addTachometer(count, name ? name->getCStringNoCopy() : 0)) {
                        tachometers->setObject(sensor->getKey(), method);
                        count++;
                    }
                    else HWSensorsErrorLog("Failed to register tachometer sensor for method \"%s\"", method->getCStringNoCopy());
                }
            };
            
            if (count)
                HWSensorsInfoLog("%d tachometer sensor%s added", count, count > 1 ? "s" : "");
        }
        else return false;
    }
    else HWSensorsErrorLog("no valid configuration provided");
    
	registerService();
    
    HWSensorsInfoLog("started");

	return true;	
}