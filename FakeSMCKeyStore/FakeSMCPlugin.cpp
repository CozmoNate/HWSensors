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

UInt8 fakeSMCPluginGetIndexFromChar(char c)
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
bool fakeSMCPluginEncodeFloatValue(float value, const char *type, const UInt8 size, void *outBuffer)
{
    if (type && outBuffer) {

        size_t typeLength = strnlen(type, 4);

        if (typeLength >= 3 && (type[0] == 'f' || type[0] == 's') && type[1] == 'p') {
            bool minus = value < 0;
            bool signd = type[0] == 's';
            UInt8 i = fakeSMCPluginGetIndexFromChar(type[2]);
            UInt8 f = fakeSMCPluginGetIndexFromChar(type[3]);

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
bool fakeSMCPluginEncodeIntValue(int value, const char *type, const UInt8 size, void *outBuffer)
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
bool fakeSMCPluginIsValidIntegerType(const char *type)
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
bool fakeSMCPluginIsValidFloatingType(const char *type)
{
    if (type) {

        size_t typeLength = strnlen(type, 4);

        if (typeLength >= 3) {
            if ((type[0] == 'f' || type[0] == 's') && type[1] == 'p') {
                UInt8 i = fakeSMCPluginGetIndexFromChar(type[2]);
                UInt8 f = fakeSMCPluginGetIndexFromChar(type[3]);

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
bool fakeSMCPluginDecodeFloatValue(const char *type, const UInt8 size, const void *data, float *outValue)
{
    if (type && data && outValue) {

        size_t typeLength = strnlen(type, 4);

        if (typeLength >= 3 && (type[0] == 'f' || type[0] == 's') && type[1] == 'p' && size == 2) {
            UInt16 encoded = 0;

            bcopy(data, &encoded, 2);

            UInt8 i = fakeSMCPluginGetIndexFromChar(type[2]);
            UInt8 f = fakeSMCPluginGetIndexFromChar(type[3]);

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
bool fakeSMCPluginDecodeIntValue(const char *type, const UInt8 size, const void *data, int *outValue)
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
        OSSafeRelease(me);

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
    if (!fakeSMCPluginEncodeFloatValue(value, type, size, outBuffer)) {
        fakeSMCPluginEncodeIntValue(value, type, size, outBuffer);
    }
}

#pragma mark
#pragma mark FakeSMCPlugin

#include "OEMInfo.h"

static IORecursiveLock *gFakeSMCPluginLock = 0;
#define LOCK    IORecursiveLockLock(gFakeSMCPluginLock)
#define UNLOCK    IORecursiveLockUnlock(gFakeSMCPluginLock)

#define super FakeSMCKeyHandler
OSDefineMetaClassAndAbstractStructors(FakeSMCPlugin, FakeSMCKeyHandler)

#pragma mark -
#pragma mark FakeSMCPlugin::methods

/**
 *  Method to get platform (matherboard) manufacturer name provided by FakeSMCKeyStore
 *
 *  @return OSString containg platform manufacturer name
 */
OSString *FakeSMCPlugin::getPlatformManufacturer(void)
{
    return OSDynamicCast(OSString, keyStore->getProperty(kOEMInfoManufacturer));
}

/**
 *  Method to get platform (matherboard) product name provided by FakeSMCKeyStore
 *
 *  @return OSString containg platform product name
 */
OSString *FakeSMCPlugin::getPlatformProduct(void)
{
    return OSDynamicCast(OSString, keyStore->getProperty(kOEMInfoProduct));
}

/**
 *  Engage global FakeSMCPlugin access lock
 */
void FakeSMCPlugin::lockAccessForOtherPlugins(void)
{
    IORecursiveLockLock(gFakeSMCPluginLock);
}

/**
 *  Disengage global FakeSMCPlugin access lock
 */
void FakeSMCPlugin::unlockAccessForOtherPlugins(void)
{
    IORecursiveLockUnlock(gFakeSMCPluginLock);
}

/**
 *  Synchronized method to check key availability
 *
 *  @param key Key name
 *
 *  @return True if the key exists False otherwise
 */
bool FakeSMCPlugin::isKeyExists(const char *key)
{
    LOCK;

    bool keyExists = keyStore->getKey(key);

    UNLOCK;

    return keyExists;
}

/**
 *  Synchronized method to check if specified key value is provided by handler service
 *
 *  @param key Key name
 *
 *  @return True if the key is handled False otherwise
 */
bool FakeSMCPlugin::isKeyHandled(const char *key)
{
    LOCK;

    bool keyHandled = false;

    if (FakeSMCKey *smcKey = keyStore->getKey(key))
        keyHandled = smcKey->getHandler();

    UNLOCK;

    return keyHandled;
}

/**
 *  Synchronized method to set a value for specified key. If the key is handled by other service value will be set and didWriteSensorValue callback will be invoked for handler service
 *
 *  @param key   Key name for wich we want to set value
 *  @param type  Key type can be changed in realtime but not recomended
 *  @param size  Value size in bytes
 *  @param value Pointer to buffer containg new value for specified key. Value should be already encoded regarding key type
 *
 *  @return True on successfull action False otherwise
 */
bool FakeSMCPlugin::setKeyValue(const char *key, const char *type, UInt8 size, void *value)
{
    LOCK;

    bool added = keyStore->addKeyWithValue(key, type, size, value);

    UNLOCK;

    return added;
}

/**
 *  Synchronized method to copy sensor value to specified buffer
 *
 *  @param key   Key name
 *  @param value Buffer to copy value to. Buffer should be already allocated with proper size to fit key value
 *
 *  @return True on success False otherwise
 */
bool FakeSMCPlugin::getKeyValue(const char *key, void *value)
{
    LOCK;

    FakeSMCKey *smcKey = keyStore->getKey(key);

    if (smcKey) {
        memcpy(value, smcKey->getValue(), smcKey->getSize());
    }

    UNLOCK;

    return smcKey != NULL;
}

/**
 *  Synchronized method to add a new key to FakeSMCKeyStore and set its handler to the plugin
 *
 *  @param key       Key name
 *  @param type      Key type
 *  @param size      Key value size
 *  @param group     Key group
 *  @param index     Key index not related to key position in FakeSMCKeyStore key list
 *  @param reference Reference modifier value
 *  @param gain      Gain modifier value
 *  @param offset    Offset modifier value
 *
 *  @return new FakeSMCSensor object or NULL otherwise
 */
FakeSMCSensor *FakeSMCPlugin::addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    LOCK;

    if (FakeSMCSensor *sensor = FakeSMCSensor::withOwner(this, key, type, size, group, index, reference, gain, offset)) {
        if (addSensor(sensor)) {
            UNLOCK;
            return sensor;
        }
        else OSSafeRelease(sensor);
    }

    UNLOCK;

	return NULL;
}

/**
 *  Synchronized method to add a new key to FakeSMCKeyStore and set its handler to the plugin
 *
 *  @param abbreviation Human readable key abbreviation used in config file
 *  @param category     Category (enum kFakeSMCCategory) key belongs to so abbreviation can be checked against the list from specified category only
 *  @param group        Key group
 *  @param index        Key index not related to key position in FakeSMCKeyStore key list
 *  @param reference Reference modifier value
 *  @param gain         Gain modifier value
 *  @param offset       Offset modifier value
 *
 *  @return new FakeSMCSensor object or NULL otherwise
 */
FakeSMCSensor *FakeSMCPlugin::addSensor(const char *abbreviation, FakeSMCSensorCategory category, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    LOCK;

    FakeSMCSensor *sensor = NULL;

    if (abbreviation && strlen(abbreviation) >= 3) {
        for (int i = 0; FakeSMCSensorDefinitions[i].name; i++) {

            FakeSMCSensorDefinitionEntry entry = FakeSMCSensorDefinitions[i];

            if (entry.category == category && 0 == strcasecmp(entry.name, abbreviation)) {
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

    UNLOCK;

    return sensor;
}

/**
 *  Synchronized method to add a new key from configuration node, set handler to this plugin, set also modifiers: reference, gain and offset read from config if present
 *
 *  @param node     OSDictionary configuration node contains key configuration
 *  @param category Category (enum kFakeSMCCategory) key belongs to so abbreviation can be checked against the list from specified category only
 *  @param group    Key group
 *  @param index    Key index not related to key position in FakeSMCKeyStore key list
 *
 *  @return new FakeSMCSensor object or NULL otherwise
 */
FakeSMCSensor *FakeSMCPlugin::addSensor(OSObject *node, FakeSMCSensorCategory category, UInt32 group, UInt32 index)
{
    LOCK;

    FakeSMCSensor *sensor = NULL;

    if (node) {

        float reference = 0, gain = 0, offset = 0;
        OSString *abbreviation = NULL;

        if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, node)) {
            if ((abbreviation = OSDynamicCast(OSString, dictionary->getObject("name")))) {
                FakeSMCSensor::parseModifiers(dictionary, &reference, &gain, &offset);
            }
        }
        else abbreviation = OSDynamicCast(OSString, node);

        if (abbreviation)
            sensor = addSensor(abbreviation->getCStringNoCopy(), category, group, index, reference, gain, offset);
    }

    UNLOCK;
    return sensor;
}

/**
 *  Synchronized method to add previously allocated and configured sensor into FakeSMCKeyStore, set represented key handler to this plugin
 *
 *  @param sensor Sensor object
 *
 *  @return True on success False otherwise
 */
bool FakeSMCPlugin::addSensor(FakeSMCSensor *sensor)
{
    LOCK;

    bool added = keyStore->addKeyWithHandler(sensor->getKey(), sensor->getType(), sensor->getSize(), this);

    if (added) {
        sensors->setObject(sensor->getKey(), sensor);
    }

    UNLOCK;

    return added;
}

/**
 *  Synchronized method to add tachometer sensor type into FakeSMCKeyStore. This will update fan counter key.
 *
 *  @param index    Key index not related to key position in FakeSMCKeyStore key list
 *  @param name     SMC fan name string up to 12 bytes long
 *  @param type     SMC fan type
 *  @param zone     SMC fan zone
 *  @param location SMC fan location
 *  @param fanIndex SMC fan index is returned on success
 *
 *  @return new FakeSMCSensor object or NULL otherwise
 */
FakeSMCSensor *FakeSMCPlugin::addTachometer(UInt32 index, const char *name, FanType type, UInt8 zone, FanLocationType location, SInt8 *fanIndex)
{
    LOCK;

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

            UNLOCK;

            return sensor;
        }
        else HWSensorsErrorLog("failed to add tachometer sensor for key %s", key);
    }
    else HWSensorsErrorLog("failed to take vacant Fan index");

    UNLOCK;

	return 0;
}

/**
 *  Method to get a key by name
 *
 *  @param key Key name
 *
 *  @return FakeSMCSensor object or NULL if the key not found
 */
FakeSMCSensor *FakeSMCPlugin::getSensor(const char* key)
{
	return OSDynamicCast(FakeSMCSensor, sensors->getObject(key));
}

/**
 *  Callback method invoked before key value will be read. Can be used by plugin to provide custom key value that can be calculated or obtained in the moment of key read action. Blocks key reading thread until returned
 *
 *  @param sensor   FakeSMCSensor object representing specific SMC key will be read
 *  @param outValue floating point key value will be exposed to SMC
 *
 *  @return if True is returned outValue will be used as a value for sensor handled SMC key. if False is returned SMC key value will not be changed
 */
bool FakeSMCPlugin::willReadSensorValue(FakeSMCSensor *sensor, float *outValue)
{
    return false;
}

/**
 *  Callback method invoked after the new value has been written to specific key. Can be used by plugin to handle key writes. Blocks key writing thread until returned
 *
 *  @param sensor FakeSMCSensor object representing specific SMC key has been written
 *  @param value  floating point value written to SMC key
 *
 *  @return not used
 */
bool FakeSMCPlugin::didWriteSensorValue(FakeSMCSensor *sensor, float value)
{
    return false;
}

/**
 *  Synchronized method that tries to take vacant GPU index available from internal list
 *
 *  @return Index taken (starting from 0) or -1 on failure
 */
SInt8 FakeSMCPlugin::takeVacantGPUIndex(void)
{
    LOCK;

    SInt8 index = keyStore->takeVacantGPUIndex();

    UNLOCK;

    return index;
}

/**
 *  Synchronized method that tries to take specific GPU index available from internal list
 *
 *  @param index GPU index to take
 *
 *  @return True if the index was taken successfully False otherwise
 */
bool FakeSMCPlugin::takeGPUIndex(UInt8 index)
{
    LOCK;

    bool taken = keyStore->takeGPUIndex(index);

    UNLOCK;

    return taken;
}

/**
 *  Synchronized method that releases previousely taken GPU index by this or another plugin
 *
 *  @param index Index to release
 */
void FakeSMCPlugin::releaseGPUIndex(UInt8 index)
{
    LOCK;

    keyStore->releaseGPUIndex(index);

    UNLOCK;
}

/**
 *  Synchronized method that takes vacant Fan index available from internal list. Will update fan counter key on success
 *
 *  @return Index taken (starting from 0) or -1 on failure
 */
SInt8 FakeSMCPlugin::takeVacantFanIndex(void)
{
    LOCK;

    SInt8 index = keyStore->takeVacantFanIndex();

    UNLOCK;

    return index;
}

/**
 *  Synchronized method that releases previousely taken Fan index by this or another plugin. Will update fan counter key on success
 *
 *  @param index Index to release
 */
void FakeSMCPlugin::releaseFanIndex(UInt8 index)
{
    LOCK;

    keyStore->releaseFanIndex(index);

    UNLOCK;
}

/**
 *  Will try to decode floating point value first, will fallback to decode integer value
 *
 *  @param name     Key name
 *  @param outValue Decoded value will be returned os success
 *
 *  @return True on success False otherwise
 */
bool FakeSMCPlugin::decodeFloatValueForKey(const char *name, float *outValue)
{
    if (!outValue)
        return false;

    if (FakeSMCKey *key = keyStore->getKey(name)) {
        if (fakeSMCPluginDecodeFloatValue(key->getType(), key->getSize(), key->getValue(), outValue)) {
            return true;
        }
        else {

            int intValue = 0;

            if (fakeSMCPluginDecodeIntValue(key->getType(), key->getSize(), key->getValue(), &intValue)) {
                *outValue = (float)intValue;
                return true;
            }
        }
    }

    return false;
}

/**
 *  Will try to decode integer value first, will fallback to decode floating point value
 *
 *  @param name     Key name
 *  @param outValue Decoded value will be returned os success
 *
 *  @return True on success False otherwise
 */
bool FakeSMCPlugin::decodeIntValueForKey(const char *name, int *outValue)
{
    if (!outValue)
        return false;

    if (FakeSMCKey *key = keyStore->getKey(name)) {
        if (fakeSMCPluginDecodeIntValue(key->getType(), key->getSize(), key->getValue(), outValue)) {
            return true;
        }
        else {

            float floatValue = 0;

            if (fakeSMCPluginDecodeFloatValue(key->getType(), key->getSize(), key->getValue(), &floatValue)) {
                *outValue = (int)floatValue;
                return true;
            }
        }
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

/**
 *  Look for configuration node for specific model in plugin properties. Will search manufacturer and product configuration nodes. Will return "Default" configuration node from product node if the model was not found or "Default" node from manufacturer node if product was not found
 *
 *  @param model Model to load configuration for
 *
 *  @return OSDictionary object containg configuration node found or NULL otherwise
 */
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

/**
 *  For internal use, do not override
 *
 */
bool FakeSMCPlugin::init(OSDictionary *properties)
{
    if (!gFakeSMCPluginLock)
        gFakeSMCPluginLock = IORecursiveLockAlloc();

    if (!super::init(properties))
        return false;

    sensors = OSDictionary::withCapacity(0);

    if (!sensors)
        return false;

	return true;
}

/**
 *  For internal use, do not override
 *
 */
bool FakeSMCPlugin::start(IOService *provider)
{
	if (!super::start(provider))
        return false;

    if (OSDictionary *matching = serviceMatching(kFakeSMCKeyStoreService)) {
        if (!(keyStore = OSDynamicCast(FakeSMCKeyStore, waitForMatchingService(matching, kFakeSMCDefaultWaitTimeout)))) {
            HWSensorsFatalLog("still waiting for FakeSMCKeyStore...");
            return false;
        }

        OSSafeRelease(matching);
    }

	return true;
}

inline UInt8 index_of_hex_char(char c)
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

/**
 *  For internal use, do not override
 *
 */
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

/**
 *  For internal use, do not override
 *
 */
void FakeSMCPlugin::free()
{
    HWSensorsDebugLog("freenig sensors collection");
    OSSafeRelease(sensors);
	super::free();
}

/**
 *  For internal use, do not override
 *
 */
IOReturn FakeSMCPlugin::readKeyCallback(const char *key, const char *type, const UInt8 size, void *buffer)
{
    if (key && buffer) {
        if (FakeSMCSensor *sensor = getSensor(key)) {
            if (size == sensor->getSize()) {

                float value;

                if (willReadSensorValue(sensor, &value)) {
                    sensor->encodeNumericValue(value, buffer);
                }

                return kIOReturnSuccess;
            }
        }
        else return kIOReturnNotFound;
    }

    return kIOReturnBadArgument;
}

/**
 *  For internal use, do not override
 *
 */
IOReturn FakeSMCPlugin::writeKeyCallback(const char *key, const char *type, const UInt8 size, const void *buffer)
{       
    if (key && type && buffer) {
        if (FakeSMCSensor *sensor = getSensor(key)) {
            if (size == sensor->getSize()) {
                float floatValue = 0;
                int intValue = 0;

                if (fakeSMCPluginDecodeFloatValue(type, size, buffer, &floatValue)) {
                    didWriteSensorValue(sensor, floatValue);
                }
                else if (fakeSMCPluginDecodeIntValue(type, size, buffer, &intValue)) {
                    didWriteSensorValue(sensor, intValue);
                }
                
                return kIOReturnSuccess;
            }
        }
        else return kIOReturnNotFound;
    }
    
    return kIOReturnBadArgument;
}
