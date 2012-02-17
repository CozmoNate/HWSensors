//
//  FakeSMCPlugin.cpp
//  HWSensors
//
//  Created by mozo on 11/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#include "FakeSMCPlugin.h"

#include <IOKit/IOLib.h>
#include "FakeSMCDefinitions.h"

#define Debug FALSE

#define LogPrefix "FakeSMCPlugin: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)


#define super IOService
OSDefineMetaClassAndAbstractStructors(FakeSMCPlugin, IOService)

bool FakeSMCPlugin::init(OSDictionary *properties)
{
	DebugLog("initialising...");
	
    isActive = false;
    
	return super::init(properties);
}

IOService * FakeSMCPlugin::probe(IOService *provider, SInt32 *score)
{
	DebugLog("probing...");
	
	if (super::probe(provider, score) != this) 
		return 0;
    
    return this;
}

bool FakeSMCPlugin::start(IOService *provider)
{		
	DebugLog("starting...");
	
	if (!super::start(provider)) 
        return false;
    
	if (!(fakeSMC = waitForService(serviceMatching(kFakeSMCDeviceService)))) {
		WarningLog("can't locate fake SMC device");
		return false;
	}
	
	return true;
}

void FakeSMCPlugin::stop(IOService* provider)
{
	DebugLog("stoping...");
    
    fakeSMC->callPlatformFunction(kFakeSMCRemoveHandler, true, this, NULL, NULL, NULL);
	
	super::stop(provider);
}

void FakeSMCPlugin::free()
{
	DebugLog("freeing...");
	
	super::free();
}

IOReturn FakeSMCPlugin::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
    return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}