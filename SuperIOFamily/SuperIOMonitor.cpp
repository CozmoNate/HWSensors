/*
 *  SuperIOFamily.cpp
 *  HWSensors
 *
 *  Created by mozo on 08/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "SuperIOMonitor.h"

#include <architecture/i386/pio.h>

#include "FakeSMCDefinitions.h"
#include "SuperIO.h"

#define Debug FALSE

#define LogPrefix "SuperIOMonitor: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super FakeSMCPlugin
OSDefineMetaClassAndAbstractStructors(SuperIOMonitor, FakeSMCPlugin)

bool SuperIOMonitor::addTemperatureSensors(OSDictionary *configuration)
{
    DebugLog("adding temperature sensors...");
    
    for (int i = 0; i < temperatureSensorsLimit(); i++) 
    {				
        char key[8];
        
        snprintf(key, 8, "TEMPIN%X", i);
        
        if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key))) {
            if (name->isEqualTo("CPU")) {
                if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i))
                    WarningLog("can't add CPU temperature sensor");
            }
            else if (name->isEqualTo("System")) {				
                if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor,i))
                    WarningLog("can't add System temperature sensor");
            }
            else if (name->isEqualTo("Ambient")) {				
                if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor,i))
                    WarningLog("can't add Ambient temperature sensor");
            }
        }
    }

    return true;
}

bool SuperIOMonitor::addVoltageSensors(OSDictionary *configuration)
{
    DebugLog("adding voltage sensors...");
    
    for (int i = 0; i < voltageSensorsLimit(); i++)
    {				
        char key[5];
        
        snprintf(key, 5, "VIN%X", i);
        
        if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key))) {
            if (name->isEqualTo("CPU")) {
                if (!addSensor(KEY_CPU_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add  CPU voltage sensor");
            }
            else if (name->isEqualTo("Memory")) {
                if (!addSensor(KEY_MEMORY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add Memory voltage sensor");
            }
            else if (name->isEqualTo("Main 12V")) {
                if (!addSensor(KEY_MAIN_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add Main 12V voltage sensor");
            }
            else if (name->isEqualTo("PCIe 12V")) {
                if (!addSensor(KEY_PCIE_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add PCIe 12V voltage sensor");
            }
            else if (name->isEqualTo("Main 5V")) {
                if (!addSensor(KEY_MAIN_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add Main 5V voltage sensor");
            }
            else if (name->isEqualTo("Standby 5V")) {
                if (!addSensor(KEY_STANDBY_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add Standby 5V voltage sensor");
            }
            else if (name->isEqualTo("Main 3.3V")) {
                if (!addSensor(KEY_MAIN_3V3_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add Main 3.3V voltage sensor");
            }
            else if (name->isEqualTo("Auxiliary 3.3V")) {
                if (!addSensor(KEY_AUXILIARY_3V3V_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add Auxiliary 3.3V voltage sensor");
            }
            else if (name->isEqualTo("Battery")) {
                if (!addSensor(KEY_POWERBATTERY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add Battery voltage sensor!");
            }
            
            for (int j = 0; j <= 0xf; j++) {
                
                snprintf(key, 5, "CPU VRM Supply %X", j);
                
                if (name->isEqualTo(key)) {
                    snprintf(key, 5, KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE, j);
                    if (!addSensor(key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                        WarningLog("can't add VRM%X voltage Sensor!", j);
                }
                else {
                    snprintf(key, 5, "Power Supply %X", j);
                    if (name->isEqualTo(key)) {
                        snprintf(key, 5, KEY_FORMAT_POWERSUPPLY_VOLTAGE, j);
                        if (!addSensor(key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                            WarningLog("can't add PWR%X voltage Sensor!", j);
                    }
                }
            }
        }
    }

    return true;
}

bool SuperIOMonitor::addTachometerSensors(OSDictionary *configuration)
{
    DebugLog("adding tachometer sensors...");
    
    for (int i = 0; i < tachometerSensorsLimit(); i++) {
        OSString* name = NULL;
        
        char key[7];
            
        snprintf(key, 7, "FANIN%X", i);
            
        name = OSDynamicCast(OSString, configuration->getObject(key));
        
        UInt64 nameLength = name ? name->getLength() : 0;
        
        if (readTachometer(i) > 10 || nameLength > 0)
            if (!addTachometer(i, (nameLength > 0 ? name->getCStringNoCopy() : 0)))
                WarningLog("error adding tachometer sensor %d", i);
    }
    
    return true;
}

UInt8 SuperIOMonitor::temperatureSensorsLimit()
{
    return 3;
}

UInt8 SuperIOMonitor::voltageSensorsLimit()
{
    return 9;
}

UInt8 SuperIOMonitor::tachometerSensorsLimit()
{
    return 5;
}

float SuperIOMonitor::readTemperature(UInt32 index)
{
	return 0;
}

float SuperIOMonitor::readVoltage(UInt32 index)
{
	return 0;
}

float SuperIOMonitor::readTachometer(UInt32 index)
{
	return 0;
}

float SuperIOMonitor::getSensorValue(FakeSMCSensor *sensor)
{
    if (sensor)
        switch (sensor->getGroup()) {
            case kSuperIOTemperatureSensor:
                return readTemperature(sensor->getIndex());
                
            case kSuperIOVoltageSensor:
                return readVoltage(sensor->getIndex());
                
            case kFakeSMCTachometerSensor:
                return readTachometer(sensor->getIndex());
        }
    
	return 0;
}

bool SuperIOMonitor::initialize()
{
    return true;
}

bool SuperIOMonitor::init(OSDictionary *properties)
{
	if (!super::init(properties))
		return false;
	
    address = 0;
    port = 0;
   	model = 0;
    
    modelName = "unknown";
    vendorName = "unknown";
        
	return true;
}

IOService *SuperIOMonitor::probe(IOService *provider, SInt32 *score)
{
	return super::probe(provider, score);
}

bool SuperIOMonitor::start(IOService *provider)
{	
	if (!super::start(provider)) 
        return false;
    
    OSNumber *number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOHWMAddress));
    
    if (!number || !(address = number->unsigned16BitValue())) {
        WarningLog("wrong address provided");
        return false;
    }
    
    number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOControlPort));
    
    if (!number || !(port = number->unsigned8BitValue())) {
        WarningLog("wrong port provided");
        return false;
    }
    
    number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOModelValue));
    
    if (!number || !(model = number->unsigned16BitValue())) {
        WarningLog("wrong model provided");
        return false;
    }
    
    OSString *string = OSDynamicCast(OSString, provider->getProperty(kSuperIOModelName));
    
    if (!string || !(modelName = string->getCStringNoCopy())) {
        WarningLog("wrong model name provided");
        return false;
    }
    
    string = OSDynamicCast(OSString, provider->getProperty(kSuperIOVendorName));
    
    if (!string || !(vendorName = string->getCStringNoCopy())) {
        WarningLog("wrong vendor name provided");
        return false;
    }
    
    if (!initialize())
        return false;
    
    //InfoLog("found %s %s", vendorName, getModelName());
        
    OSDictionary* list = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration"));
    OSDictionary* configuration = list ? OSDynamicCast(OSDictionary, list->getObject(modelName)) : 0;
	
    if (list && !configuration) 
        configuration = OSDynamicCast(OSDictionary, list->getObject("Default"));
    
	if (configuration) {    
        addTemperatureSensors(configuration);
        addVoltageSensors(configuration);
        addTachometerSensors(configuration);
    }
        
    registerService();

	return true;
}