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
#include "FakeSMCKey.h"
#include "FakeSMCKeyStore.h"

#include <IOKit/IOLib.h>

#pragma mark FakeSMCPSensor

OSDefineMetaClassAndStructors(FakeSMCSensor, OSObject)

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

void FakeSMCSensor::encodeNumericValue(float value, void *outBuffer)
{
    if ((type[0] == 'u' || type[0] == 's') && type[1] == 'i') {
        
        bool minus = value < 0;
        bool signd = type[0] == 's';
        
        if (minus) value = -value;
        
        switch (type[2]) {
            case '8':
                if (type[3] == '\0' && size == 1) {
                    UInt8 encoded = (UInt8)value;
                    if (signd) bit_write(signd && minus, encoded, BIT(7));
                    bcopy(&encoded, outBuffer, 1);
                }
                break;
                
            case '1':
                if (type[3] == '6' && size == 2) {
                    UInt16 encoded = (UInt16)value;
                    if (signd) bit_write(signd && minus, encoded, BIT(15));
                    OSWriteBigInt16(outBuffer, 0, encoded);
                }
                break;
                
            case '3':
                if (type[3] == '2' && size == 4) {
                    UInt32 encoded = (UInt32)value;
                    if (signd) bit_write(signd && minus, encoded, BIT(31));
                    OSWriteBigInt32(outBuffer, 0, encoded);
                }
                break;
                
            default:
                return;
        }
    }
    else if ((type[0] == 'f' || type[0] == 's') && type[1] == 'p') {
        bool minus = value < 0;
        bool signd = type[0] == 's';
        UInt8 i = get_index(type[2]);
        UInt8 f = get_index(type[3]);

        if (i + f == (signd ? 15 : 16)) {
            if (minus) value = -value;
            UInt16 encoded = value * (float)BIT(f);
            if (signd) bit_write(minus, encoded, BIT(15));
            OSWriteBigInt16(outBuffer, 0, encoded);
        }
    }
}

#pragma mark
#pragma mark FakeSMCPlugin

#include "OEMInfo.h"

static IORecursiveLock *gPluginSyncLock = 0;

#define SYNCLOCK        if (!gPluginSyncLock) gPluginSyncLock = IORecursiveLockAlloc(); IORecursiveLockLock(gPluginSyncLock)
#define SYNCUNLOCK      IORecursiveLockUnlock(gPluginSyncLock)

#define super FakeSMCKeyHandler
OSDefineMetaClassAndAbstractStructors(FakeSMCPlugin, FakeSMCKeyHandler)

#pragma mark -
#pragma mark FakeSMCPlugin::methods

OSString *FakeSMCPlugin::getPlatformManufacturer(void)
{
    return OSDynamicCast(OSString, keyStore->getProperty(kOEMInfoManufacturer));
}

OSString *FakeSMCPlugin::getPlatformProduct(void)
{
    return OSDynamicCast(OSString, keyStore->getProperty(kOEMInfoProduct));
}

void FakeSMCPlugin::enableExclusiveAccessMode(void)
{
    SYNCLOCK;
}

void FakeSMCPlugin::disableExclusiveAccessMode(void)
{
    SYNCUNLOCK;
}

bool FakeSMCPlugin::isKeyExists(const char *key)
{
    SYNCLOCK;

    bool keyExists = keyStore->getKey(key);

    SYNCUNLOCK;
    
    return keyExists;
}

bool FakeSMCPlugin::isKeyHandled(const char *key)
{
    SYNCLOCK;

    bool keyHandled = false;

    if (FakeSMCKey *smcKey = keyStore->getKey(key))
        keyHandled = smcKey->getHandler();

    SYNCUNLOCK;

    return keyHandled;
}

bool FakeSMCPlugin::setKeyValue(const char *key, const char *type, UInt8 size, void *value)
{
    SYNCLOCK;

    bool added = keyStore->addKeyWithValue(key, type, size, value);

    SYNCUNLOCK;

    return added;
}

FakeSMCSensor *FakeSMCPlugin::addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    SYNCLOCK;

    if (FakeSMCSensor *sensor = FakeSMCSensor::withOwner(this, key, type, size, group, index, reference, gain, offset)) {
        if (addSensor(sensor)) {
            SYNCUNLOCK;
            return sensor;
        }
        else OSSafeRelease(sensor);
    }
	
    SYNCUNLOCK;
    
	return NULL;
}

