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

// Sensor

OSDefineMetaClassAndStructors(SuperIOSensor, FakeSMCSensor)

SuperIOSensor *SuperIOSensor::withOwner(FakeSMCPlugin *aOwner, const char *aKey, const char *aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference, float aGain, float aOffset)
{
	SuperIOSensor *me = new SuperIOSensor;
	
    if (me && !me->initWithOwner(aOwner, aKey, aType, aSize, aGroup, aIndex, aReference, aGain, aOffset)) {
        me->release();
        return 0;
    }
	
    return me;
}

bool SuperIOSensor::initWithOwner(FakeSMCPlugin *aOwner, const char *aKey, const char *aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference, float aGain, float aOffset)
{
	if (!FakeSMCSensor::initWithOwner(aOwner, aKey, aType, aSize, aGroup, aIndex))
        return false;
        
    reference = aReference;
    gain = aGain;
    offset = aOffset;
	
	return true;
}

float SuperIOSensor::getReference()
{
    return reference;
}

float SuperIOSensor::getGain()
{
    return gain;
}

float SuperIOSensor::getOffset()
{
    return offset;
}

// Plugin

#define super FakeSMCPlugin
OSDefineMetaClassAndAbstractStructors(SuperIOPlugin, FakeSMCPlugin)

bool SuperIOPlugin::parseConfigurationNode(OSObject *object, OSString **name, float *reference, float *gain, float *offset)
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

bool SuperIOPlugin::matchSensorToNodeName(OSString *node, const char *name, const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    if (node->isEqualTo(name)) {
        if (addSensor(key, type, size, group, index, reference, gain, offset))
            return true;
        else HWSensorsWarningLog("failed to add %s %s sensor", name, group == kSuperIOTemperatureSensor ? "temperature" : group == kSuperIOVoltageSensor ? "voltage" : "unknown");
    }
    
    return false;
}

SuperIOSensor *SuperIOPlugin::addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    if (getSensor(key)) {
        HWSensorsDebugLog("will not add handler for key %s, key already handled", key);
		return NULL;
    }
	
    if (SuperIOSensor *sensor = SuperIOSensor::withOwner((FakeSMCPlugin*)this, key, type, size, group, index, reference, gain, offset)) {
        if (FakeSMCPlugin::addSensor(sensor))
            return sensor;
        else
            sensor->release();
    }
	
	return NULL;
}

bool SuperIOPlugin::addTemperatureSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding temperature sensors...");
    
    for (int i = 0; i < temperatureSensorsLimit(); i++) 
    {				
        char key[8];
        OSString* nodeName;
        float reference = 0.0f;
        float gain = 0.0f;
        float offset = 0.0f;

        snprintf(key, 8, "TEMPIN%X", i);
        
        if (parseConfigurationNode(configuration->getObject(key), &nodeName, &reference, &gain, &offset)) {
            if (matchSensorToNodeName(nodeName, "CPU", KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "CPU Proximity", KEY_CPU_PROXIMITY_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "System", KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "Ambient", KEY_AMBIENT_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "PCH", KEY_PCH_DIE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                continue;
            
            for (int j = 1; j <= 0xf; j++) {
                
                char sensorName[64];
                
                snprintf(sensorName, 10, "Ambient %X", j);
                snprintf(key, 5, KEY_FORMAT_AMBIENT_TEMPERATURE, j);
                
                if (matchSensorToNodeName(nodeName, sensorName, key, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i, reference, gain, offset))
                    break;
            }
        }
    }

    return true;
}

bool SuperIOPlugin::addVoltageSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding voltage sensors...");
    
    for (int i = 0; i < voltageSensorsLimit(); i++)
    {				
        char key[5];
        OSString* nodeName;
        float reference = 0.0f;
        float gain = 0.0f;
        float offset = 0.0f;
        
        snprintf(key, 5, "VIN%X", i);
        
        if (parseConfigurationNode(configuration->getObject(key), &nodeName, &reference, &gain, &offset)) {
            if (matchSensorToNodeName(nodeName, "CPU", KEY_CPU_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "Memory", KEY_MEMORY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "Main 12V", KEY_MAIN_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "PCIe 12V", KEY_PCIE_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "Main 5V", KEY_MAIN_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "Standby 5V", KEY_STANDBY_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "Main 3V", KEY_MAIN_3V3_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "Auxiliary 3V", KEY_AUXILIARY_3V3V_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            if (matchSensorToNodeName(nodeName, "CMOS Battery", KEY_POWERBATTERY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                continue;
            
            if (nodeName->isEqualTo("GPU")) {
                SInt8 index = getVacantGPUIndex();
                
                if (index > -1) {
                    snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, index);
                    
                    if (!addSensor(key, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                        HWSensorsWarningLog("failed to add GPU %d voltage sensor", index);
                }
                else HWSensorsErrorLog("failed to obtain vacant GPU index");
                
                continue;
            }
 
            for (int j = 0; j <= 0xf; j++) {
                char sensorName[32];
                
                snprintf(sensorName, 17, "CPU VRM Supply %X", j);
                snprintf(key, 5, KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE, j);
                
                if (matchSensorToNodeName(nodeName, sensorName, key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
                    break;
            }
            
            for (int j = 0; j <= 0xf; j++) {
                char sensorName[32];
                
                snprintf(sensorName, 15, "Power Supply %X", j);
                snprintf(key, 5, KEY_FORMAT_POWERSUPPLY_VOLTAGE, j);
                
                if (matchSensorToNodeName(nodeName, sensorName, key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i, reference, gain, offset))
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
    
    SuperIOSensor *superio = (SuperIOSensor*)sensor;
    
    value = superio->getOffset() + value + (value - superio->getReference()) * superio->getGain();
    
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

	if (OSDictionary *configuration = getConfigurationNode(
                                                           OSDynamicCast(OSString, provider->getProperty("mb-manufacturer")),
                                                           OSDynamicCast(OSString, provider->getProperty("mb-product")),
                                                           modelString))
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