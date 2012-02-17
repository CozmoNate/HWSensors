#ifndef _VIRTUALSMC_H
#define _VIRTUALSMC_H

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "FakeSMCDevice.h"
#include "FakeSMCKey.h"
#include "FakeSMCDefinitions.h"


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