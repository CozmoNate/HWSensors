//
//  FakeSMCKeyStore.cpp
//  HWSensors
//
//  Created by Kozlek on 04/11/13.
//
//

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

#include "FakeSMCKeyStore.h"

#include "FakeSMCDefinitions.h"

#include "FakeSMCKey.h"
#include "FakeSMCKeyHandler.h"
#include "FakeSMCKeyStoreUserClient.h"

#include "OEMInfo.h"

#include <IOKit/IONVRAM.h>
#include <IOKit/IOLib.h>

#define super IOService
OSDefineMetaClassAndStructors(FakeSMCKeyStore, IOService)

#pragma mark -
#pragma mark Key storage engine

UInt32 FakeSMCKeyStore::getCount()
{
    return keys->getCount();
}

void FakeSMCKeyStore::updateKeyCounterKey()
{
	UInt32 count = OSSwapHostToBigInt32(keys->getCount());

	//char value[] = { static_cast<char>(count << 24), static_cast<char>(count << 16), static_cast<char>(count << 8), static_cast<char>(count) };

    //KEYSLOCK;
	keyCounterKey->setValueFromBuffer(&count, 4);
    //KEYSUNLOCK;
}

void FakeSMCKeyStore::updateFanCounterKey()
{
	UInt8 count = 0;

    for (UInt8 i = 0; i <= 0xf; i++) {
        if (bit_get(vacantFanIndex, BIT(i))) {
            count = i + 1;
        }
    }

    //addKeyWithValue(KEY_FAN_NUMBER, TYPE_UI8, TYPE_UI8_SIZE, &count);
    //KEYSLOCK;
	fanCounterKey->setValueFromBuffer(&count, 1);
    //KEYSUNLOCK;
}

FakeSMCKey *FakeSMCKeyStore::addKeyWithValue(const char *name, const char *type, unsigned char size, const void *value)
{
    if (FakeSMCKey *key = getKey(name)) {

//        if (type && strncmp(type, key->getType(), 4) != 0) {
//            key->setType(type);
//        }

        if (value) {
            key->setValueFromBuffer(value, size);
        }

        if (kHWSensorsDebug) {
            if (strncmp("NATJ", key->getKey(), 5) == 0) {
                UInt8 val = *(UInt8*)key->getValue();

                switch (val) {
                    case 0:
                        HWSensorsInfoLog("Ninja Action Timer Job: do nothing");
                        break;

                    case 1:
                        HWSensorsInfoLog("Ninja Action Timer Job: force shutdown to S5");
                        break;

                    case 2:
                        HWSensorsInfoLog("Ninja Action Timer Job: force restart");
                        break;

                    case 3:
                        HWSensorsInfoLog("Ninja Action Timer Job: force startup");
                        break;

                    default:
                        break;
                }
            }
            else if (strncmp("NATi", key->getKey(), 5) == 0) {
                UInt16 val = *(UInt16*)key->getValue();

                HWSensorsInfoLog("Ninja Action Timer is set to %d", val);
            }
            else if (strncmp("MSDW", key->getKey(), 5) == 0) {
                UInt8 val = *(UInt8*)key->getValue();

                switch (val) {
                    case 0:
                        HWSensorsInfoLog("display is now asleep");
                        break;

                    case 1:
                        HWSensorsInfoLog("display is now awake");
                        break;

                    default:
                        break;
                }
            }
        }

		HWSensorsDebugLog("value updated for key %s, type: %s, size: %d", key->getKey(), key->getType(), key->getSize());

		return key;
	}

	HWSensorsDebugLog("adding key %s with value, type: %s, size: %d", name, type, size);

    OSString *wellKnownType = 0;

    if (!type) wellKnownType = OSDynamicCast(OSString, types->getObject(name));

	if (FakeSMCKey *key = FakeSMCKey::withValue(name, type ? type : wellKnownType ? wellKnownType->getCStringNoCopy() : 0, size, value)) {
        //KEYSLOCK;
		keys->setObject(key);
        //KEYSUNLOCK;
		updateKeyCounterKey();
		return key;
	}

	HWSensorsErrorLog("failed to create key %s", name);

	return 0;
}

