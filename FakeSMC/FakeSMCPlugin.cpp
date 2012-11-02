//
//  FakeSMCPlugin.cpp
//  HWSensors
//
//  Created by kozlek on 11/02/12.
//
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


#include "FakeSMCPlugin.h"
#include "FakeSMCDefinitions.h"

#include <IOKit/IOLib.h>

// Sensor

OSDefineMetaClassAndStructors(FakeSMCSensor, OSObject)

FakeSMCSensor *FakeSMCSensor::withOwner(FakeSMCPlugin *aOwner, const char *aKey, const char *aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference, float aGain, float aOffset)
{
	FakeSMCSensor *me = new FakeSMCSensor;
	
    if (me && !me->initWithOwner(aOwner, aKey, aType, aSize, aGroup, aIndex, aReference, aGain, aOffset))
        OSSafeRelease(me);
	
    return me;
}

bool FakeSMCSensor::initWithOwner(FakeSMCPlugin *aOwner, const char *aKey, const char *aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference, float aGain, float aOffset)
{
	if (!OSObject::init())
		return false;
	
	if (!(owner = aOwner))
		return false;
    
	bzero(key, 5);
    bcopy(aKey, key, 4);
    
    bzero(type, 5);
	bcopy(aType, type, 4);
	
	size = aSize;
	group = aGroup;
	index = aIndex;
    
    reference = aReference;
    gain = aGain;
    offset = aOffset;
    
	return true;
}

const char *FakeSMCSensor::getKey()
{
	return key;
}

const char *FakeSMCSensor::getType()
{
	return type;
}

UInt8 FakeSMCSensor::getSize()
{
	return size;
}

UInt32 FakeSMCSensor::getGroup()
{
	return group;
}

UInt32 FakeSMCSensor::getIndex()
{
	return index;
}

float FakeSMCSensor::getReference()
{
    return reference;
}

float FakeSMCSensor::getGain()
{
    return gain;
}

float FakeSMCSensor::getOffset()
{
    return offset;
}

inline UInt8 get_index(char c)
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

void FakeSMCSensor::encodeValue(float value, void *outBuffer)
{
    if ((type[0] == 'u' || type[0] == 's') && type[1] == 'i') {
        
        bool minus = value < 0;
        
        if (type[0] == 'u' && minus) {
            value = -value;
            minus = false;
        }
        
        switch (type[2]) {
            case '8':
                if (type[3] == '\0' && size == 1) {
                    UInt8 out = minus ? (UInt8)(-value) | 0x80 : (UInt8)value;
                    bcopy(&out, outBuffer, 1);
                }
                break;
                
            case '1':
                if (type[3] == '6' && size == 2) {
                    UInt16 out = OSSwapHostToBigInt16(minus ? (UInt16)(-value) | 0x8000 : (UInt16)value);
                    bcopy(&out, outBuffer, 2);
                }
                break;
                
            case '3':
                if (type[3] == '2' && size == 4) {
                    UInt32 out = OSSwapHostToBigInt32(minus ? (UInt32)(-value) | 0x80000000 : (UInt32)value);
                    bcopy(&out, outBuffer, 4);
                }
                break;
                
            default:
                return;
        }
    }
    else if ((type[0] == 'f' || type[0] == 's') && type[1] == 'p') {
        
        bool minus = value < 0;
        UInt8 i = get_index(type[2]);
        UInt8 f = get_index(type[3]);
        
        if (i + f == (type[0] == 'f' ? 16 : 15)) {
            
            UInt64 mult = (minus ? -value : value) * 1000 ;
            UInt64 encoded = ((mult << f) / 1000) & 0xffff;
            
            UInt16 out = OSSwapHostToBigInt16(minus ? (UInt16)(encoded | 0x8000) : (UInt16)encoded);
            
            bcopy(&out, outBuffer, 2);
        }
    }
}

// Plugin

#include "OEMInfo.h"

#define super IOService
OSDefineMetaClassAndAbstractStructors(FakeSMCPlugin, IOService)


