/*
 *  ACPISensors.cpp
 *  ACPISensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "ACPISensors.h"

#define ACPISensorsDebugLog(string, args...)	do { if (kACPISensorsDebug) { IOLog ("%s (%s): [Debug] " string "\n",getName(), acpiDevice->getName() , ## args); } } while(0)
#define ACPISensorsWarningLog(string, args...) do { IOLog ("%s (%s): [Warning] " string "\n",getName(), acpiDevice->getName(), ## args); } while(0)
#define ACPISensorsErrorLog(string, args...) do { IOLog ("%s (%s): [Error] " string "\n",getName(), acpiDevice->getName() , ## args); } while(0)
#define ACPISensorsFatalLog(string, args...) do { IOLog ("%s (%s): [Fatal] " string "\n",getName(), acpiDevice->getName() , ## args); } while(0)
#define ACPISensorsInfoLog(string, args...)	do { IOLog ("%s (%s): " string "\n",getName(), acpiDevice->getName() , ## args); } while(0)

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPISensors, FakeSMCPlugin)

float ACPISensors::getSensorValue(FakeSMCSensor *sensor)
{
    UInt32 value = 0;
    OSString *method = NULL;
    
    if (sensor->getIndex() < methods->getCount() && (method = OSDynamicCast(OSString, methods->getObject(sensor->getIndex())))) {
        if (kIOReturnSuccess == acpiDevice->evaluateInteger(method->getCStringNoCopy(), &value)) {
            switch(sensor->getGroup()) {
                case kFakeSMCTemperatureSensor:                    
                    // all temperatures returned from ACPI should be in Kelvins
                    return ((float)value - (float)0xAAC) / (float)0xA;
                    
                case kFakeSMCVoltageSensor:
                case kFakeSMCCurrentSensor:
                case kFakeSMCPowerSensor:
                    // all voltage values returned from ACPI should be in millivolts?
                    return (float)value * 0.001f;
            
                case kFakeSMCTachometerSensor:
                default:
                    return (float)value;
                
            }
        }
    }
    
    return 0;
}

bool ACPISensors::start(IOService * provider)
{
	if (!super::start(provider))
        return false;

	if (!(acpiDevice = OSDynamicCast(IOACPIPlatformDevice, provider))) {
        ACPISensorsFatalLog("ACPI device not ready");
        return false;
    }
    
    if (OSDictionary *configuration = getConfigurationNode())
    {
        methods = OSArray::withCapacity(0);
        
        // Temperatures
        if (OSDictionary *temps = OSDynamicCast(OSDictionary, configuration->getObject("Temperatures"))) {
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(temps);
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, temps->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                    if (FakeSMCSensor *sensor = addSensor(key, kFakeSMCCategoryTemperature, kFakeSMCTemperatureSensor, methods->getCount())) {
                        methods->setObject(method);//sensor->getKey(), method);
                    }
                    else {
                        ACPISensorsErrorLog("Failed to register temperature sensor \"%s\" for method \"%s\"", key->getCStringNoCopy(), method->getCStringNoCopy());
                    }
                }
            }
        }        
        
        // Voltages
        if (OSDictionary *volts = OSDynamicCast(OSDictionary, configuration->getObject("Voltages"))) {
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(volts);
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, volts->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                    if (FakeSMCSensor *sensor = addSensor(key, kFakeSMCCategoryVoltage, kFakeSMCVoltageSensor, methods->getCount())) {
                        methods->setObject(method);//sensor->getKey(), method);
                    }
                    else {
                        ACPISensorsErrorLog("Failed to register voltage sensor \"%s\" for method \"%s\"", key->getCStringNoCopy(), method->getCStringNoCopy());
                    }
                }
            }
        }
        
        // Currents
        if (OSDictionary *currents = OSDynamicCast(OSDictionary, configuration->getObject("Currents"))) {
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(currents);
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, currents->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                    if (FakeSMCSensor *sensor = addSensor(key, kFakeSMCCategoryCurrent, kFakeSMCCurrentSensor, methods->getCount())) {
                        methods->setObject(method);//sensor->getKey(), method);
                    }
                    else {
                        ACPISensorsErrorLog("Failed to register current sensor \"%s\" for method \"%s\"", key->getCStringNoCopy(), method->getCStringNoCopy());
                    }
                }
            }
        }
        
        // Powers
        if (OSDictionary *powers = OSDynamicCast(OSDictionary, configuration->getObject("Powers"))) {
            
            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(powers);
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, powers->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                    if (FakeSMCSensor *sensor = addSensor(key, kFakeSMCCategoryPower, kFakeSMCPowerSensor, methods->getCount())) {
                        methods->setObject(method);//sensor->getKey(), method);
                    }
                    else {
                        ACPISensorsErrorLog("Failed to register power sensor \"%s\" for method \"%s\"", key->getCStringNoCopy(), method->getCStringNoCopy());
                    }
                }
            }
        }
        
        // Tachometers
        if (OSDictionary *fans = OSDynamicCast(OSDictionary, configuration->getObject("Tachometers"))) {

            OSCollectionIterator *iterator = OSCollectionIterator::withCollection(fans);
            
            OSDictionary* fanNames = OSDynamicCast(OSDictionary, configuration->getObject("Fan Names"));
            
            while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                
                OSString *method = OSDynamicCast(OSString, fans->getObject(key));
                
                if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                    OSString* name = NULL;

                    if (fanNames)
                        name = OSDynamicCast(OSString, fanNames->getObject(key));
                    
                    if (FakeSMCSensor *sensor = addTachometer(methods->getCount(), name ? name->getCStringNoCopy() : 0)) {
                        methods->setObject(method);//sensor->getKey(), method);
                    }
                    else {
                        ACPISensorsErrorLog("Failed to register tachometer sensor for method \"%s\"", method->getCStringNoCopy());
                    }
                }
            }
        }
    }
    else ACPISensorsErrorLog("no valid configuration provided");
    
    
    if (methods->getCount())
        ACPISensorsInfoLog("%d sensor%s added", methods->getCount(), methods->getCount() > 1 ? "s" : "");
    
	registerService();
    
    ACPISensorsInfoLog("started");

	return true;	
}