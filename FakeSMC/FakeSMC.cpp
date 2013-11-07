#include "version.h"

#include "FakeSMCKey.h"
#include "FakeSMCKeyStore.h"
#include "FakeSMCKeyStoreUserClient.h"
#include "FakeSMC.h"
#include "FakeSMCDevice.h"
#include "FakeSMCDefinitions.h"

#include "OEMInfo.h"

#include <IOKit/IODeviceTreeSupport.h>

#define super IOService
OSDefineMetaClassAndStructors (FakeSMC, IOService)

#pragma mark -
#pragma mark Overridden methods

bool FakeSMC::init(OSDictionary *properties)
{	
	if (!super::init(properties))
		return false;
    
    IOLog("FakeSMC v%s Copyright %d netkas, slice, usr-sse2, kozlek, navi, THe KiNG, RehabMan. All rights reserved.\n", HWSENSORS_VERSION_STRING, HWSENSORS_LASTYEAR);

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

bool FakeSMC::start(IOService *provider)
{
	if (!super::start(provider)) 
        return false;

    if (IOService *resources = waitForMatchingService(serviceMatching("IOResources"), 0))
        this->attach(resources);

    OSDictionary *configuration = OSDynamicCast(OSDictionary, getProperty("Configuration"));

    if (!(keyStore = OSDynamicCast(FakeSMCKeyStore, waitForMatchingService(serviceMatching(kFakeSMCKeyStoreService), 0)))) {

        HWSensorsDebugLog("creating FakeSMCKeyStore");
        
        if (!(keyStore = new FakeSMCKeyStore)) {
            HWSensorsInfoLog("failed to create FakeSMCKeyStore");
            return false;
        }

        HWSensorsDebugLog("initializing FakeSMCKeyStore");

        if (keyStore->initAndStart(this, configuration)) {
            keyStore->setProperty("IOUserClientClass", "FakeSMCKeyStoreUserClient");
        }
        else {
            keyStore->release();
            HWSensorsFatalLog("failed to initialize FakeSMCKeyStore device");
            return false;
        }
    }

    // Load preconfigured keys
    HWSensorsDebugLog("loading keys...");

    if (!configuration) {
        HWSensorsFatalLog("no configuration node found!");
        return false;
    }

    if (UInt32 count = keyStore->addKeysFromDictionary(OSDynamicCast(OSDictionary, configuration->getObject("Keys")))) {
        HWSensorsInfoLog("%d preconfigured key%s added", count, count == 1 ? "" : "s");
    }
	else {
		HWSensorsWarningLog("no preconfigured keys found");
	}

    // Load wellknown type names
    HWSensorsDebugLog("loading types...");

    keyStore->addWellKnownTypesFromDictionary(OSDynamicCast(OSDictionary, configuration->getObject("Types")));

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

                            if (keyStore->addKeyWithValue(key->getCStringNoCopy(), type->getCStringNoCopy(), data->getLength(), data->getBytesNoCopy()))
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

        IOService *platformExpert = waitForMatchingService(serviceMatching("IOACPIPlatformExpert"), kFakeSMCDefaultWaitTimeout);

        if (!smcDevice->initAndStart(platformExpert, this)) {
            HWSensorsFatalLog("failed to initialize SMC device");
            return false;
        }
    }
    else {
        HWSensorsInfoLog("found physical SMC device, will not create virtual one. Providing only basic plugins functionality");
    }
    
    // Load keys from NVRAM
    if (PE_parse_boot_argn("-fakesmc-use-nvram", &arg_value, sizeof(arg_value))) {
        if (UInt32 count = keyStore->loadKeysFromNVRAM())
            HWSensorsInfoLog("%d key%s loaded from NVRAM", count, count == 1 ? "" : "s");
        else
            HWSensorsInfoLog("NVRAM will be used to store system written keys...");
    }

  	registerService();

	return true;
}