FakeSMCKey *FakeSMCKeyStore::addKeyWithHandler(const char *name, const char *type, unsigned char size, FakeSMCKeyHandler *handler)
{
	if (FakeSMCKey *key = getKey(name)) {

        FakeSMCKeyHandler *existedHandler = key->getHandler();

        if (handler->getProbeScore() < existedHandler->getProbeScore()) {
            HWSensorsErrorLog("key %s already handled with prioritized handler %s", name, existedHandler ? existedHandler->getName() : "*Unreferenced*");
            return 0;
        }
        else {
            HWSensorsInfoLog("key %s handler %s has been replaced with new prioritized handler %s", name, existedHandler ? existedHandler->getName() : "*Unreferenced*", handler ? handler->getName() : "*Unreferenced*");
        }

        key->setType(type);
        key->setSize(size);
        key->setHandler(handler);

		return key;
	}

	HWSensorsDebugLog("adding key %s with handler, type: %s, size: %d", name, type, size);

	if (FakeSMCKey *key = FakeSMCKey::withHandler(name, type, size, handler)) {
        //KEYSLOCK;
		keys->setObject(key);
        //KEYSUNLOCK;
		updateKeyCounterKey();
		return key;
	}

	HWSensorsErrorLog("failed to create key %s", name);

	return 0;
}

FakeSMCKey *FakeSMCKeyStore::getKey(const char *name)
{
    //KEYSLOCK;
    OSCollection *snapshotKeys = keys->copyCollection();
    //KEYSUNLOCK;

    if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(snapshotKeys)) {

        char validKeyNameBuffer[5];
        copySymbol(name, validKeyNameBuffer);

		while (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, iterator->getNextObject())) {
            UInt32 key1 = HWSensorsKeyToInt(&validKeyNameBuffer);
			UInt32 key2 = HWSensorsKeyToInt(key->getKey());
			if (key1 == key2) {
				OSSafeReleaseNULL(iterator);
                OSSafeReleaseNULL(snapshotKeys);
				return key;
			}
		}

        OSSafeReleaseNULL(iterator);
	}

    OSSafeReleaseNULL(snapshotKeys);

 	HWSensorsDebugLog("key %s not found", name);

	return 0;
}

FakeSMCKey *FakeSMCKeyStore::getKey(unsigned int index)
{
    //KEYSLOCK;
    FakeSMCKey *key = OSDynamicCast(FakeSMCKey, keys->getObject(index));
    //KEYSUNLOCK;

	if (!key) HWSensorsDebugLog("key with index %d not found", index);

	return key;
}

OSArray *FakeSMCKeyStore::getKeys()
{
    return keys;
}

UInt32 FakeSMCKeyStore::addKeysFromDictionary(OSDictionary* dictionary)
{
    UInt32 keysAdded = 0;

    if (dictionary) {
        if (OSIterator *iterator = OSCollectionIterator::withCollection(dictionary)) {
            while (const OSSymbol *key = (const OSSymbol *)iterator->getNextObject()) {
                if (OSArray *array = OSDynamicCast(OSArray, dictionary->getObject(key))) {
                    if (OSIterator *aiterator = OSCollectionIterator::withCollection(array)) {

                        OSString *type = OSDynamicCast(OSString, aiterator->getNextObject());
                        OSData *value = OSDynamicCast(OSData, aiterator->getNextObject());

                        if (type && value) {
                            addKeyWithValue(key->getCStringNoCopy(), type->getCStringNoCopy(), value->getLength(), value->getBytesNoCopy());
                            keysAdded++;
                        }

                        OSSafeReleaseNULL(aiterator);
                    }
                }
                key = 0;
            }
            
            OSSafeReleaseNULL(iterator);
        }
    }

    return keysAdded;
}

UInt32 FakeSMCKeyStore::addWellKnownTypesFromDictionary(OSDictionary* dictionary)
{
    UInt32 typesCount = 0;

    if (OSIterator *iterator = OSCollectionIterator::withCollection(dictionary)) {
        while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
            if (OSString *value = OSDynamicCast(OSString, dictionary->getObject(key))) {
                types->setObject(key, value);
                typesCount++;
            }
        }
        OSSafeReleaseNULL(iterator);
    }

    return typesCount;
}

SInt8 FakeSMCKeyStore::takeVacantGPUIndex()
{
    for (UInt8 i = 0; i <= 0xf; i++) {
        if (!bit_get(vacantGPUIndex, BIT(i))) {
            bit_set(vacantGPUIndex, BIT(i));
            return i;
        }
    }

    return -1;
}

bool FakeSMCKeyStore::takeGPUIndex(UInt8 index)
{
    if (index < 0xf && !bit_get(vacantGPUIndex, BIT(index))) {
        bit_set(vacantGPUIndex, BIT(index));
        return true;
    }

    return false;
}

