
#include "FakeSMC.h"
#include "FakeSMCDefinitions.h"
#include "HWSensorsVersion.h"

#include "OEMInfo.h"

#define super IOService
OSDefineMetaClassAndStructors (FakeSMC, IOService)

bool FakeSMC::init(OSDictionary *dictionary)
{	
	if (!super::init(dictionary))
		return false;
    
    IOLog("HWSensors Project Copyright %d netkas, slice, usr-sse2, kozlek, navi, THe KiNG. All rights reserved.\n",HWSENSORS_LASTYEAR);
    
    //HWSensorsInfoLog("Opensource SMC device emulator. Copyright 2009 netkas. All rights reserved.");
    
    if (!(smcDevice = new FakeSMCDevice)) {
		HWSensorsInfoLog("failed to create SMC device");
		return false;
	}
    
    if (!setOemProperties(this))
        HWSensorsWarningLog("failed to read OEM data, specific OEM configuration will be unavailable");
		
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
	
	if (!smcDevice->init(provider, OSDynamicCast(OSDictionary, getProperty("Configuration")))) {
		HWSensorsInfoLog("failed to initialize SMC device");
		return false;
	}

	smcDevice->registerService();
    
	registerService();
		
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