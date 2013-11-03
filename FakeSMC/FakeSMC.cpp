#include "version.h"

#include "FakeSMC.h"
#include "FakeSMCDevice.h"
#include "FakeSMCDefinitions.h"
#include "FakeSMCUserClient.h"

#include "OEMInfo.h"

#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IONVRAM.h>

static IORecursiveLock *gKeysSyncLock = 0;

#define SYNCLOCK        if (!gKeysSyncLock) gKeysSyncLock = IORecursiveLockAlloc(); IORecursiveLockLock(gKeysSyncLock)
#define SYNCUNLOCK      IORecursiveLockUnlock(gKeysSyncLock)

#define super IOService
OSDefineMetaClassAndStructors (FakeSMC, IOService)

#pragma mark -
#pragma mark Key storage engine

UInt32 FakeSMC::getCount()
{
    return keys->getCount();
}

void FakeSMC::updateKeyCounterKey()
{
	UInt32 count = OSSwapHostToBigInt32(keys->getCount());

	//char value[] = { static_cast<char>(count << 24), static_cast<char>(count << 16), static_cast<char>(count << 8), static_cast<char>(count) };

    //KEYSLOCK;
	keyCounterKey->setValueFromBuffer(&count, 4);
    //KEYSUNLOCK;
}

void FakeSMC::updateFanCounterKey()
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

FakeSMCKey *FakeSMC::addKeyWithValue(const char *name, const char *type, unsigned char size, const void *value)
{
    if (FakeSMCKey *key = getKey(name)) {

        if (type && strncmp(type, key->getType(), 4) == 0) {
            key->setType(type);
        }

        if (value) {
            key->setSize(size);
            key->setValueFromBuffer(value, size);
        }

        if (debug) {
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

		HWSensorsDebugLog("value updated for key %s, type: %s, size: %d", name, type, size);

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

inline UInt32 getHandlingPriority(IOService *service)
{
    if (service != NULL) {
        if (OSNumber *priority = OSDynamicCast(OSNumber, service->getProperty("IOProbeScore"))) {
            return priority->unsigned32BitValue();
        }
    }

    return -1;
}

FakeSMCKey *FakeSMC::addKeyWithHandler(const char *name, const char *type, unsigned char size, IOService *handler)
{
	if (FakeSMCKey *key = getKey(name)) {

        IOService *existedHandler = key->getHandler();

        if (getHandlingPriority(handler) < getHandlingPriority(existedHandler)) {
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

FakeSMCKey *FakeSMC::getKey(const char *name)
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
				OSSafeRelease(iterator);
                OSSafeRelease(snapshotKeys);
				return key;
			}
		}

        OSSafeRelease(iterator);
	}

    OSSafeRelease(snapshotKeys);

 	HWSensorsDebugLog("key %s not found", name);

	return 0;
}

FakeSMCKey *FakeSMC::getKey(unsigned int index)
{
    //KEYSLOCK;
    FakeSMCKey *key = OSDynamicCast(FakeSMCKey, keys->getObject(index));
    //KEYSUNLOCK;

	if (!key) HWSensorsDebugLog("key with index %d not found", index);

	return key;
}

OSArray *FakeSMC::getKeys()
{
    return keys;
}

SInt8 FakeSMC::takeVacantGPUIndex()
{
    for (UInt8 i = 0; i <= 0xf; i++) {
        if (!bit_get(vacantGPUIndex, BIT(i))) {
            bit_set(vacantGPUIndex, BIT(i));
            return i;
        }
    }

    return -1;
}

bool FakeSMC::takeGPUIndex(UInt8 index)
{
    if (index < 0xf && !bit_get(vacantGPUIndex, BIT(index))) {
        bit_set(vacantGPUIndex, BIT(index));
        return true;
    }

    return false;
}

void FakeSMC::releaseGPUIndex(UInt8 index)
{
    if (index <= 0xf)
        bit_clear(vacantGPUIndex, BIT(index));
}

SInt8 FakeSMC::getVacantFanIndex(void)
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

void FakeSMC::releaseFanIndex(UInt8 index)
{
    if (index <= 0xf)
        bit_clear(vacantFanIndex, BIT(index));
}

#pragma mark -
#pragma mark NVRAM

void FakeSMC::saveKeyToNVRAM(FakeSMCKey *key)
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

        OSSafeRelease(tempName);
        OSSafeRelease(nvram);
    }
}

