/*
 *  SuperIOFamily.cpp
 *  HWSensors
 *
 *  Created by kozlek on 08/10/10.
 *
 */

//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "FakeSMCDefinitions.h"
#include "SuperIOMonitor.h"
#include "SuperIO.h"

/*#define Debug FALSE

#define LogPrefix "SuperIOMonitor: "
#define HWSensorsDebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define HWSensorsWarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define HWSensorsInfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)*/

#define super FakeSMCPlugin
OSDefineMetaClassAndAbstractStructors(SuperIOMonitor, FakeSMCPlugin)

inline bool process_sensor_entry(OSObject *object, OSString **name, float *reference, float *gain, float *offset)
{
    if ((*name = OSDynamicCast(OSString, object))) {
        return true;
    }
    else if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, object))
        if ((*name = OSDynamicCast(OSString, dictionary->getObject("name")))) {
            if (OSNumber *number = OSDynamicCast(OSNumber, dictionary->getObject("reference")))
                *reference = (float)number->unsigned64BitValue() / 1000.0f;
            
            if (OSNumber *number = OSDynamicCast(OSNumber, dictionary->getObject("gain")))
                *gain = (float)number->unsigned64BitValue() / 1000.0f;
            
            if (OSNumber *number = OSDynamicCast(OSNumber, dictionary->getObject("offset")))
                *offset = (float)number->unsigned64BitValue() / 1000.0f;
            
            return true;
        }
    
    return false;
}

