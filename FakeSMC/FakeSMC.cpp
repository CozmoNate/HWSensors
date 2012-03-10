
#include "FakeSMC.h"
#include "FakeSMCDefinitions.h"

#define Debug FALSE

#define LogPrefix "FakeSMC: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super IOService
OSDefineMetaClassAndStructors (FakeSMC, IOService)

bool FakeSMC::init(OSDictionary *dictionary)
{	
	if (!super::init(dictionary))
		return false;
    
    InfoLog("Opensource SMC device emulator. Copyright 2009 netkas. All rights reserved.");
	InfoLog("Supports hardware monitoring plugins. Copyright 2012 kozlek, usr-sse2, slice. All rights reserved.");
    
    if (!(smcDevice = new FakeSMCDevice)) {
		InfoLog("failed to create SMC device");
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
		InfoLog("failed to initialize SMC device");
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