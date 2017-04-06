//
//  FakeSMCSensor.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 05/04/2017.
//
//

#include "FakeSMCSensor.h"
#include "FakeSMCDefinitions.h"
#include "FakeSMCKey.h"

#pragma mark FakeSMCPSensor

OSDefineMetaClassAndStructors(FakeSMCSensor, OSObject)

/**
 *  Parse modifiers from configuration node
 *
 *  @param node      Configuration node to parse modifiers from
 *  @param reference Parsed 'reference' modifier will be returned
 *  @param gain      Parsed 'gain' modifier will be returned
 *  @param offset    Parsed 'offset' modifier will be returned
 *
 *  @return True on success False otherwise
 */
bool FakeSMCSensor::parseModifiers(OSDictionary *node, float *reference, float *gain, float *offset)
{
    if (OSDynamicCast(OSDictionary, node)) {
        
        if (OSNumber *number = OSDynamicCast(OSNumber, node->getObject("reference")))
            if (reference)
                *reference = (float)number->unsigned64BitValue() / 1000.0f;
        
        if (OSNumber *number = OSDynamicCast(OSNumber, node->getObject("gain")))
            if (gain)
                *gain = (float)number->unsigned64BitValue() / 1000.0f;
        
        if (OSNumber *number = OSDynamicCast(OSNumber, node->getObject("offset")))
            if (offset)
                *offset = (float)number->unsigned64BitValue() / 1000.0f;
        
        return true;
    }
    
    return false;
}

/**
 *  Create new FakeSMCSensor object
 *
 *  @param aOwner     Handler of a sensor
 *  @param aKey       SMC key name
 *  @param aType      SMC type name
 *  @param aSize      SMC value size
 *  @param aGroup     <#aGroup description#>
 *  @param aIndex     <#aIndex description#>
 *  @param aReference <#aReference description#>
 *  @param aGain      <#aGain description#>
 *  @param aOffset    <#aOffset description#>
 *
 *  @return <#return value description#>
 */
FakeSMCSensor *FakeSMCSensor::withOwner(FakeSMCPlugin *aOwner, const char *aKey, const char *aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference, float aGain, float aOffset)
{
    FakeSMCSensor *me = new FakeSMCSensor;
    
    if (me && !me->initWithOwner(aOwner, aKey, aType, aSize, aGroup, aIndex, aReference, aGain, aOffset))
        OSSafeReleaseNULL(me);
    
    return me;
}


/**
 *  For internal use
 */
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

void FakeSMCSensor::encodeNumericValue(float value, void *outBuffer)
{
    if (!FakeSMCKey::encodeFloatValue(value, type, size, outBuffer)) {
        FakeSMCKey::encodeIntValue(value, type, size, outBuffer);
    }
}
