/*
 *  ACPISensors.cpp
 *  ACPISensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "ACPISensors.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPISensors, FakeSMCPlugin)

void ACPISensors::addSensorsFromDictionary(OSDictionary *dictionary, kFakeSMCCategory category)
{
    if (dictionary) {
        
        UInt32 group = 0;
        const char *title = NULL;
        
        switch (category) {
            case kFakeSMCCategoryTemperature:
                group = kFakeSMCTemperatureSensor;
                title = "temperature";
                break;
                
            case kFakeSMCCategoryFrequency:
                group = kFakeSMCFrequencySensor;
                title = "frequency";
                break;
                
            case kFakeSMCCategoryVoltage:
                group = kFakeSMCVoltageSensor;
                title = "voltage";
                break;
                
            case kFakeSMCCategoryCurrent:
                group = kFakeSMCCurrentSensor;
                title = "amperage";
                break;
                
            case kFakeSMCCategoryPower:
                group = kFakeSMCPowerSensor;
                title = "power";
                break;
                
            case kFakeSMCCategoryFan:
                group = kFakeSMCTachometerSensor;
                title = "tachometer";
                break;
                
            default:
                return;
        }
        
        OSCollectionIterator *iterator = OSCollectionIterator::withCollection(dictionary);
        
        while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
            
            OSString *method = OSDynamicCast(OSString, dictionary->getObject(key));
            
            if (method && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                FakeSMCSensor *sensor = NULL;
                
                if (category == kFakeSMCCategoryFan) {
                    sensor = addTachometer(methods->getCount(), key->getCStringNoCopy());
                }
                else {
                    sensor = addSensor(key, category, group, methods->getCount());
                }
                
                if (sensor) {
                    methods->setObject(method);//sensor->getKey(), method);
                }
                else {
                    ACPISensorsErrorLog("Failed to register %s sensor \"%s\" for method \"%s\"", title, key->getCStringNoCopy(), method->getCStringNoCopy());
                }
            }
        }
    }
}

void ACPISensors::addSensorsFromArray(OSArray *array, kFakeSMCCategory category)
{
    if (array) {
        
        UInt32 group = 0;
        const char *title = NULL;
        
        switch (category) {
            case kFakeSMCCategoryTemperature:
                group = kFakeSMCTemperatureSensor;
                title = "temperature";
                break;
                
            case kFakeSMCCategoryFrequency:
                group = kFakeSMCFrequencySensor;
                title = "frequency";
                break;
                
            case kFakeSMCCategoryVoltage:
                group = kFakeSMCVoltageSensor;
                title = "voltage";
                break;
                
            case kFakeSMCCategoryCurrent:
                group = kFakeSMCCurrentSensor;
                title = "amperage";
                break;
                
            case kFakeSMCCategoryPower:
                group = kFakeSMCPowerSensor;
                title = "power";
                break;
                
            case kFakeSMCCategoryFan:
                group = kFakeSMCTachometerSensor;
                title = "tachometer";
                break;
                
            default:
                return;
        }
        
        for (UInt32 index = 0; index + 1 < array->getCount(); index += 2) {
            if (OSString *key = OSDynamicCast(OSString, array->getObject(index))) {
                if (OSString *method = OSDynamicCast(OSString, array->getObject(index + 1))) {
                    FakeSMCSensor *sensor = NULL;
                    
                    if (category == kFakeSMCCategoryFan) {
                        sensor = addTachometer(methods->getCount(), key->getCStringNoCopy());
                    }
                    else {
                        sensor = addSensor(key, category, group, methods->getCount());
                    }
                    
                    if (sensor) {
                        methods->setObject(method);//sensor->getKey(), method);
                    }
                    else {
                        ACPISensorsErrorLog("Failed to register %s sensor \"%s\" for method \"%s\"", title, key->getCStringNoCopy(), method->getCStringNoCopy());
                    }
                }
            }
        }
    }
}

bool ACPISensors::willReadSensorValue(FakeSMCSensor *sensor, float *outValue)
{
    UInt32 value = 0;
    OSString *method = NULL;
    
    if (sensor->getIndex() < methods->getCount() && (method = (OSString*)methods->getObject(sensor->getIndex()))) {
        if (kIOReturnSuccess == acpiDevice->evaluateInteger(method->getCStringNoCopy(), &value)) {
            switch(sensor->getGroup()) {
                case kFakeSMCTemperatureSensor:                    
                    // all temperatures returned from ACPI should be in Kelvins?
                    if (useKelvins)
                        *outValue = ((float)value - (float)0xAAC) / (float)0xA;
                    else
                        *outValue = (float)value;
                    break;
                    
                case kFakeSMCVoltageSensor:
                case kFakeSMCCurrentSensor:
                case kFakeSMCPowerSensor:
                    // all voltages returned from ACPI should be in millivolts?
                    *outValue = (float)value * 0.001f;
                    break;
            
                case kFakeSMCTachometerSensor:
                default:
                    *outValue = (float)value;
                    break;
            }
        }
        else return false;
    }
    else return false;
    
    return true;
}

bool ACPISensors::start(IOService * provider)
{
    ACPISensorsDebugLog("starting...");
    
	if (!super::start(provider))
        return false;

	if (!(acpiDevice = OSDynamicCast(IOACPIPlatformDevice, provider))) {
        ACPISensorsFatalLog("ACPI device not ready");
        return false;
    }

    methods = OSArray::withCapacity(0);
    
    // Try to load configuration provided by ACPI device
    OSObject *object = NULL;
        
    // Use Kelvins?
    if (kIOReturnSuccess == acpiDevice->evaluateObject("KLVN", &object) && object) {
        if (OSNumber *kelvins = OSDynamicCast(OSNumber, object)) {
            useKelvins = kelvins->unsigned8BitValue() == 1;
        }
    }
    
    // Parse temperature table
    if (kIOReturnSuccess == acpiDevice->evaluateObject("TEMP", &object) && object) {
        addSensorsFromArray(OSDynamicCast(OSArray, object), kFakeSMCCategoryTemperature);
    }
    else ACPISensorsDebugLog("temprerature description table (TEMP) not found");
    
    // Parse voltage table
    if (kIOReturnSuccess == acpiDevice->evaluateObject("VOLT", &object) && object) {
        addSensorsFromArray(OSDynamicCast(OSArray, object), kFakeSMCCategoryVoltage);
    }
    else ACPISensorsDebugLog("voltage description table (VOLT) not found");
    
    // Parse amperage table
    if (kIOReturnSuccess == acpiDevice->evaluateObject("AMPR", &object) && object) {
        addSensorsFromArray(OSDynamicCast(OSArray, object), kFakeSMCCategoryCurrent);
    }
    else ACPISensorsDebugLog("amperage description table (AMPR) not found");
    
    // Parse power table
    if (kIOReturnSuccess == acpiDevice->evaluateObject("POWR", &object) && object) {
        addSensorsFromArray(OSDynamicCast(OSArray, object), kFakeSMCCategoryPower);
    }
    else ACPISensorsDebugLog("power description table (POWR) not found");
    
    // Parse tachometer table
    if (kIOReturnSuccess == acpiDevice->evaluateObject("TACH", &object) && object) {
        addSensorsFromArray(OSDynamicCast(OSArray, object), kFakeSMCCategoryFan);
    }
    else ACPISensorsDebugLog("tachometer description table (TACH) not found");

    OSDictionary *configuration = NULL;

    // If nothing was found on ACPI device try to load configuration from info.plist
    if (object == NULL && (configuration = getConfigurationNode()))
    {
        if (OSDictionary *temperatures = OSDynamicCast(OSDictionary, configuration->getObject("Temperatures"))) {

            if (OSBoolean *kelvins = OSDynamicCast(OSBoolean, temperatures->getObject("UseKelvins"))) {
                useKelvins = kelvins->isTrue();
            }

            addSensorsFromDictionary(OSDynamicCast(OSDictionary, temperatures), kFakeSMCCategoryTemperature);
        }

        addSensorsFromDictionary(OSDynamicCast(OSDictionary, configuration->getObject("Voltages")), kFakeSMCCategoryVoltage);
        addSensorsFromDictionary(OSDynamicCast(OSDictionary, configuration->getObject("Currents")), kFakeSMCCategoryCurrent);
        addSensorsFromDictionary(OSDynamicCast(OSDictionary, configuration->getObject("Powers")), kFakeSMCCategoryPower);
        addSensorsFromDictionary(OSDynamicCast(OSDictionary, configuration->getObject("Tachometers")), kFakeSMCCategoryFan);
    }
    
    if (methods->getCount())
        ACPISensorsInfoLog("%d sensor%s added", methods->getCount(), methods->getCount() > 1 ? "s" : "");

	registerService();
    
    ACPISensorsInfoLog("started");

	return true;	
}