#include "FakeSMC.h"

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
		
	return true;
}

IOService *FakeSMC::probe(IOService *provider, SInt32 *score)
{
    if (!super::probe(provider, score))
		return 0;
	
	InfoLog("opensource SMC device emulator by netkas (C) 2009");
	InfoLog("plugins & plugins support modifications by mozodojo, usr-sse2, slice (C) 2010");
	
	return this;
}

bool FakeSMC::start(IOService *provider)
{
	if (!super::start(provider)) return false;
			
	if (!(smcDevice = new FakeSMCDevice)) {
		InfoLog("failed to create smcDevice");
		return false;
	}
		
	if (!smcDevice->init(provider, OSDynamicCast(OSDictionary, getProperty("Configuration")))) {
		InfoLog("failed to init smcDevice");
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