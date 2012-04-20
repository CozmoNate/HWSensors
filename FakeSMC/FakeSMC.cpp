
#include "FakeSMC.h"
#include "FakeSMCDefinitions.h"

#define super IOService
OSDefineMetaClassAndStructors (FakeSMC, IOService)

bool FakeSMC::init(OSDictionary *dictionary)
{	
	if (!super::init(dictionary))
		return false;
    
    HWSensorsInfoLog("Opensource SMC device emulator. Copyright 2009 netkas. All rights reserved.");
	HWSensorsInfoLog("Hardware monitoring plugins. Copyright 2012 kozlek, usr-sse2, slice, navi. All rights reserved.");
    
    if (!(smcDevice = new FakeSMCDevice)) {
		HWSensorsInfoLog("failed to create SMC device");
		return false;
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