#include "version.h"

#include "FakeSMC.h"
#include "FakeSMCDefinitions.h"

#include "OEMInfo.h"

#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IONVRAM.h>

#define super IOService
OSDefineMetaClassAndStructors (FakeSMC, IOService)

bool FakeSMC::init(OSDictionary *dictionary)
{	
	if (!super::init(dictionary))
		return false;
    
    IOLog("HWSensors v%s Copyright %d netkas, slice, usr-sse2, kozlek, navi, THe KiNG, RehabMan. All rights reserved.\n", HWSENSORS_VERSION_STRING, HWSENSORS_LASTYEAR);
    
    //HWSensorsInfoLog("Opensource SMC device emulator. Copyright 2009 netkas. All rights reserved.");
    
    if (!(smcDevice = new FakeSMCDevice)) {
		HWSensorsInfoLog("failed to create SMC device");
		return false;
	}
    
    setOemProperties(this);
    
    if (!getProperty(kOEMInfoProduct) || !getProperty(kOEMInfoManufacturer)) {

        HWSensorsErrorLog("failed to obtain OEM vendor & product information from DMI");
        
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
            HWSensorsErrorLog("failed to get OEM info from Clover EFI, specific platform profiles will be unavailable");
        }
    }
    
    if (IORegistryEntry *efi = IORegistryEntry::fromPath("/efi", gIODTPlane)) {
        if (OSData *vendor = OSDynamicCast(OSData, efi->getProperty("firmware-vendor"))) {
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
    
    OSString *vendor = OSDynamicCast(OSString, getProperty(kFakeSMCFirmwareVendor));
    
    int arg_value = 1;
    
    if (PE_parse_boot_argn("-fakesmc_force_start", &arg_value, sizeof(arg_value))) {
        HWSensorsInfoLog("firmware vendor check disabled");
    }
    else if (vendor && vendor->isEqualTo("Apple")) {
        HWSensorsFatalLog("forbidding start on Apple hardware");
        return false;
    }
    
	if (!smcDevice->initAndStart(provider, this)) {
        HWSensorsInfoLog("failed to initialize SMC device");
		return false;
    }

	registerService();

    // Find driver and load keys from NVRAM
    if (OSDictionary *matching = serviceMatching("IODTNVRAM")) {
        if (IODTNVRAM *nvram = OSDynamicCast(IODTNVRAM, waitForMatchingService(matching, 1000000000ULL * 10))) {
            
            OSSerialize *s = OSSerialize::withCapacity(0); // Workaround for IODTNVRAM->getPropertyTable returns IOKitPersonalities instead of NVRAM properties dictionary
            
            if (nvram->serializeProperties(s)) {
                if (OSDictionary *props = OSDynamicCast(OSDictionary, OSUnserializeXML(s->text()))) {
                    if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(props)) {
                        
                        int count = 0;
                        size_t prefix_length = strlen(kFakeSMCKeyPropertyPrefix);
                        
                        char name[5]; name[4] = 0;
                        char type[5]; type[4] = 0;
                        
                        while (OSString *property = OSDynamicCast(OSString, iterator->getNextObject())) {
                            const char *buffer = static_cast<const char *>(property->getCStringNoCopy());
                            
                            if (0 == strncmp(buffer, kFakeSMCKeyPropertyPrefix, prefix_length)) {
                                if (OSData *data = OSDynamicCast(OSData, props->getObject(property))) {
                                    memcpy(name, buffer + prefix_length + 1, 4); // fakesmc-key. ->
                                    memcpy(type, buffer + prefix_length + 1 + 4 + 1, 4); // fakesmc-key.xxxx: ->
                                    
                                    if (FakeSMCKey *key = smcDevice->addKeyWithValue(name, type, data->getLength(), data->getBytesNoCopy())) {
                                        HWSensorsInfoLog("key %s of type %s loaded from NVRAM", name, type);
                                        
                                        // Add key to NVRAM keys list
                                        smcDevice->saveKeyToNVRAM(key, false);
                                        
                                        count++;
                                    }
                                }
                            }
                        }
                        
                        if (count) HWSensorsInfoLog("%d key%s loaded from NVRAM", count, count == 1 ? "" : "s");
                        
                        OSSafeRelease(iterator);
                    }

                    OSSafeRelease(props);
                }
            }

            OSSafeRelease(s);
            
//            if (OSData *keys = OSDynamicCast(OSData, nvram->getProperty(kFakeSMCPropertyKeys))) {
//                
//                int count = 0;
//                unsigned int offset = 0;
//                const unsigned char* data = static_cast<const unsigned char*>(keys->getBytesNoCopy());
//                const unsigned int length = keys->getLength();
//                char name[5]; name[4] = 0;
//                char type[5]; type[4] = 0;
//                
//                while (offset + 9 < length) {
//                    memcpy(name, data + offset, 4); offset += 4;
//                    memcpy(type, data + offset, 4); offset += 4;
//                    unsigned char size = data[offset++];
//                    const void *value = data + offset; offset += size;
//                    
//                    if (FakeSMCKey *key = smcDevice->addKeyWithValue(name, type, size, value)) {
//                        HWSensorsDebugLog("key %s loaded from NVRAM", name);
//                        
//                        // Add key to NVRAM keys list
//                        smcDevice->saveKeyToNVRAM(key, false);
//                        
//                        count++;
//                    }
//                }
//                
//                if (count) HWSensorsInfoLog("%d key%s loaded from NVRAM", count, count == 1 ? "" : "s");
//            }
            
            OSSafeRelease(nvram);
        }
        else {
            HWSensorsWarningLog("NVRAM is unavailable");
        }
        
        OSSafeRelease(matching);
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