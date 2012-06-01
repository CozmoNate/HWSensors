/*
 *  FakeSMCKey.cpp
 *  FakeSMC
 *
 *  Created by mozo on 03/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "FakeSMCKey.h"
#include "FakeSMCDefinitions.h"

#include <IOKit/IOLib.h>

#define super OSObject
OSDefineMetaClassAndStructors(FakeSMCKey, OSObject)

FakeSMCKey *FakeSMCKey::withValue(const char *aKey, const char *aType, unsigned char aSize, const void *aValue)
{
    FakeSMCKey *me = new FakeSMCKey;
	
    if (me && !me->init(aKey, aType, aSize, aValue)) {
        me->release();
        return 0;
    }
	
    return me;
}

FakeSMCKey *FakeSMCKey::withHandler(const char *aKey, const char *aType, unsigned char aSize, IOService *aHandler)
{
    FakeSMCKey *me = new FakeSMCKey;
	
    if (me && !me->init(aKey, aType, aSize, 0, aHandler)) {
        me->release();
        return 0;
    }
	
    return me;
}

bool FakeSMCKey::init(const char * aKey, const char * aType, unsigned char aSize, const void *aValue, IOService * aHandler)
{
    if (!super::init())
        return false;

	if (!aKey || strlen(aKey) == 0 || !(key = (char *)IOMalloc(5))) 
		return false;
	
	copySymbol(aKey, key);
    
	size = aSize;
	
	if (!(type = (char *)IOMalloc(5)))
		return false;
    
	if (!aType || strlen(aType) == 0) {
		switch (size) 
		{
			case 1:
				copySymbol("ui8", type);
				break;
			case 2:
				copySymbol("ui16", type);
				break;
			case 4:
				copySymbol("ui32", type);
				break;
			default:
				copySymbol("ch8*", type);
				break;
		}
        //copySymbol("\0\0\0\0", type);
	}
	else 
        copySymbol(aType, type);
	
	if (size == 0)
		size++;
    
	if (!(value = IOMalloc(size)))
		return false;
	
	if (aValue)
		bcopy(aValue, value, size);
	else
		bzero(value, size);
	
	handler = aHandler;
	
    return true;
}

void FakeSMCKey::free() 
{
	if (key)
		IOFree(key, 5);
	
	if (type)
		IOFree(type, 5);
	
	if (value)
		IOFree(value, size);
	
	super::free(); 
}

const char *FakeSMCKey::getName() { return "FakeSMCKey"; }; // this is used by logging functions

const char *FakeSMCKey::getKey() { return key; };

const char *FakeSMCKey::getType() { return type; };

UInt8 FakeSMCKey::getSize() const { return size; };

const void *FakeSMCKey::getValue() 
{ 
	if (handler) {
        mach_timespec_t now, end;

        end.tv_sec = lastUpdated.tv_sec;
        end.tv_nsec = lastUpdated.tv_nsec;
        now.tv_sec = 1;
        now.tv_nsec = 0;
        
        ADD_MACH_TIMESPEC(&end, &now);
        
        clock_get_system_nanotime((clock_sec_t*)&now.tv_sec, (clock_nsec_t*)&now.tv_nsec);
        
        if (CMP_MACH_TIMESPEC(&end, &now) < 0) {            
            IOReturn result = handler->callPlatformFunction(kFakeSMCGetValueCallback, false, (void *)key, (void *)value, (void *)size, 0);
            
            if (kIOReturnSuccess == result)
                clock_get_system_nanotime((clock_sec_t*)&lastUpdated.tv_sec, (clock_nsec_t*)&lastUpdated.tv_nsec);
            else 
                HWSensorsWarningLog("value update request callback error for key %s, return 0x%x", key, result);
        }
	}
    
	return value; 
};

const IOService *FakeSMCKey::getHandler() { return handler; };

bool FakeSMCKey::setValueFromBuffer(const void *aBuffer, UInt8 aSize)
{
	if (!aBuffer || aSize == 0) 
		return false;
	
	if (aSize != size) {
		if (value)
			IOFree(value, size);
		
		size = aSize;
		
		if (!(value = IOMalloc(size)))
			return false;
	}
	
	bcopy(aBuffer, value, size);
	
	if (handler) {       
		IOReturn result = handler->callPlatformFunction(kFakeSMCSetValueCallback, false, (void *)key, (void *)value, (void *)size, 0);
		
		if (kIOReturnSuccess != result)
			HWSensorsWarningLog("value changed event callback error for key %s, return 0x%x", key, result);
	}
	
	return true;
}

bool FakeSMCKey::setHandler(IOService *aHandler)
{
	if (!aHandler) 
		return false;
	
	handler = aHandler;
	
	return true;
}

bool FakeSMCKey::isEqualTo(const char *aKey)
{
	return strncmp(key, aKey, 4) == 0;
}

bool FakeSMCKey::isEqualTo(FakeSMCKey *aKey)
{
	return (aKey && aKey->isEqualTo(key));
}

bool FakeSMCKey::isEqualTo(const OSMetaClassBase *anObject)
{
    if (FakeSMCKey *aKey = OSDynamicCast(FakeSMCKey, anObject))
        return isEqualTo(aKey);
    
    return false;
}