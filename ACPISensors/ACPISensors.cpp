/*
 *  ACPIMonitor.cpp
 *  HWSensors
 *
 *  Created by mozo on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "ACPISensors.h"

#include "FakeSMCDefinitions.h"
#include "FakeSMCValueEncoder.h"

#define Debug FALSE

#define LogPrefix "ACPISensors: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPIMonitor, FakeSMCPlugin)

float ACPIMonitor::getSensorValue(FakeSMCSensor *sensor)
{
    UInt64 value;
    
    switch(sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            if (acpiDevice->evaluateInteger(OSDynamicCast(OSSymbol, temperatures->getObject(sensor->getKey())), &value))
                return (float)value;

            break;
        case kFakeSMCVoltageSensor:
            if (acpiDevice->evaluateInteger(OSDynamicCast(OSSymbol, temperatures->getObject(sensor->getKey())), &value))
                // all floating point values returned from ACPI should be 
                // multiplied to 1000 (simple integer encoding)
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

IOService *ACPIMonitor::probe(IOService *provider, SInt32 *score)
{
    if (super::probe(provider, score) != this) 
        return 0;
    
    isActive = true;
    
    return this;
}

bool ACPIMonitor::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;

	acpiDevice = (IOACPIPlatformDevice *)provider;
	
	if (!acpiDevice) {
        WarningLog("ACPI device not ready");
        return true;
    }
    
    OSDictionary *config = OSDynamicCast(OSDictionary, getProperty("Keys Association"));
    
    // Temperatures
    temperatures = OSDynamicCast(OSDictionary, config->getObject("Temperatures"));
    
    if (temperatures) {
        OSCollectionIterator *iterator = OSCollectionIterator::withCollection(temperatures);
        
        iterator->reset();
        
        OSString *method = NULL;
        
        do {
            method = OSDynamicCast(OSString, iterator->getNextObject());
            
            OSString *key = OSDynamicCast(OSString, temperatures->getObject(method));
            
            if (key) 
                if (!addSensor(key->getCStringNoCopy(), TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0))
                    WarningLog("can't add temperature sensor for method %s with key %s", method->getCStringNoCopy(), key->getCStringNoCopy());
            
        } while (method);
    }
        
        
    // Voltages
    voltages = OSDynamicCast(OSDictionary, config->getObject("Voltages"));
    
    if (voltages) {
        OSCollectionIterator *iterator = OSCollectionIterator::withCollection(temperatures);
        
        iterator->reset();
        
        OSString *method = OSDynamicCast(OSString, iterator->getNextObject());
        
        while (method) {
            
            OSString *key = OSDynamicCast(OSString, temperatures->getObject(method));
            
            if (key) 
                if (!addSensor(key->getCStringNoCopy(), TYPE_FP4C, 2, kFakeSMCTemperatureSensor, 0))
                    WarningLog("can't add voltage sensor for method %s with key %s", method->getCStringNoCopy(), key->getCStringNoCopy());
            
            method = OSDynamicCast(OSString, iterator->getNextObject());
        };
    }
        
    // Fans
    OSArray* fanNames = OSDynamicCast(OSArray, getProperty("Fan Names"));
    
    if (fanNames)
        for (int i=0; i<10; i++) 
        {
            char key[5];
            
            snprintf(key, 5, ACPI_NAME_FORMAT_TACHOMETER, i);
            
            if (kIOReturnSuccess == acpiDevice->validateObject(key)) {
                OSString* name = NULL;
                
                if (fanNames)
                    name = OSDynamicCast(OSString, fanNames->getObject(i));
                
                if (!addTachometer(i, name ? name->getCStringNoCopy() : 0))
                    WarningLog("Can't add tachometer sensor %d", i);
            }
        }

	registerService(0);

	return true;	
}