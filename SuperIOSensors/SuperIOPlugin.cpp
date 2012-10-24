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
#include "SuperIOPlugin.h"
#include "SuperIO.h"
#include "OEMInfo.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndAbstractStructors(SuperIOPlugin, FakeSMCPlugin)

bool SuperIOPlugin::addSensorFromConfigurationNode(OSObject *configuration, const char *name, const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index)
{
    float reference = 0, gain = 0, offset = 0;
    
    if (configuration) {
        if (OSString *configName = OSDynamicCast(OSString, configuration)) {
            if (!configName->isEqualTo(name))
                return false;
        }
        else if (OSDictionary *configDict = OSDynamicCast(OSDictionary, configuration)) {
            if ((configName = OSDynamicCast(OSString, configDict->getObject("name")))) {
                if (configName->isEqualTo(name)) {
                    if (OSNumber *number = OSDynamicCast(OSNumber, configDict->getObject("reference")))
                        reference = (float)number->unsigned64BitValue() / 1000.0f;
                    
                    if (OSNumber *number = OSDynamicCast(OSNumber, configDict->getObject("gain")))
                        gain = (float)number->unsigned64BitValue() / 1000.0f;
                    
                    if (OSNumber *number = OSDynamicCast(OSNumber, configDict->getObject("offset")))
                        offset = (float)number->unsigned64BitValue() / 1000.0f;
                }
                else return false;
            }
            else return false;
        }
        else return false;
    }
    
    if (!this->addSensor(key, type, size, group, index, reference, gain, offset)) {
        HWSensorsWarningLog("failed to add %s sensor", name);
        return false;
    }
    
    return true;
}

bool SuperIOPlugin::addTemperatureSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding temperature sensors...");
    
    for (int i = 0; i < temperatureSensorsLimit(); i++) 
    {				
        char key[8];

        snprintf(key, 8, "TEMPIN%X", i);
        
        if (OSObject* node = configuration->getObject(key)) {
            if (addSensorFromConfigurationNode(node, "CPU", KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "CPU Proximity", KEY_CPU_PROXIMITY_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "System", KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "PCH", KEY_PCH_DIE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "Ambient", KEY_AMBIENT_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i))
                continue;
            
            for (int j = 1; j <= 0xf; j++) {
                
                char sensorName[10];
                
                snprintf(sensorName, 10, "Ambient %X", j);
                snprintf(key, 5, KEY_FORMAT_AMBIENT_TEMPERATURE, j);
                
                if (addSensorFromConfigurationNode(node, sensorName, key, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i))
                    break;
            }
        }
    }

    return true;
}

bool SuperIOPlugin::addVoltageSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding voltage sensors...");
    
    SInt8 gpuIndex = getVacantGPUIndex();
    
    for (int i = 0; i < voltageSensorsLimit(); i++)
    {				
        char key[5];
        
        snprintf(key, 5, "VIN%X", i);
        
        if (OSObject* node = configuration->getObject(key)) {
            if (addSensorFromConfigurationNode(node, "CPU", KEY_CPU_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "Memory", KEY_MEMORY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "Main 12V", KEY_MAIN_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "PCIe 12V", KEY_PCIE_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "Main 5V", KEY_MAIN_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "Standby 5V", KEY_STANDBY_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "Main 3V", KEY_MAIN_3V3_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "Auxiliary 3V", KEY_AUXILIARY_3V3V_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;
            if (addSensorFromConfigurationNode(node, "CMOS Battery", KEY_POWERBATTERY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                continue;

            if (gpuIndex > -1) {
                snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, gpuIndex);
                
                if(addSensorFromConfigurationNode(node, "GPU", key, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    continue;
            }
            
            for (int j = 0; j <= 0xf; j++) {
                char sensorName[17];
                snprintf(sensorName, 17, "CPU VRM Supply %X", j);
                snprintf(key, 5, KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE, j);
                
                if (addSensorFromConfigurationNode(node, sensorName, key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    break;
            }
            
            for (int j = 0; j <= 0xf; j++) {
                char sensorName[15];                
                snprintf(sensorName, 15, "Power Supply %X", j);
                snprintf(key, 5, KEY_FORMAT_POWERSUPPLY_VOLTAGE, j);
                
                if (addSensorFromConfigurationNode(node, sensorName, key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    break;
            }
        }
    }

    return true;
}

bool SuperIOPlugin::addTachometerSensors(OSDictionary *configuration)
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
                HWSensorsWarningLog("failed to add tachometer sensor %d", i);
    }
    
    return true;
}

UInt8 SuperIOPlugin::temperatureSensorsLimit()
{
    return 3;
}

UInt8 SuperIOPlugin::voltageSensorsLimit()
{
    return 9;
}

UInt8 SuperIOPlugin::tachometerSensorsLimit()
{
    return 5;
}

float SuperIOPlugin::readTemperature(UInt32 index)
{
	return 0;
}

float SuperIOPlugin::readVoltage(UInt32 index)
{
	return 0;
}

float SuperIOPlugin::readTachometer(UInt32 index)
{
	return 0;
}

float SuperIOPlugin::getSensorValue(FakeSMCSensor *sensor)
{
    float value = 0;
    
    if (sensor) {
        switch (sensor->getGroup()) {
            case kSuperIOTemperatureSensor:
                value = sensor->getOffset() + readTemperature(sensor->getIndex());
                break;
                
            case kSuperIOVoltageSensor:
                value = readVoltage(sensor->getIndex());
                value = sensor->getOffset() + value + (value - sensor->getReference()) * sensor->getGain();
                break;
                
            case kFakeSMCTachometerSensor:
                value = readTachometer(sensor->getIndex());
                break;
        }
    }
    
	return value;
}

bool SuperIOPlugin::initialize()
{
    return true;
}

bool SuperIOPlugin::init(OSDictionary *properties)
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

IOService *SuperIOPlugin::probe(IOService *provider, SInt32 *score)
{
    return super::probe(provider, score);
}

bool SuperIOPlugin::start(IOService *provider)
{	
	if (!super::start(provider)) 
        return false;
    
    OSNumber *number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOHWMAddress));
    
    if (!number || !(address = number->unsigned16BitValue())) {
        HWSensorsFatalLog("wrong address provided");
        return false;
    }
    
    number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOControlPort));
    
    if (!number || !(port = number->unsigned8BitValue())) {
        HWSensorsFatalLog("wrong port provided");
        return false;
    }
    
    number = OSDynamicCast(OSNumber, provider->getProperty(kSuperIOModelValue));
    
    if (!number || !(model = number->unsigned16BitValue())) {
        HWSensorsFatalLog("wrong model provided");
        return false;
    }
    
    OSString *string = OSDynamicCast(OSString, provider->getProperty(kSuperIOModelName));
    
    if (!string || !(modelName = string->getCStringNoCopy())) {
        HWSensorsFatalLog("wrong model name provided");
        return false;
    }
    
    string = OSDynamicCast(OSString, provider->getProperty(kSuperIOVendorName));
    
    if (!string || !(vendorName = string->getCStringNoCopy())) {
        HWSensorsFatalLog("wrong vendor name provided");
        return false;
    }
    
    if (!initialize())
        return false;
    
    OSString *modelString = OSString::withCString(modelName);

	if (OSDictionary *configuration = getConfigurationNode(modelString))
    {
        addTemperatureSensors(configuration);
        addVoltageSensors(configuration);
        addTachometerSensors(configuration);
    }
    else HWSensorsWarningLog("no sensors configuration provided");
    
    OSSafeReleaseNULL(modelString);
    
    registerService();
    
    HWSensorsInfoLog("started");

	return true;
}