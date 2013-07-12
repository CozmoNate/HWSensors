#include "version.h"

#include "FakeSMC.h"
#include "FakeSMCDefinitions.h"

#include "OEMInfo.h"

#include <IOKit/IODeviceTreeSupport.h>

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
    
    if (PE_parse_boot_argn("-fakesmc_force_start", &arg_value, sizeof(arg_value))) {
        HWSensorsInfoLog("firmware vendor check disabled");
    }
    else {
        IORegistryEntry *efi = IORegistryEntry::fromPath("/efi", gIODTPlane);
        OSData *vendor = efi ? OSDynamicCast(OSData, efi->getProperty("firmware-vendor")) : NULL;
        
        if (vendor && vendor->getLength() == 12 && 0 == memcmp(vendor->getBytesNoCopy(), "A\0p\0p\0l\0e\0\0\0", 12) ) {
            HWSensorsFatalLog("forbidding start on Apple hardware");
            return false;
        }
    }
    
	if (!smcDevice->init(provider, OSDynamicCast(OSDictionary, getProperty("Configuration")))) {
        HWSensorsInfoLog("failed to initialize SMC device");
		return false;
    }

	registerService();
    
    // Chameleon/Chimera exporting NVRAM to IODeviceTree:/chosen/nvram
    IORegistryEntry* nvram = IORegistryEntry::fromPath("/chosen/nvram", gIODTPlane);
    
    //if (vendor && vendor->getLength() == 14 && 0 == memcmp(vendor->getBytesNoCopy(), "C\0L\0O\0V\0E\0R\0\0\0", 14)) {
    
    // Fallback method. Try to access NVRAM via driver
    if (!nvram) {
        if (OSDictionary *matching = serviceMatching("IODTNVRAM")) {
            nvram = OSDynamicCast(IORegistryEntry, waitForMatchingService(matching));
            OSSafeRelease(matching);
        }
    }
    
    if (nvram) {
        if (OSData *keys = OSDynamicCast(OSData, nvram->getProperty(kFakeSMCPropertyKeys))) {
            
            int count = 0;
            unsigned int offset = 0;
            const unsigned char* data = static_cast<const unsigned char*>(keys->getBytesNoCopy());
            const unsigned int length = keys->getLength();
            char name[5]; name[4] = 0;
            char type[5]; type[4] = 0;
            
            while (offset + 9 < length) {
                memcpy(name, data + offset, 4); offset += 4;
                memcpy(type, data + offset, 4); offset += 4;
                unsigned char size = data[offset++];
                const void *value = data + offset; offset += size;
                
                if (FakeSMCKey *key = smcDevice->addKeyWithValue(name, type, size, value)) {
                    // Add key to NVRAM keys list
                    smcDevice->saveKeyToNVRAM(key, false);
                    HWSensorsDebugLog("key %s loaded from NVRAM", name);
                    count++;
                }
                
            }
            
            if (count)
                HWSensorsInfoLog("%d key%s loaded from NVRAM", count, count == 1 ? "" : "s");
        }
        
        OSSafeRelease(nvram);
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