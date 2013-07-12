

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
    
    IOLog("HWSensors Project Copyright %d netkas, slice, usr-sse2, kozlek, navi, THe KiNG, RehabMan. All rights reserved.\n",HWSENSORS_LASTYEAR);
    
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
    
    IORegistryEntry *efi = IORegistryEntry::fromPath("/efi", gIODTPlane);
    OSData *vendor = efi ? OSDynamicCast(OSData, efi->getProperty("firmware-vendor")) : NULL;
    
    if (vendor && vendor->getLength() == 12 && 0 == memcmp(vendor->getBytesNoCopy(), "A\0p\0p\0l\0e\0\0\0", 12) ) {
        HWSensorsFatalLog("forbidding run on Apple hardware");
        return false;
    }
    
	if (!smcDevice->init(provider, OSDynamicCast(OSDictionary, getProperty("Configuration")))) {
        HWSensorsInfoLog("failed to initialize SMC device");
		return false;
    }

    smcDevice->registerService();
	registerService();
    
    // Load keys from NVRAM
    IORegistryEntry* nvram = NULL;
    
    if (vendor) {
        if (vendor->getLength() == 14 && 0 == memcmp(vendor->getBytesNoCopy(), "C\0L\0O\0V\0E\0R\0\0\0", 14) ) {
            // System booted with Clover
            if (OSDictionary *matching = serviceMatching("IORegistryEntry")) {
                nvram = OSDynamicCast(IORegistryEntry, waitForMatchingService(matching));
                OSSafeRelease(matching);
            }
        }
        else /*if (vendor->getLength() == 18 && 0 == memcmp(vendor->getBytesNoCopy(), "C\0h\0a\0m\0e\0l\0e\0o\0n\0", 18))*/ {
            // System booted with chameleon bootloader
            nvram = OSDynamicCast(IORegistryEntry, IORegistryEntry::fromPath("/chosen/nvram", gIODTPlane));
        }
    }
    
    if (nvram) {
        if (OSData *keys = OSDynamicCast(OSData, nvram->getProperty(kFakeSMCPropertyKeys))) {
            
            int count = 0;
            unsigned int offset = 0;
            
            while (offset + 9 < keys->getLength()) {
                char name[5]; memcpy(name, keys->getBytesNoCopy(offset, 4), 4); name[4] = '\0'; offset += 4;
                char type[5]; memcpy(type, keys->getBytesNoCopy(offset, 4), 4); type[4] = '\0'; offset += 4;
                unsigned char size = 0; memcpy(&size, keys->getBytesNoCopy(offset, 1), 1); offset++;
                const void *value = keys->getBytesNoCopy(offset, size); offset += size;
                
                if (smcDevice->addKeyWithValue(name, type, size, value)) {
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