FakeSMCSensor *FakeSMCPlugin::addSensor(const char *abbriviation, kFakeSMCCategory category, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    SYNCLOCK;
    
    FakeSMCSensor *sensor = NULL;
    
    if (abbriviation && strlen(abbriviation) >= 3) {
        for (int i = 0; FakeSMCSensorDefinitions[i].name; i++) {

            FakeSMCSensorDefinitionEntry entry = FakeSMCSensorDefinitions[i];

            if (entry.category == category && 0 == strcasecmp(entry.name, abbriviation)) {
                if (entry.count) {
                    for (int counter = 0; counter < entry.count; counter++) {

                        char key[5];
                        snprintf(key, 5, entry.key, entry.shift + counter);
                        
                        if (!isKeyExists(key)) {
                            sensor = addSensor(key, entry.type, entry.size, group, index, reference, gain, offset);
                            break;
                        }
                    }
                }
                else {
                    sensor = addSensor(entry.key, entry.type, entry.size, group, index, reference, gain, offset);
                }
            }
        }
    }
    
    SYNCUNLOCK;

    return sensor;
}

FakeSMCSensor *FakeSMCPlugin::addSensor(OSObject *node, kFakeSMCCategory category, UInt32 group, UInt32 index)
{
    SYNCLOCK;
    
    FakeSMCSensor *sensor = NULL;
    
    if (node) {
 
        float reference = 0, gain = 0, offset = 0;
        OSString *abbriviation = NULL;
        
        if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, node)) {
            if ((abbriviation = OSDynamicCast(OSString, dictionary->getObject("name")))) {
                FakeSMCSensor::parseModifiers(dictionary, &reference, &gain, &offset);
            }
        }
        else abbriviation = OSDynamicCast(OSString, node);

        if (abbriviation)
            sensor = addSensor(abbriviation->getCStringNoCopy(), category, group, index, reference, gain, offset);
    }
    
    SYNCUNLOCK;
    return sensor;
}

bool FakeSMCPlugin::addSensor(FakeSMCSensor *sensor)
{
    SYNCLOCK;
    
    bool added = keyStore->addKeyWithHandler(sensor->getKey(), sensor->getType(), sensor->getSize(), this);

    if (added) {
        sensors->setObject(sensor->getKey(), sensor);
    }

    SYNCUNLOCK;
    
    return added;
}

