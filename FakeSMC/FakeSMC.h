#ifndef _VIRTUALSMC_H
#define _VIRTUALSMC_H

#include "FakeSMCKeyStore.h"

#include <IOKit/IOService.h>

class FakeSMCDevice;

class FakeSMC : public IOService
{
	OSDeclareDefaultStructors(FakeSMC)
	
private:
    FakeSMCKeyStore     *keyStore;
    FakeSMCDevice       *smcDevice;

public:
    virtual bool		init(OSDictionary *dictionary = 0);
    virtual bool		start(IOService *provider);
    
};

#endif