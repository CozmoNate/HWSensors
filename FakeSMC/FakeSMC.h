#ifndef _VIRTUALSMC_H
#define _VIRTUALSMC_H

#include "FakeSMCDevice.h"

#include <IOKit/IOService.h>

class FakeSMC : public IOService
{
	OSDeclareDefaultStructors(FakeSMC)
	
private:
	FakeSMCDevice		*smcDevice;
	
public:
    virtual bool		init(OSDictionary *dictionary = 0);
    virtual void		free(void);
    virtual IOService	*probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
    virtual void		stop(IOService *provider);
};

#endif