FakeSMCSensor *FakeSMCPlugin::addTachometer(UInt32 index, const char *name, FanType type, UInt8 zone, FanLocationType location, SInt8 *fanIndex)
{
    SYNCLOCK;
    
    SInt8 vacantFanIndex = keyStore->takeVacantFanIndex();
    
    if (vacantFanIndex >= 0) {
        char key[5];
        snprintf(key, 5, KEY_FORMAT_FAN_SPEED, vacantFanIndex);
        
        if (FakeSMCSensor *sensor = addSensor(key, TYPE_FPE2, TYPE_FPXX_SIZE, kFakeSMCTachometerSensor, index)) {
            FanTypeDescStruct fds;
            
            bzero(&fds, sizeof(fds));
            
            fds.type = type;
            fds.ui8Zone = zone;
            fds.location = location;
            
            if (name)
                strlcpy(fds.strFunction, name, DIAG_FUNCTION_STR_LEN);
            else
                snprintf(fds.strFunction, DIAG_FUNCTION_STR_LEN, "MB Fan %X", index);
            
            snprintf(key, 5, KEY_FORMAT_FAN_ID, vacantFanIndex);
            
            if (!setKeyValue(key, TYPE_FDS, sizeof(fds), &fds))
                HWSensorsWarningLog("failed to add tachometer name for key %s", key);
            
            if (fanIndex) *fanIndex = vacantFanIndex;
            
            SYNCUNLOCK;
            
            return sensor;
        }
        else HWSensorsErrorLog("failed to add tachometer sensor for key %s", key);
    }
    else HWSensorsErrorLog("failed to take vacant Fan index");
	
    SYNCUNLOCK;
    
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

void FakeSMCPlugin::setSensorValue(FakeSMCSensor *sensor, void *data)
{
    //
}

SInt8 FakeSMCPlugin::takeVacantGPUIndex(void)
{
    SYNCLOCK;
    
    SInt8 index = keyStore->takeVacantGPUIndex();
    
    SYNCUNLOCK;
    
    return index;
}

bool FakeSMCPlugin::takeGPUIndex(UInt8 index)
{
    SYNCLOCK;
    
    bool taken = keyStore->takeGPUIndex(index);

    SYNCUNLOCK;
    
    return taken;
}

void FakeSMCPlugin::releaseGPUIndex(UInt8 index)
{
    SYNCLOCK;
    
    keyStore->releaseGPUIndex(index);

    SYNCUNLOCK;
}

SInt8 FakeSMCPlugin::takeVacantFanIndex(void)
{
    SYNCLOCK;
    
    SInt8 index = keyStore->takeVacantFanIndex();

    SYNCUNLOCK;
    
    return index;
}

void FakeSMCPlugin::releaseFanIndex(UInt8 index)
{
    SYNCLOCK;
    
    keyStore->releaseFanIndex(index);
    
    SYNCUNLOCK;
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
    
    if (OSDictionary *list = OSDynamicCast(OSDictionary, getProperty("Platform Profile")))
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

bool FakeSMCPlugin::start(IOService *provider)
{	
	if (!super::start(provider)) 
        return false;

    if (!(keyStore = OSDynamicCast(FakeSMCKeyStore, waitForMatchingService(serviceMatching(kFakeSMCKeyStoreService), kFakeSMCDefaultWaitTimeout)))) {
		HWSensorsFatalLog("still waiting for FakeSMCKeyStore...");
        return false;
    }

	return true;
}

inline UInt8 index_of_hex_char(char c)
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

void FakeSMCPlugin::stop(IOService* provider)
{
    HWSensorsDebugLog("removing handler");

    if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(keyStore->getKeys())) {
        while (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, iterator->getNextObject())) {
            if (key->getHandler() == this) {
                if (FakeSMCSensor *sensor = getSensor(key->getKey())) {
                    if (sensor->getGroup() == kFakeSMCTachometerSensor) {
                        UInt8 index = index_of_hex_char(sensor->getKey()[1]);
                        HWSensorsDebugLog("releasing Fan%X", index);
                        keyStore->releaseFanIndex(index);
                    }
                }

                key->setHandler(NULL);
            }
        }
        OSSafeRelease(iterator);
    }

    HWSensorsDebugLog("releasing sensors collection");
    
    sensors->flushCollection();
	
	super::stop(provider);
}

void FakeSMCPlugin::free()
{
    HWSensorsDebugLog("freenig sensors collection");
    OSSafeRelease(sensors);
	super::free();
}

IOReturn FakeSMCPlugin::getValueCallback(const char *key, const char *type, const UInt8 size, void *buffer)
{
    if (key && type && buffer) {
        if (FakeSMCSensor *sensor = getSensor(key)) {
            if (size == sensor->getSize() && 0 == strncmp(type, sensor->getType(), 4)) {
                sensor->encodeNumericValue(getSensorValue(sensor), buffer);
                return kIOReturnSuccess;
            }
        }
        else return kIOReturnNotFound;
    }

    return kIOReturnBadArgument;
}


/*IOReturn FakeSMCPlugin::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
    if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
        if (param1 && param2 && param3) {
            const char *name = (const char*)param1;
            void *data = param2;
            UInt8 size = (UInt64)param3;
            
            if (name && data)
                if (FakeSMCSensor *sensor = getSensor(name))
                    if (size == sensor->getSize()) {
                        sensor->encodeNumericValue(getSensorValue(sensor), data);
                        return kIOReturnSuccess;
                    }
        }
		
		return kIOReturnBadArgument;
	}
    else if (functionName->isEqualTo(kFakeSMCSetValueCallback)) {
        if (param1 && param2 && param3) {
            const char *name = (const char*)param1;
            void *data = param2;
            UInt8 size = (UInt64)param3;

            if (name && data)
                if (FakeSMCSensor *sensor = getSensor(name))
                    if (size == sensor->getSize()) {
                        setSensorValue(sensor, data);
                        return kIOReturnSuccess;
                    }
        }

		return kIOReturnBadArgument;
	}
    
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}*/