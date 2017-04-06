/*
 *  FakeSMCKey.cpp
 *  FakeSMC
 *
 *  Created by kozlek on 03/10/10.
 *
 */

//  The MIT License (MIT)
//
//  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
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

#include <IOKit/IOLib.h>

#include "FakeSMCDefinitions.h"
#include "FakeSMCKey.h"
#include "FakeSMCKeyHandler.h"

#include "timer.h"

#define super OSObject
OSDefineMetaClassAndStructors(FakeSMCKey, OSObject)

/**
 Get numeric representation from char containing number

 @param c Char
 @return Numeric value or 0 if char contains letter
 */
UInt8 FakeSMCKey::getIndexFromChar(char c)
{
    return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

/**
 *  Encode floating point value to SMC float format
 *
 *  @param value     Floating point value to be encoded
 *  @param type      Floating point SMC type name ("fp2e", "fpe2", "sp78" are correct float SMC types)
 *  @param size      Buffer size for encoded bytes, for every floating SMC type should be 2 bytes
 *  @param outBuffer Buffer where encoded bytes will be copied to, should be already allocated with correct size
 *
 *  @return True on success False otherwise
 */
bool FakeSMCKey::encodeFloatValue(float value, const char *type, const UInt8 size, void *outBuffer)
{
    if (type && outBuffer) {
        
        size_t typeLength = strnlen(type, 4);
        
        if (typeLength >= 3 && (type[0] == 'f' || type[0] == 's') && type[1] == 'p') {
            bool minus = value < 0;
            bool signd = type[0] == 's';
            UInt8 i = getIndexFromChar(type[2]);
            UInt8 f = getIndexFromChar(type[3]);
            
            if (i + f == (signd ? 15 : 16)) {
                if (minus) value = -value;
                UInt16 encoded = value * (float)BIT(f);
                if (signd) bit_write(minus, encoded, BIT(15));
                OSWriteBigInt16(outBuffer, 0, encoded);
                return true;
            }
        }
    }
    
    return false;
}

/**
 *  Encode integer value to SMC integer format
 *
 *  @param value     Integer value to be encoded
 *  @param type      Integer SMC type ("ui8 ", "ui16", "ui32", "si8 ")
 *  @param size      Buffer size for encoded bytes, one byte for ui8, 2 bytes for ui16 etc.
 *  @param outBuffer Buffer where encoded bytes will be copied to, should be already allocated with correct size
 *
 *  @return True on success False otherwise
 */
bool FakeSMCKey::encodeIntValue(int value, const char *type, const UInt8 size, void *outBuffer)
{
    if (type && outBuffer) {
        
        size_t typeLength = strnlen(type, 4);
        
        if (typeLength >= 3 && (type[0] == 'u' || type[0] == 's') && type[1] == 'i') {
            
            bool minus = value < 0;
            bool signd = type[0] == 's';
            
            if (minus) value = -value;
            
            switch (type[2]) {
                case '8':
                    if (type[3] == '\0' && size == 1) {
                        UInt8 encoded = (UInt8)value;
                        if (signd) bit_write(signd && minus, encoded, BIT(7));
                        bcopy(&encoded, outBuffer, 1);
                        return true;
                    }
                    break;
                    
                case '1':
                    if (type[3] == '6' && size == 2) {
                        UInt16 encoded = (UInt16)value;
                        if (signd) bit_write(signd && minus, encoded, BIT(15));
                        OSWriteBigInt16(outBuffer, 0, encoded);
                        return true;
                    }
                    break;
                    
                case '3':
                    if (type[3] == '2' && size == 4) {
                        UInt32 encoded = (UInt32)value;
                        if (signd) bit_write(signd && minus, encoded, BIT(31));
                        OSWriteBigInt32(outBuffer, 0, encoded);
                        return true;
                    }
                    break;
            }
        }
    }
    return false;
}

/**
 *  Cheks if a type name is correct integer SMC type name
 *
 *  @param type Type name to check
 *
 *  @return True is returned when the type name is correct False otherwise
 */
bool FakeSMCKey::isValidIntegerType(const char *type)
{
    if (type) {
        size_t typeLength = strnlen(type, 4);
        
        if (typeLength >= 3) {
            if ((type[0] == 'u' || type[0] == 's') && type[1] == 'i') {
                
                switch (type[2]) {
                    case '8':
                        return true;
                    case '1':
                        return type[3] == '6' ? true : false;
                    case '3':
                        return type[3] == '2'? true : false;
                }
            }
        }
    }
    
    return false;
}

/**
 *  Cheks if a type name is a correct floating point SMC type name
 *
 *  @param type Type name to check
 *
 *  @return True is returned when the type name is correct False otherwise
 */
bool FakeSMCKey::isValidFloatType(const char *type)
{
    if (type) {
        
        size_t typeLength = strnlen(type, 4);
        
        if (typeLength >= 3) {
            if ((type[0] == 'f' || type[0] == 's') && type[1] == 'p') {
                UInt8 i = getIndexFromChar(type[2]);
                UInt8 f = getIndexFromChar(type[3]);
                
                if (i + f == (type[0] == 's' ? 15 : 16))
                    return true;
            }
        }
    }
    
    return false;
}

/**
 *  Decode buffer considering it's a floating point encoded value of an SMC key
 *
 *  @param type     SMC type name will be used to determine decoding rules
 *  @param size     Encoded value buffer size
 *  @param data     Pointer to a encoded value buffer
 *  @param outValue Decoded float value will be returned
 *
 *  @return True on success False otherwise
 */
bool FakeSMCKey::decodeFloatValue(const char *type, const UInt8 size, const void *data, float *outValue)
{
    if (type && data && outValue) {
        
        size_t typeLength = strnlen(type, 4);
        
        if (typeLength >= 3 && (type[0] == 'f' || type[0] == 's') && type[1] == 'p' && size == 2) {
            UInt16 encoded = 0;
            
            bcopy(data, &encoded, 2);
            
            UInt8 i = getIndexFromChar(type[2]);
            UInt8 f = getIndexFromChar(type[3]);
            
            if (i + f != (type[0] == 's' ? 15 : 16) )
                return false;
            
            UInt16 swapped = OSSwapBigToHostInt16(encoded);
            
            bool signd = type[0] == 's';
            bool minus = bit_get(swapped, BIT(15));
            
            if (signd && minus) bit_clear(swapped, BIT(15));
            
            *outValue = ((float)swapped / (float)BIT(f)) * (signd && minus ? -1 : 1);
            
            return true;
        }
    }
    
    return false;
}

/**
 *  Decode buffer considering it's an integer encoded value of an SMC key
 *
 *  @param type     SMC type name will be used to determine decoding rules
 *  @param size     Size of encoded value buffer
 *  @param data     Pointer to encoded value buffer
 *  @param outValue Decoded integer value will be returned
 *
 *  @return True on success False otherwise
 */
bool FakeSMCKey::decodeIntValue(const char *type, const UInt8 size, const void *data, int *outValue)
{
    if (type && data && outValue) {
        
        size_t typeLength = strnlen(type, 4);
        
        if (typeLength >= 3 && (type[0] == 'u' || type[0] == 's') && type[1] == 'i') {
            
            bool signd = type[0] == 's';
            
            switch (type[2]) {
                case '8':
                    if (size == 1) {
                        UInt8 encoded = 0;
                        
                        bcopy(data, &encoded, 1);
                        
                        if (signd && bit_get(encoded, BIT(7))) {
                            bit_clear(encoded, BIT(7));
                            *outValue = -encoded;
                        }
                        
                        *outValue = encoded;
                        
                        return true;
                    }
                    break;
                    
                case '1':
                    if (type[3] == '6' && size == 2) {
                        UInt16 encoded = 0;
                        
                        bcopy(data, &encoded, 2);
                        
                        encoded = OSSwapBigToHostInt16(encoded);
                        
                        if (signd && bit_get(encoded, BIT(15))) {
                            bit_clear(encoded, BIT(15));
                            *outValue = -encoded;
                        }
                        
                        *outValue = encoded;
                        
                        return true;
                    }
                    break;
                    
                case '3':
                    if (type[3] == '2' && size == 4) {
                        UInt32 encoded = 0;
                        
                        bcopy(data, &encoded, 4);
                        
                        encoded = OSSwapBigToHostInt32(encoded);
                        
                        if (signd && bit_get(encoded, BIT(31))) {
                            bit_clear(encoded, BIT(31));
                            *outValue = -encoded;
                        }
                        
                        *outValue = encoded;
                        
                        return true;
                    }
                    break;
            }
        }
    }
    
    return false;
}

FakeSMCKey *FakeSMCKey::withValue(const char *aKey, const char *aType, unsigned char aSize, const void *aValue)
{
    FakeSMCKey *me = new FakeSMCKey;
	
    if (me && !me->init(aKey, aType, aSize, aValue))
        OSSafeReleaseNULL(me);
	
    return me;
}

FakeSMCKey *FakeSMCKey::withHandler(const char *aKey, const char *aType, const unsigned char aSize, FakeSMCKeyHandler *aHandler)
{
    FakeSMCKey *me = new FakeSMCKey;
	
    if (me && !me->init(aKey, aType, aSize, 0, aHandler))
        OSSafeReleaseNULL(me);
	
    return me;
}

bool FakeSMCKey::init(const char * aKey, const char * aType, const unsigned char aSize, const void *aValue, FakeSMCKeyHandler *aHandler)
{
    if (!super::init())
        return false;

	if (!aKey || strnlen(aKey, 4) == 0 || !(key = (char *)IOMalloc(5)))
		return false;
	
	copySymbol(aKey, key);
    
	size = aSize > 32 ? 32 : aSize;
	
	if (!(type = (char *)IOMalloc(5)))
		return false;
    
	if (!aType || strnlen(aType, 4) == 0) {
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
	}
	else copySymbol(aType, type);
	
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

const UInt8 FakeSMCKey::getSize() const { return size; };

const void *FakeSMCKey::getValue() 
{ 
	if (handler) {
        
        double time = ptimer_read_seconds();

        // Allows update value twice in a second
        if (time - lastValueReadTime >= 0.5) {
            
            IOReturn result = handler->readKeyCallback(key, type, size, value);
            
            if (kIOReturnSuccess == result) {
                lastValueReadTime = time;
            }
            else {
                HWSensorsWarningLog("value update request callback returned error for key %s (%s)", key, handler->stringFromReturn(result));
            }
        }
	}
    
	return value; 
};

FakeSMCKeyHandler *FakeSMCKey::getHandler() { return handler; };

bool FakeSMCKey::setType(const char *aType)
{
    if (aType) {
        copySymbol(aType, type);
        return true;
    }
    
    return false;
}

bool FakeSMCKey::setSize(UInt8 aSize)
{
    size = aSize;
    
    return true;
}

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
        
        /*double time = ptimer_read_seconds();
        
        if (time - lastValueWrote >= 0.5) {
            
            IOReturn result = handler->writeKeyCallback(key, type, size, value);
            
            if (kIOReturnSuccess == result) {
                lastValueWrote = time;
            }
            else {
                HWSensorsWarningLog("value changed event callback returned error for key %s (%s)", key, handler->stringFromReturn(result));
            }
        }*/

        IOReturn result = handler->writeKeyCallback(key, type, size, value);

        if (kIOReturnSuccess != result) {
            HWSensorsWarningLog("value changed event callback returned error for key %s (%s)", key, handler->stringFromReturn(result));
        }
    }
	
	return true;
}

bool FakeSMCKey::setHandler(FakeSMCKeyHandler *newHandler)
{
    if (handler && newHandler) {
        if (newHandler->getProbeScore() < handler->getProbeScore()) {
            HWSensorsErrorLog("key %s already handled with prioritized handler %s", key, handler->getName());
            return false;
        }
        else {
            handler = newHandler;

            HWSensorsInfoLog("key %s handler %s has been replaced with new prioritized handler %s", key, handler->getName(), newHandler->getName());
        }
    }

	return false;
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