OSString *FakeSMCPlugin::getPlatformManufacturer()
{
    if (headingProvider)
        return OSDynamicCast(OSString, headingProvider->getProperty(kOEMInfoManufacturer));
    
    return NULL;
}

OSString *FakeSMCPlugin::getPlatformProduct()
{
    if (headingProvider)
        return OSDynamicCast(OSString, headingProvider->getProperty(kOEMInfoProduct));
    
    return NULL;
}

bool FakeSMCPlugin::isKeyHandled(const char *key)
{
    if (storageProvider) {
        IOService *handler = 0;
        return kIOReturnSuccess == storageProvider->callPlatformFunction(kFakeSMCGetKeyHandler, false, (void *)key, (void *)&handler, 0, 0);
    }
    
    return false;
}

FakeSMCSensor *FakeSMCPlugin::addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset)
{   
    if (getSensor(key)) {
        HWSensorsDebugLog("will not add handler for key %s, key already handled", key);
		return NULL;
    }
	
    if (FakeSMCSensor *sensor = FakeSMCSensor::withOwner(this, key, type, size, group, index, reference, gain, offset)) {
        if (addSensor(sensor))
            return sensor;
        else 
            OSSafeRelease(sensor);
    }
	
	return NULL;
}

bool FakeSMCPlugin::addSensor(FakeSMCSensor *sensor)
{
    if(sensor && kIOReturnSuccess == storageProvider->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)sensor->getKey(), (void *)sensor->getType(), (void *)sensor->getSize(), (void *)this))
        return sensors->setObject(sensor->getKey(), sensor);
    
    return false;
}

FakeSMCSensor *FakeSMCPlugin::addTachometer(UInt32 index, const char* name, UInt8 *fanIndex)
{
    UInt8 length = 0;
	void * data = 0;
    
	if (kIOReturnSuccess == storageProvider->callPlatformFunction(kFakeSMCGetKeyValue, false, (void *)KEY_FAN_NUMBER, (void *)&length, (void *)&data, 0)) {
		length = 0;
		
		bcopy(data, &length, 1);
		
        for (int i = 0; i <= 0xf; i++) {
            char key[5];
            
            snprintf(key, 5, KEY_FORMAT_FAN_SPEED, i); 
            
            if (!isKeyHandled(key)) {
                if (FakeSMCSensor *sensor = addSensor(key, TYPE_FPE2, 2, kFakeSMCTachometerSensor, index)) {
                    if (name) {
                        snprintf(key, 5, KEY_FORMAT_FAN_ID, i); 
                        
                        if (kIOReturnSuccess != storageProvider->callPlatformFunction(kFakeSMCAddKeyValue, false, (void *)key, (void *)TYPE_CH8, (void *)((UInt64)strlen(name)), (void *)name))
                            HWSensorsWarningLog("failed to add tachometer name for key %s", key);
                    }
                    
                    if (i + 1 > length) {
                        length++;
                        
                        if (kIOReturnSuccess != storageProvider->callPlatformFunction(kFakeSMCSetKeyValue, false, (void *)KEY_FAN_NUMBER, (void *)(UInt8)1, (void *)&length, 0))
                            HWSensorsWarningLog("failed to update FNum value");
                    }
                    
                    if (fanIndex) *fanIndex = (UInt8)i;
                    
                    return sensor;
                }
                else HWSensorsWarningLog("failed to add tachometer sensor for key %s", key);
            }
        }
	}
	else HWSensorsWarningLog("failed to read FNum value");
	
	return 0;
}

FakeSMCSensor *FakeSMCPlugin::getSensor(const char* key)
{
	return OSDynamicCast(FakeSMCSensor, sensors->getObject(key));
}

float FakeSMCPlugin::getSensorValue(FakeSMCSensor *sensor)
{
    return 0;
}