void FakeSMCKeyStore::releaseGPUIndex(UInt8 index)
{
    if (index <= 0xf)
        bit_clear(vacantGPUIndex, BIT(index));
}

SInt8 FakeSMCKeyStore::takeVacantFanIndex(void)
{
    for (UInt8 i = 0; i <= 0xf; i++) {
        if (!bit_get(vacantFanIndex, BIT(i))) {
            bit_set(vacantFanIndex, BIT(i));
            updateFanCounterKey();
            return i;
        }
    }

    return -1;
}

void FakeSMCKeyStore::releaseFanIndex(UInt8 index)
{
    if (index <= 0xf)
        bit_clear(vacantFanIndex, BIT(index));
}

#pragma mark -
#pragma mark NVRAM

void FakeSMCKeyStore::saveKeyToNVRAM(FakeSMCKey *key)
{
    if (!useNVRAM)
        return;

    if (IORegistryEntry *nvram = OSDynamicCast(IORegistryEntry, fromPath("/options", gIODTPlane))) {
        char name[32];

        snprintf(name, 32, "%s-%s-%s", kFakeSMCKeyPropertyPrefix, key->getKey(), key->getType());

        const OSSymbol *tempName = OSSymbol::withCString(name);

        if (genericNVRAM)
            nvram->IORegistryEntry::setProperty(tempName, OSData::withBytes(key->getValue(), key->getSize()));
        else
            nvram->setProperty(tempName, OSData::withBytes(key->getValue(), key->getSize()));

        OSSafeReleaseNULL(tempName);
        OSSafeReleaseNULL(nvram);
    }
}

UInt32 FakeSMCKeyStore::loadKeysFromNVRAM()
{
    UInt32 count = 0;

    // Find driver and load keys from NVRAM
    if (OSDictionary *matching = serviceMatching("IODTNVRAM")) {
        if (IODTNVRAM *nvram = OSDynamicCast(IODTNVRAM, waitForMatchingService(matching, 1000000000ULL * 15))) {

            useNVRAM = true;

            if ((genericNVRAM = (0 == strncmp(nvram->getName(), "AppleNVRAM", sizeof("AppleNVRAM")))))
                HWSensorsInfoLog("fallback to generic NVRAM methods");

            OSSerialize *s = OSSerialize::withCapacity(0); // Workaround for IODTNVRAM->getPropertyTable returns IOKitPersonalities instead of NVRAM properties dictionary

            if (nvram->serializeProperties(s)) {
                if (OSDictionary *props = OSDynamicCast(OSDictionary, OSUnserializeXML(s->text()))) {
                    if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(props)) {

                        size_t prefix_length = strlen(kFakeSMCKeyPropertyPrefix);

                        char name[5]; name[4] = 0;
                        char type[5]; type[4] = 0;

                        while (OSString *property = OSDynamicCast(OSString, iterator->getNextObject())) {
                            const char *buffer = static_cast<const char *>(property->getCStringNoCopy());

                            if (property->getLength() >= prefix_length + 1 + 4 + 1 + 0 && 0 == strncmp(buffer, kFakeSMCKeyPropertyPrefix, prefix_length)) {
                                if (OSData *data = OSDynamicCast(OSData, props->getObject(property))) {
                                    strncpy(name, buffer + prefix_length + 1, 4); // fakesmc-key-???? ->
                                    strncpy(type, buffer + prefix_length + 1 + 4 + 1, 4); // fakesmc-key-xxxx-???? ->

                                    if (addKeyWithValue(name, type, data->getLength(), data->getBytesNoCopy())) {
                                        HWSensorsDebugLog("key %s of type %s loaded from NVRAM", name, type);
                                        count++;
                                    }
                                }
                            }
                        }
                        
                        OSSafeReleaseNULL(iterator);
                    }
                    
                    OSSafeReleaseNULL(props);
                }
            }
            
            OSSafeReleaseNULL(s);
            OSSafeReleaseNULL(nvram);
        }
        else {
            HWSensorsWarningLog("NVRAM is unavailable");
        }

        OSSafeReleaseNULL(matching);
    }
    
    return count;
}

bool FakeSMCKeyStore::initAndStart(IOService *provider, OSDictionary *properties)
{
    //HWSensorsDebugLog("init()");
    if (!provider || !init())
		return false;

    //HWSensorsDebugLog("attach()");
    attach(provider);

    //HWSensorsDebugLog("start()");
    if (!start(provider))
        return false;

    return true;
}