bool SuperIOMonitor::addTemperatureSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding temperature sensors...");
    
    for (int i = 0; i < temperatureSensorsLimit(); i++) 
    {				
        char key[8];
        OSString* name;
        float reference = 0.0f;
        float gain = 0.0f;
        float offset = 0.0f;

        snprintf(key, 8, "TEMPIN%X", i);
        
        if (process_sensor_entry(configuration->getObject(key), &name, &reference, &gain, &offset)) {
            if (name->isEqualTo("CPU")) {
                if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add CPU temperature sensor");
            }
            if (name->isEqualTo("CPU Proximity")) {
                if (!addSensor(KEY_CPU_PROXIMITY_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add CPU Proximity temperature sensor");
            }
            else if (name->isEqualTo("System")) {				
                if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add System temperature sensor");
            }
            else if (name->isEqualTo("Ambient")) {
                if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add Ambient temperature sensor");
            }
            else if (name->isEqualTo("PCH")) {				
                if (!addSensor(KEY_PCH_DIE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add PCH temperature sensor");
            }
            else {
                for (int j = 1; j <= 0xf; j++) {
                    
                    char caption[64];
                    
                    snprintf(caption, 10, "Ambient %X", j);
                    
                    if (name->isEqualTo(caption)) {
                        snprintf(key, 5, KEY_FORMAT_AMBIENT_TEMPERATURE, j);
                        if (!addSensor(key, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                            HWSensorsWarningLog("can't add Ambient %X temperature sensor", j);
                    }
                }
            }
        }
    }

    return true;
}

bool SuperIOMonitor::addVoltageSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding voltage sensors...");
    
    for (int i = 0; i < voltageSensorsLimit(); i++)
    {				
        char key[5];
        OSString* name;
        float reference = 0.0f;
        float gain = 0.0f;
        float offset = 0.0f;
        
        snprintf(key, 5, "VIN%X", i);
        
        if (process_sensor_entry(configuration->getObject(key), &name, &reference, &gain, &offset)) {
            if (name->isEqualTo("CPU")) {
                if (!addSensor(KEY_CPU_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add CPU voltage sensor");
            }
            else if (name->isEqualTo("Memory")) {
                if (!addSensor(KEY_MEMORY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add Memory voltage sensor");
            }
            else if (name->isEqualTo("Main 12V")) {
                if (!addSensor(KEY_MAIN_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add Main 12V voltage sensor");
            }
            else if (name->isEqualTo("PCIe 12V")) {
                if (!addSensor(KEY_PCIE_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add PCIe 12V voltage sensor");
            }
            else if (name->isEqualTo("Main 5V")) {
                if (!addSensor(KEY_MAIN_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add Main 5V voltage sensor");
            }
            else if (name->isEqualTo("Standby 5V")) {
                if (!addSensor(KEY_STANDBY_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add Standby 5V voltage sensor");
            }
            else if (name->isEqualTo("Main 3V")) {
                if (!addSensor(KEY_MAIN_3V3_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add Main 3V voltage sensor");
            }
            else if (name->isEqualTo("Auxiliary 3V")) {
                if (!addSensor(KEY_AUXILIARY_3V3V_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add Auxiliary 3V voltage sensor");
            }
            else if (name->isEqualTo("Power/Battery")) {
                if (!addSensor(KEY_POWERBATTERY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    HWSensorsWarningLog("can't add Power/Battery voltage sensor");
            }
            else if (name->isEqualTo("GPU")) {
                SInt8 index = getVacantGPUIndex();
                
                if (index > -1) {
                    snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, index);
                    
                    if (!addSensor(key, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                        HWSensorsWarningLog("can't add GPU voltage sensor");
                }
                else HWSensorsWarningLog("failed to obtain vacant GPU index");
            }
            
            
            for (int j = 0; j <= 0xf; j++) {
                
                char caption[32];
                
                snprintf(caption, 15, "Power Supply %X", j);
                
                if (name->isEqualTo(caption)) {
                    snprintf(key, 5, KEY_FORMAT_POWERSUPPLY_VOLTAGE, j);
                    if (!addSensor(key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                        HWSensorsWarningLog("can't add PWR%X voltage sensor", j);
                }
                else {
                
                    snprintf(caption, 17, "CPU VRM Supply %X", j);
                    
                    if (name->isEqualTo(caption)) {
                        snprintf(key, 5, KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE, j);
                        if (!addSensor(key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                            HWSensorsWarningLog("can't add VRM%X voltage sensor", j);
                    }
                }
            }
        }
    }

    return true;
}

bool SuperIOMonitor::addTachometerSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding tachometer sensors...");
    
    for (int i = 0; i < tachometerSensorsLimit(); i++) {
        OSString* name = NULL;
        
        char key[7];
            
        snprintf(key, 7, "FANIN%X", i);

        name = OSDynamicCast(OSString, configuration->getObject(key));
        
        UInt64 nameLength = name ? name->getLength() : 0;
        
        if (readTachometer(i) > 10 || nameLength > 0)
            if (!addTachometer(i, (nameLength > 0 ? name->getCStringNoCopy() : 0)))
                HWSensorsWarningLog("error adding tachometer sensor %d", i);
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
    float value = 0;
    
    if (sensor)
        switch (sensor->getGroup()) {
            case kSuperIOTemperatureSensor:
                value = readTemperature(sensor->getIndex());
                break;
                
            case kSuperIOVoltageSensor:
                value = readVoltage(sensor->getIndex());
                break;
                
            case kFakeSMCTachometerSensor:
                value = readTachometer(sensor->getIndex());
                break;
        }
    
    value = sensor->getOffset() + value + (value - sensor->getReference()) * sensor->getGain();
    
	return value;
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

OSDictionary *SuperIOMonitor::lookupConfiguration(OSDictionary *root, OSString *name)
{
    OSDictionary *configuration = NULL;
    
    if (root && name) {
        HWSensorsDebugLog("looking up for configuration node: %s", name->getCStringNoCopy());
        
        if (!(configuration = OSDynamicCast(OSDictionary, root->getObject(name))))
            if (OSString *link = OSDynamicCast(OSString, root->getObject(name)))
                configuration = lookupConfiguration(root, link);    
    }
    
    return configuration;
}

OSDictionary *SuperIOMonitor::lookupConfiguration(OSDictionary *root, const char *name)
{
    OSDictionary *configuration = NULL;
    
    if (root && name) {
        OSString *nameNode = OSString::withCStringNoCopy(name);
        
        configuration = lookupConfiguration(root, nameNode);
        
        OSSafeReleaseNULL(nameNode);
    }
    
    return configuration;
}

bool SuperIOMonitor::start(IOService *provider)
{	
	if (!super::start(provider)) 
        return false;
    
    OSNumber *number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOHWMAddress));
    
    if (!number || !(address = number->unsigned16BitValue())) {
        HWSensorsWarningLog("wrong address provided");
        return false;
    }
    
    number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOControlPort));
    
    if (!number || !(port = number->unsigned8BitValue())) {
        HWSensorsWarningLog("wrong port provided");
        return false;
    }
    
    number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOModelValue));
    
    if (!number || !(model = number->unsigned16BitValue())) {
        HWSensorsWarningLog("wrong model provided");
        return false;
    }
    
    OSString *string = OSDynamicCast(OSString, provider->getProperty(kSuperIOModelName));
    
    if (!string || !(modelName = string->getCStringNoCopy())) {
        HWSensorsWarningLog("wrong model name provided");
        return false;
    }
    
    string = OSDynamicCast(OSString, provider->getProperty(kSuperIOVendorName));
    
    if (!string || !(vendorName = string->getCStringNoCopy())) {
        HWSensorsWarningLog("wrong vendor name provided");
        return false;
    }
    
    if (!initialize())
        return false;

    OSString *manufacturerName = OSDynamicCast(OSString, provider->getProperty("mb-manufacturer"));
    OSString *productName = OSDynamicCast(OSString, provider->getProperty("mb-product"));
    
    OSDictionary *configuration = NULL;
    
    if (OSDictionary *list = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration")))
    {
        if (OSDictionary *manufacturer = OSDynamicCast(OSDictionary, list->getObject(manufacturerName)))
            if (!(configuration = lookupConfiguration(manufacturer, productName)))
                if (!(configuration = lookupConfiguration(manufacturer, modelName)))
                    configuration = lookupConfiguration(manufacturer, "Default");
        
        if (!configuration && !(configuration = lookupConfiguration(list, modelName)))
            configuration = lookupConfiguration(list, "Default");
    }

	if (configuration) {    
        addTemperatureSensors(configuration);
        addVoltageSensors(configuration);
        addTachometerSensors(configuration);
        registerService();
    }
    else HWSensorsWarningLog("no sensors configuration provided");
    
    HWSensorsInfoLog("started");

	return true;
}