SInt8 FakeSMCPlugin::getVacantGPUIndex()
{
    //Find card number
    char key[5];
    for (UInt8 i = 0; i <= 0xf; i++) {
        
        snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, i); 
        if (isKeyHandled(key)) continue;
            
        snprintf(key, 5, KEY_FORMAT_GPU_HEATSINK_TEMPERATURE, i);             
        if (isKeyHandled(key)) continue;
            
        snprintf(key, 5, KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE, i);             
        if (isKeyHandled(key)) continue;

        snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, i); 
        if (isKeyHandled(key)) continue;
                    
        snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_FREQUENCY, i); 
        if (isKeyHandled(key)) continue;
                    
        return i;
    }
    
    return false;
}

OSDictionary *FakeSMCPlugin::getConfigurationNode(OSDictionary *root, OSString *name)
{
    OSDictionary *configuration = NULL;
    
    if (root && name) {
        HWSensorsDebugLog("looking up for configuration node: %s", name->getCStringNoCopy());
        
        if (!(configuration = OSDynamicCast(OSDictionary, root->getObject(name))))
            if (OSString *link = OSDynamicCast(OSString, root->getObject(name)))
                configuration = getConfigurationNode(root, link);
    }
    
    return configuration;
}

OSDictionary *FakeSMCPlugin::getConfigurationNode(OSDictionary *root, const char *name)
{
    OSDictionary *configuration = NULL;
    
    if (root && name) {
        OSString *nameNode = OSString::withCStringNoCopy(name);
        
        configuration = getConfigurationNode(root, nameNode);
        
        OSSafeReleaseNULL(nameNode);
    }
    
    return configuration;
}

OSDictionary *FakeSMCPlugin::getConfigurationNode(OSString *model)
{
    OSDictionary *configuration = NULL;
    
    if (OSDictionary *list = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration")))
    {
        if (OSString *manufacturer = getPlatformManufacturer())
            if (OSDictionary *manufacturerNode = OSDynamicCast(OSDictionary, list->getObject(manufacturer)))
                if (!(configuration = getConfigurationNode(manufacturerNode, getPlatformProduct())))
                    if (!(configuration = getConfigurationNode(manufacturerNode, model)))
                        configuration = getConfigurationNode(manufacturerNode, "Default");
        
        if (!configuration && !(configuration = getConfigurationNode(list, model)))
            configuration = getConfigurationNode(list, "Default");
    }

    return configuration;
}

bool FakeSMCPlugin::init(OSDictionary *properties)
{
    if (!super::init(properties))
        return false;
    
    sensors = OSDictionary::withCapacity(0);
    
    if (!sensors)
        return false;
    
	return true;
}

IOService * FakeSMCPlugin::probe(IOService *provider, SInt32 *score)
{	
	if (super::probe(provider, score) != this) 
		return 0;
    
    return this;
}

bool FakeSMCPlugin::start(IOService *provider)
{	
	if (!super::start(provider)) 
        return false;

    if (!(headingProvider = waitForService(serviceMatching(kFakeSMCService))))
		HWSensorsWarningLog("can't locate FakeSMC service, specific OEM configuration will be unavailable");
    
	if (!(storageProvider = waitForService(serviceMatching(kFakeSMCDeviceService)))) {
		HWSensorsFatalLog("can't locate FakeSMCDevice");
		return false;
	}
	
	return true;
}

void FakeSMCPlugin::stop(IOService* provider)
{
    storageProvider->callPlatformFunction(kFakeSMCRemoveKeyHandler, false, this, NULL, NULL, NULL);
    
    sensors->flushCollection();
	
	super::stop(provider);
}

void FakeSMCPlugin::free()
{
    OSSafeRelease(sensors);	
	super::free();
}

IOReturn FakeSMCPlugin::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
    if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char *name = (const char*)param1;
		void *data = param2;
		UInt8 size = (UInt64)param3;
		
		if (name && data)
			if (FakeSMCSensor *sensor = getSensor(name))
                if (size == sensor->getSize()) {
                    sensor->encodeValue(getSensorValue(sensor), data);
                    return kIOReturnSuccess;
                }
		
		return kIOReturnBadArgument;
	}
    
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}