#pragma mark -
#pragma mark Overridden methods

bool FakeSMCKeyStore::init(OSDictionary *properties)
{
	if (!super::init(properties))
		return false;

	keys = OSArray::withCapacity(2);
    types = OSDictionary::withCapacity(0);

    keyCounterKey = FakeSMCKey::withValue(KEY_COUNTER, TYPE_UI32, TYPE_UI32_SIZE, "\0\0\0\1");
    keys->setObject(keyCounterKey);
    fanCounterKey = FakeSMCKey::withValue(KEY_FAN_NUMBER, TYPE_UI8, TYPE_UI8_SIZE, "\0");
    keys->setObject(fanCounterKey);

	return true;
}

bool FakeSMCKeyStore::start(IOService *provider)
{
	if (!super::start(provider))
        return false;


    // Try to obtain OEM info from Clover EFI
    if (IORegistryEntry* platformNode = fromPath("/efi/platform", gIODTPlane)) {

        if (OSData *data = OSDynamicCast(OSData, platformNode->getProperty("OEMVendor"))) {
            if (OSString *vendor = OSString::withCString((char*)data->getBytesNoCopy())) {
                if (OSString *manufacturer = getManufacturerNameFromOEMName(vendor)) {
                    this->setProperty(kOEMInfoManufacturer, manufacturer);
                    OSSafeReleaseNULL(manufacturer);
                }
                //OSSafeReleaseNULL(vendor);
            }
            //OSSafeReleaseNULL(data);
        }

        if (OSData *data = OSDynamicCast(OSData, platformNode->getProperty("OEMBoard"))) {
            if (OSString *product = OSString::withCString((char*)data->getBytesNoCopy())) {
                this->setProperty(kOEMInfoProduct, product);
                //OSSafeReleaseNULL(product);
            }
            //OSSafeReleaseNULL(data);
        }
    }

    if ((!getProperty(kOEMInfoProduct) || !getProperty(kOEMInfoManufacturer)) && !setOemProperties(this)) {
        HWSensorsErrorLog("failed to get OEM info from Chameleon/Chimera or Clover EFI, platform profiles will be unavailable");
    }

    if (OSString *manufacturer = OSDynamicCast(OSString, getProperty(kOEMInfoManufacturer)) ) {
        this->addKeyWithValue("HWS0", TYPE_CH8, manufacturer->getLength(), manufacturer->getCStringNoCopy());
    }

    if (OSString *product = OSDynamicCast(OSString, getProperty(kOEMInfoProduct)) ) {
        this->addKeyWithValue("HWS1", TYPE_CH8, product->getLength(), product->getCStringNoCopy());
    }

    IOService::publishResource(kFakeSMCKeyStoreService, this);

    registerService();

    HWSensorsInfoLog("started");

	return true;
}

void FakeSMCKeyStore::free()
{
    OSSafeReleaseNULL(keys);
    OSSafeReleaseNULL(types);

    super::free();
}

#pragma mark -
#pragma mark Cross-driver and user client communications

IOReturn FakeSMCKeyStore::newUserClient(task_t owningTask, void *security_id, UInt32 type, IOUserClient ** handler)
{
    FakeSMCKeyStoreUserClient * client = new FakeSMCKeyStoreUserClient;

    if (!client->initWithTask(owningTask, security_id, type, NULL)) {
        client->release();
        return kIOReturnBadArgument;
    }

    if (!client->attach(this)) {
        client->release();
        return kIOReturnUnsupported;
    }

    if (!client->start(this)) {
        client->detach(this);
        client->release();
        return kIOReturnUnsupported;
    }

    *handler = client;

    return kIOReturnSuccess;
}