UInt32 FakeSMC::loadKeysFromNVRAM()
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

                        OSSafeRelease(iterator);
                    }

                    OSSafeRelease(props);
                }
            }

            OSSafeRelease(s);
            OSSafeRelease(nvram);
        }
        else {
            HWSensorsWarningLog("NVRAM is unavailable");
        }

        OSSafeRelease(matching);
    }

    return count;
}

#pragma mark -
#pragma mark Overridden methods

bool FakeSMC::init(OSDictionary *properties)
{	
	if (!super::init(properties))
		return false;
    
    IOLog("HWSensors v%s Copyright %d netkas, slice, usr-sse2, kozlek, navi, THe KiNG, RehabMan. All rights reserved.\n", HWSENSORS_VERSION_STRING, HWSENSORS_LASTYEAR);

    //HWSensorsInfoLog("Opensource SMC device emulator. Copyright 2009 netkas. All rights reserved.");

	keys = OSArray::withCapacity(1);
    types = OSDictionary::withCapacity(0);

    // Add fist key - counter key
    keyCounterKey = FakeSMCKey::withValue(KEY_COUNTER, TYPE_UI32, TYPE_UI32_SIZE, "\0\0\0\1");
	keys->setObject(keyCounterKey);

    fanCounterKey = FakeSMCKey::withValue(KEY_FAN_NUMBER, TYPE_UI8, TYPE_UI8_SIZE, "\0");
    keys->setObject(fanCounterKey);

    // Load preconfigured keys
    HWSensorsDebugLog("loading keys...");

    OSDictionary *configuration = OSDynamicCast(OSDictionary, properties->getObject("Configuration"));

    if (!configuration) {
        HWSensorsFatalLog("no configuration node found!");
        return false;
    }

    if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, configuration->getObject("Keys"))) {
		if (OSIterator *iterator = OSCollectionIterator::withCollection(dictionary)) {
			while (const OSSymbol *key = (const OSSymbol *)iterator->getNextObject()) {
				if (OSArray *array = OSDynamicCast(OSArray, dictionary->getObject(key))) {
					if (OSIterator *aiterator = OSCollectionIterator::withCollection(array)) {

						OSString *type = OSDynamicCast(OSString, aiterator->getNextObject());
						OSData *value = OSDynamicCast(OSData, aiterator->getNextObject());

						if (type && value)
							addKeyWithValue(key->getCStringNoCopy(), type->getCStringNoCopy(), value->getLength(), value->getBytesNoCopy());

                        OSSafeRelease(aiterator);
					}
				}
				key = 0;
			}

			OSSafeRelease(iterator);
		}

		HWSensorsInfoLog("%d preconfigured key%s added", keys->getCount(), keys->getCount() == 1 ? "" : "s");
	}
	else {
		HWSensorsWarningLog("no preconfigured keys found");
	}

    // Load wellknown type names
    HWSensorsDebugLog("loading types...");

    if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, configuration->getObject("Types"))) {
        if (OSIterator *iterator = OSCollectionIterator::withCollection(dictionary)) {
			while (OSString *key = OSDynamicCast(OSString, iterator->getNextObject())) {
                if (OSString *value = OSDynamicCast(OSString, dictionary->getObject(key))) {
                    types->setObject(key, value);
                }
            }
            OSSafeRelease(iterator);
        }
    }

    // Set Clover platform keys
    if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, configuration->getObject("Clover"))) {
        UInt32 count = 0;
        if (IORegistryEntry* cloverPlatformNode = fromPath("/efi/platform", gIODTPlane)) {
            if (OSIterator *iterator = OSCollectionIterator::withCollection(dictionary)) {
                while (OSString *name = OSDynamicCast(OSString, iterator->getNextObject())) {
                    if (OSData *data = OSDynamicCast(OSData, cloverPlatformNode->getProperty(name))) {
                        if (OSArray *items = OSDynamicCast(OSArray, dictionary->getObject(name))) {
                            OSString *key = OSDynamicCast(OSString, items->getObject(0));
                            OSString *type = OSDynamicCast(OSString, items->getObject(1));

                            if (addKeyWithValue(key->getCStringNoCopy(), type->getCStringNoCopy(), data->getLength(), data->getBytesNoCopy()))
                                count++;
                        }
                    }
                }
                OSSafeRelease(iterator);
            }
        }

        if (count)
            HWSensorsInfoLog("%d key%s exported by Clover EFI", count, count == 1 ? "" : "s");
    }
    
    if (!setOemProperties(this)) {
        // Another try after 200 ms spin
        IOSleep(200);
        setOemProperties(this);
    }
    
    if (!getProperty(kOEMInfoProduct) || !getProperty(kOEMInfoManufacturer)) {

        //HWSensorsErrorLog("failed to obtain OEM vendor & product information from DMI");
        
        // Try to obtain OEM info from Clover EFI
        if (IORegistryEntry* platformNode = fromPath("/efi/platform", gIODTPlane)) {
            
            if (OSData *data = OSDynamicCast(OSData, platformNode->getProperty("OEMVendor"))) {
                if (OSString *vendor = OSString::withCString((char*)data->getBytesNoCopy())) {
                    if (OSString *manufacturer = getManufacturerNameFromOEMName(vendor)) {
                        this->setProperty(kOEMInfoManufacturer, manufacturer);
                        //OSSafeReleaseNULL(manufacturer);
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
        else {
            HWSensorsErrorLog("failed to get OEM info from DMI or Clover EFI, specific platform profiles will be unavailable");
        }
    }
    
    if (IORegistryEntry *efi = IORegistryEntry::fromPath("/efi", gIODTPlane)) {
        if (OSData *vendor = OSDynamicCast(OSData, efi->getProperty("firmware-vendor"))) { // firmware-vendor is in EFI node
            OSData *buffer = OSData::withCapacity(128);
            const unsigned char* data = static_cast<const unsigned char*>(vendor->getBytesNoCopy());
            
            for (unsigned int index = 0; index < vendor->getLength(); index += 2) {
                buffer->appendByte(data[index], 1);
            }
            
            OSString *name = OSString::withCString(static_cast<const char *>(buffer->getBytesNoCopy()));
            
            setProperty(kFakeSMCFirmwareVendor, name);
            
            //OSSafeRelease(vendor);
            //OSSafeRelease(name);
            OSSafeRelease(buffer);
        }
        
        OSSafeRelease(efi);
    }
		
	return true;
}

IOService *FakeSMC::probe(IOService *provider, SInt32 *score)
{
    if (!super::probe(provider, score))
		return 0;
	
	return this;
}

bool FakeSMC::start(IOService *provider)
{
	if (!super::start(provider)) 
        return false;
    
    int arg_value = 1;
    
    // Check if we have SMC already
    bool smcDeviceFound = false;

    if (OSDictionary *matching = serviceMatching("IOACPIPlatformDevice")) {
        if (OSIterator *iterator = getMatchingServices(matching)) {
            
            OSString *smcNameProperty = OSString::withCString("APP0001");

            while (IOService *service = (IOService*)iterator->getNextObject()) {
                
                OSObject *serviceNameProperty = service->getProperty("name");
                
                if (serviceNameProperty && serviceNameProperty->isEqualTo(smcNameProperty)) {
                    smcDeviceFound = true;
                }
            }
            
            OSSafeRelease(iterator);
        }
        
        OSSafeRelease(matching);
    }

    if (!smcDeviceFound) {
        if (!(smcDevice = new FakeSMCDevice)) {
            HWSensorsInfoLog("failed to create SMC device");
            return false;
        }

        if (!smcDevice->initAndStart(provider, this)) {
            HWSensorsFatalLog("failed to initialize SMC device");
            return false;
        }
    }
    else {
        HWSensorsInfoLog("found physical SMC device, will not create virtual one. Providing only basic plugins functionality");
    }

	registerService();
    
    // Load keys from NVRAM
    if (PE_parse_boot_argn("-fakesmc-use-nvram", &arg_value, sizeof(arg_value))) {
        if (UInt32 count = loadKeysFromNVRAM())
            HWSensorsInfoLog("%d key%s loaded from NVRAM", count, count == 1 ? "" : "s");
        else
            HWSensorsInfoLog("NVRAM will be used to store system written keys...");
    }

	return true;
}

void FakeSMC::stop(IOService *provider)
{
    super::stop(provider);
}

void FakeSMC::free()
{
    super::free();
}

#pragma mark -
#pragma mark Cross-driver and user client communications

IOReturn FakeSMC::newUserClient(task_t owningTask, void *security_id, UInt32 type, IOUserClient ** handler)
{
    FakeSMCUserClient * client = new FakeSMCUserClient;

    if ( !client->initWithTask(owningTask, security_id, type, NULL) ) {
        client->release();
        return kIOReturnBadArgument;
    }

    if ( !client->attach(this) ) {
        client->release();
        return kIOReturnUnsupported;
    }

    if ( !client->start(this) ) {
        client->detach(this);
        client->release();
        return kIOReturnUnsupported;
    }

    *handler = client;

    return kIOReturnSuccess;
}

IOReturn FakeSMC::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
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
                OSSafeRelease(iterator);
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

            *index = getVacantFanIndex();

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
}