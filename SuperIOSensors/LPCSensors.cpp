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
#include "LPCSensors.h"
#include "SuperIO.h"
#include "OEMInfo.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndAbstractStructors(LPCSensors, FakeSMCPlugin)

bool LPCSensors::checkConfigurationNode(OSObject *node, const char *name)
{
    if (node) {
        if (OSString *configName = OSDynamicCast(OSString, node)) {
            if (!configName->isEqualTo(name))
                return false;
        }
        else if (OSDictionary *configDict = OSDynamicCast(OSDictionary, node)) {
            if ((configName = OSDynamicCast(OSString, configDict->getObject("name")))) {
                if (!configName->isEqualTo(name))
                    return false;
            }
            else return false;
        }
        else return false;
    }
    
    return true;
}

bool LPCSensors::addSensorFromConfigurationNode(OSObject *node, const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index)
{
    float reference = 0, gain = 0, offset = 0;
    
    if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, node))
        FakeSMCSensor::parseModifiers(dictionary, &reference, &gain, &offset);
    
    if (!this->addSensor(key, type, size, group, index, reference, gain, offset)) {
        const char *group_name;
        
        switch (group) {
            case kFakeSMCTemperatureSensor:
                group_name = "temperature";
                break;
            case kFakeSMCTachometerSensor:
                group_name = "tachometer";
                break;
            case kFakeSMCVoltageSensor:
                group_name = "voltage";
                break;
            case kFakeSMCFrequencySensor:
                group_name = "frequency";
                break;
            case kFakeSMCMultiplierSensor:
                group_name = "multiplier";
                break;
                
            default:
                group_name = "";
                break;
        }
        
        HWSensorsWarningLog("failed to add %s sensor for key %s", group_name, key);
        
        return false;
    }
    
    return true;
}

bool LPCSensors::addTemperatureSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding temperature sensors...");
        
    for (int i = 0; i < temperatureSensorsLimit(); i++) 
    {				
        char key[8];

        snprintf(key, 8, "TEMPIN%X", i);
        
        if (OSObject* node = configuration->getObject(key)) {
            if (!addSensor(node, kFakeSMCCategoryTemperature, kFakeSMCTemperatureSensor, i)) {
                if (gpuIndex < 0)
                    gpuIndex = takeVacantGPUIndex();
                    
                if (gpuIndex >= 0 && checkConfigurationNode(configuration, "GPU Die")) {
                    snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, gpuIndex);
                    if (!addSensorFromConfigurationNode(node, key, TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, i)) {
                        releaseGPUIndex(gpuIndex);
                        gpuIndex = -1;
                    }
                }
            }
        }
    }

    return true;
}

bool LPCSensors::addVoltageSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding voltage sensors...");
       
    for (int i = 0; i < voltageSensorsLimit(); i++)
    {				
        char key[5];
        
        snprintf(key, 5, "VIN%X", i);
        
        if (OSObject* node = configuration->getObject(key)) {
            if (!addSensor(node, kFakeSMCCategoryVoltage, kFakeSMCVoltageSensor, i)) {
                if (gpuIndex < 0)
                    gpuIndex = takeVacantGPUIndex();
                
                if (gpuIndex >= 0 && checkConfigurationNode(configuration, "GPU Core")) {
                    snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, gpuIndex);
                    if (!addSensorFromConfigurationNode(node, key, TYPE_FP2E, TYPE_FPXX_SIZE, kFakeSMCVoltageSensor, i)) {
                        releaseGPUIndex(gpuIndex);
                        gpuIndex = -1;
                    }
                }
            }
        }
    }

    return true;
}

bool LPCSensors::addTachometerSensors(OSDictionary *configuration)
{
    HWSensorsDebugLog("adding tachometer sensors...");
    
    for (int i = 0; i < tachometerSensorsLimit(); i++) {
        char key[7];
        snprintf(key, 7, "FANIN%X", i);
        
        if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key)))
            if (!addTachometer(i, name->getLength() > 0 ? name->getCStringNoCopy() : 0))
                HWSensorsWarningLog("failed to add tachometer sensor %d", i);
    }
    
    return true;
}

UInt8 LPCSensors::temperatureSensorsLimit()
{
    return 3;
}

UInt8 LPCSensors::voltageSensorsLimit()
{
    return 9;
}

UInt8 LPCSensors::tachometerSensorsLimit()
{
    return 5;
}

float LPCSensors::readTemperature(UInt32 index)
{
	return 0;
}

float LPCSensors::readVoltage(UInt32 index)
{
	return 0;
}

float LPCSensors::readTachometer(UInt32 index)
{
	return 0;
}

float LPCSensors::getSensorValue(FakeSMCSensor *sensor)
{
    float value = 0;
    
    if (sensor) {
        switch (sensor->getGroup()) {
            case kFakeSMCTemperatureSensor:
                value = sensor->getOffset() + readTemperature(sensor->getIndex());
                break;
                
            case kFakeSMCVoltageSensor:
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

bool LPCSensors::initialize()
{
    return true;
}

bool LPCSensors::init(OSDictionary *properties)
{
	if (!super::init(properties))
		return false;
	
    address = 0;
    port = 0;
   	model = 0;
    
    modelName = "unknown";
    vendorName = "unknown";
    
    gpuIndex = -1;
        
	return true;
}

IOService *LPCSensors::probe(IOService *provider, SInt32 *score)
{
    return super::probe(provider, score);
}

bool LPCSensors::start(IOService *provider)
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
        enableExclusiveAccessMode();
        addTemperatureSensors(configuration);
        addVoltageSensors(configuration);
        addTachometerSensors(configuration);
        disableExclusiveAccessMode();
    }
    else HWSensorsWarningLog("no platform profile provided");
    
    OSSafeReleaseNULL(modelString);
    
    registerService();
    
    HWSensorsInfoLog("started");

	return true;
}

void LPCSensors::stop(IOService *provider)
{
    if (gpuIndex >= 0)
        if (!releaseGPUIndex(gpuIndex))
            HWSensorsFatalLog("failed to release GPU index");
    
    super::stop(provider);
}