/*IOReturn FakeSMCKeyStore::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
    IOReturn result = kIOReturnUnsupported;

    SYNCLOCK;

    if (functionName->isEqualTo(kFakeSMCAddKeyHandler)) {

        result = kIOReturnBadArgument;

        if (param1 && param2 && param3 && param4) {
            const char *name = (const char *)param1;
            const char *type = (const char *)param2;
            UInt8 size = (UInt64)param3;
            IOService *handler = (IOService*)param4;

            if (name && type && size > 0 && handler) {
                if (addKeyWithHandler(name, type, size, handler))
                    result = kIOReturnSuccess;
                else
                    result = kIOReturnError;
            }
        }
	}
    else if (functionName->isEqualTo(kFakeSMCGetKeyHandler)) {

        result = kIOReturnBadArgument;

        if (const char *name = (const char *)param1) {

            result = kIOReturnError;

            if (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, getKey(name))) {

                result = kIOReturnSuccess;

                if (key->getHandler()) {

                    result = kIOReturnBadArgument;

                    if (param2) {
                        IOService **handler = (IOService**)param2;
                        *handler = key->getHandler();
                        result = kIOReturnSuccess;
                    }
                }
            }
        }
	}
    else if (functionName->isEqualTo(kFakeSMCRemoveKeyHandler)) {

        result = kIOReturnBadArgument;

        if (param1) {

            result = kIOReturnError;

            if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(getKeys())) {
                IOService *handler = (IOService *)param1;
                while (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, iterator->getNextObject())) {
                    if (key->getHandler() == handler)
                        key->setHandler(NULL);
                }
                result = kIOReturnSuccess;
                OSSafeReleaseNULL(iterator);
            }
        }
    }
    else if (functionName->isEqualTo(kFakeSMCAddKeyValue)) {

        result = kIOReturnBadArgument;

        if (param1 && param2 && param3) {
            const char *name = (const char *)param1;
            const char *type = (const char *)param2;
            UInt8 size = (UInt64)param3;
            const void *value = (const void *)param4;

            if (name && type && size > 0) {
                if (addKeyWithValue(name, type, size, value))
                    result = kIOReturnSuccess;
                else
                    result = kIOReturnError;
            }
        }
	}
    else if (functionName->isEqualTo(kFakeSMCSetKeyValue)) {

        result = kIOReturnBadArgument;

        if (param1 && param2 && param3) {
            const char *name = (const char *)param1;
            UInt8 size = (UInt64)param2;
            const void *data = (const void *)param3;

            result = kIOReturnError;

            if (name && data && size > 0) {
                if (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, getKey(name))) {
                    if (key->setValueFromBuffer(data, size)) {
                        result = kIOReturnSuccess;
                    }
                }
            }
        }
	}
    else if (functionName->isEqualTo(kFakeSMCGetKeyValue)) {

        result = kIOReturnBadArgument;

        if (const char *name = (const char *)param1) {

            result = kIOReturnError;

            if (FakeSMCKey *key = getKey(name)) {

                result = kIOReturnBadArgument;

                if (param2 && param3) {
                    UInt8 *size = (UInt8*)param2;
                    const void **value = (const void **)param3;

                    *size = key->getSize();
                    *value = key->getValue();

                    result = kIOReturnSuccess;
                }
            }
        }
	}
    else if (functionName->isEqualTo(kFakeSMCTakeVacantGPUIndex)) {

        result = kIOReturnBadArgument;

        if (SInt8 *index = (SInt8*)param1) {

            *index = takeVacantGPUIndex();

            if (*index > -1)
                result = kIOReturnSuccess;
            else
                result = kIOReturnError;
        }
    }
    else if (functionName->isEqualTo(kFakeSMCTakeGPUIndex)) {

        result = kIOReturnBadArgument;

        if (UInt8 *index = (UInt8*)param1) {
            if (*index < 0xf) {
                if (takeGPUIndex(*index))
                    result = kIOReturnSuccess;
                else
                    result = kIOReturnError;
            }
        }
    }
    else if (functionName->isEqualTo(kFakeSMCReleaseGPUIndex)) {
        
        result = kIOReturnBadArgument;
        
        if (UInt8 *index = (UInt8*)param1) {
            if (*index <= 0xf) {
                releaseGPUIndex(*index);
                result = kIOReturnSuccess;
            }
        }
    }
    else if (functionName->isEqualTo(kFakeSMCTakeVacantFanIndex)) {
        
        result = kIOReturnBadArgument;
        
        if (SInt8 *index = (SInt8*)param1) {
            
            *index = takeVacantFanIndex();
            
            if (*index > -1)
                result = kIOReturnSuccess;
            else
                result = kIOReturnError;
        }
    }
    else if (functionName->isEqualTo(kFakeSMCReleaseFanIndex)) {
        
        result = kIOReturnBadArgument;
        
        if (UInt8 *index = (UInt8*)param1) {
            if (*index <= 0xf) {
                releaseFanIndex(*index);
                result = kIOReturnSuccess;
            }
        }
    }
    else {
        result = super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
    }
    
    SYNCUNLOCK;
    
	return result;
}*/
