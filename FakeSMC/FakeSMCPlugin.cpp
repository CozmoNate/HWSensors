//
//  FakeSMCPlugin.cpp
//  HWSensors
//
//  Created by mozo on 11/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#include "FakeSMCPlugin.h"

#include <IOKit/IOLib.h>
#include "FakeSMCDefinitions.h"
#include "FakeSMCValueEncoder.h"

#define Debug FALSE

#define LogPrefix "FakeSMCPlugin: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

// Sensor

OSDefineMetaClassAndStructors(FakeSMCSensor, OSObject)

FakeSMCSensor *FakeSMCSensor::withOwner(FakeSMCPlugin *aOwner, const char *aKey, const char *aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex)
{
	FakeSMCSensor *me = new FakeSMCSensor;
	
    if (me && !me->initWithOwner(aOwner, aKey, aType, aSize, aGroup, aIndex)) {
        me->release();
        return 0;
    }
	
    return me;
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

bool FakeSMCSensor::initWithOwner(FakeSMCPlugin *aOwner, const char *aKey, const char *aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex)
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
	
	return true;
}

void FakeSMCSensor::encodeValue(float value, void *outBuffer)
{
    if (type[0] == 'u' && type[1] == 'i') {
        switch (type[3]) {
            case '8':
                if (type[4] == '\0' && size == 1) {
                    UInt8 out = (UInt8)value;
                    bcopy(&out, outBuffer, 1);
                }
                break;
                
            case '1':
                if (type[4] == '6' && size == 2) {
                    UInt16 out = OSSwapHostToBigInt16((UInt16)value);
                    bcopy(&out, outBuffer, 2);
                }
                break;
                
            case '3':
                if (type[4] == '2' && size == 4) {
                    UInt32 out = OSSwapHostToBigInt32((UInt32)value);
                    bcopy(&out, outBuffer, 4);
                }
                break;
        }
    }
    else if (type[0] == 's' && type[1] == 'i') {
        switch (type[3]) {
            case '8':
                if (type[4] == '\0' && size == 1) {
                    SInt8 out = (SInt8)value;
                    bcopy(&out, outBuffer, 1);
                }
                break;
                
            case '1':
                if (type[4] == '6' && size == 2) {
                    SInt16 out = OSSwapHostToBigInt16((SInt16)value);
                    bcopy(&out, outBuffer, 2);
                }
                break;
                
            case '3':
                if (type[4] == '2' && size == 4) {
                    SInt32 out = OSSwapHostToBigInt32((SInt32)value);
                    bcopy(&out, outBuffer, 4);
                }
                break;
        }
    }
    else if ((type[0] == 'f' || type[0] == 's') && type[1] == 'p') {
        UInt16 out = encode_16bit_fractional(type, value);
        bcopy(&out, outBuffer, 2);
    }
}

// Plug-In

#define super IOService
OSDefineMetaClassAndAbstractStructors(FakeSMCPlugin, IOService)

bool FakeSMCPlugin::isKeyHandled(const char *key)
{
    if (fakeSMC) {
        IOService *handler = 0;
        return kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyHandler, true, (void *)key, (void *)&handler, 0, 0);
    }
    
    return false;
}

FakeSMCSensor *FakeSMCPlugin::addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index)
{   
    if (getSensor(key)) {
        DebugLog("will not add sensor for key %s, key handler already assigned", key);
		return NULL;
    }
	
    if (FakeSMCSensor *sensor = FakeSMCSensor::withOwner(this, key, type, size, group, index)) {
        if (addSensor(sensor))
           return sensor;
        else 
            sensor->release();
    }
	
	return NULL;
}

bool FakeSMCPlugin::addSensor(FakeSMCSensor *sensor)
{
    if(sensor && kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)sensor->getKey(), (void *)sensor->getType(), (void *)sensor->getSize(), (void *)this))
        return sensors->setObject(sensor->getKey(), sensor);
    
    return false;
}

FakeSMCSensor *FakeSMCPlugin::addTachometer(UInt32 index, const char* name)
{
    UInt8 length = 0;
	void * data = 0;
    
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)KEY_FAN_NUMBER, (void *)&length, (void *)&data, 0)) {
		length = 0;
		
		bcopy(data, &length, 1);
		
        for (int i = 0; i <= 0xf; i++) {
            char key[5];
            
            snprintf(key, 5, KEY_FORMAT_FAN_SPEED, i); 
            
            if (!isKeyHandled(key)) {
                if (FakeSMCSensor *sensor = addSensor(key, TYPE_FPE2, 2, kFakeSMCTachometerSensor, index)) {
                    if (name) {
                        snprintf(key, 5, KEY_FORMAT_FAN_ID, i); 
                        
                        if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyValue, false, (void *)key, (void *)TYPE_CH8, (void *)((UInt64)strlen(name)), (void *)name))
                            WarningLog("can't add tachometer name for key %s", key);
                    }
                    
                    length++;
                    
                    if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void *)KEY_FAN_NUMBER, (void *)1, (void *)&length, 0))
                        WarningLog("can't update FNum value");
                    
                    return sensor;
                }
                else WarningLog("can't add tachometer sensor for key %s", key);
            }
        }
	}
	else WarningLog("can't read FNum value");
	
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

bool FakeSMCPlugin::init(OSDictionary *properties)
{
	DebugLog("initialising...");
    
    if (!super::init(properties))
        return false;
	
    isActive = false;
    
    sensors = OSDictionary::withCapacity(0);
    
    if (!sensors)
        return false;
    
	return true;
}

IOService * FakeSMCPlugin::probe(IOService *provider, SInt32 *score)
{
	DebugLog("probing...");
	
	if (super::probe(provider, score) != this) 
		return 0;
    
    return this;
}

bool FakeSMCPlugin::start(IOService *provider)
{		
	DebugLog("starting...");
	
	if (!super::start(provider)) 
        return false;
    
    if (!isActive) return true;
    
	if (!(fakeSMC = waitForService(serviceMatching(kFakeSMCDeviceService)))) {
		WarningLog("can't locate FakeSMCDevice");
		return false;
	}
	
	return true;
}

void FakeSMCPlugin::stop(IOService* provider)
{
	DebugLog("stoping...");
    
    fakeSMC->callPlatformFunction(kFakeSMCRemoveHandler, true, this, NULL, NULL, NULL);
    
    sensors->flushCollection();
	
	super::stop(provider);
}

void FakeSMCPlugin::free()
{
	DebugLog("freeing...");
    
    sensors->release();
	
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