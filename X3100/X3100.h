/*
 *  X3100.h
 *  HWSensors
 *
 *  Created by Sergey on 19.12.10 with templates of Mozodojo.
 *  Copyright 2010 Slice.
 *
 */

#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>

#include "FakeSMCPlugin.h"

class X3100monitor : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(X3100monitor) 
    
private:
	OSDictionary *		sensors;
	volatile UInt8*     mmio_base;
	int					numCard;  //numCard=0 if only one Video, but may be any other value
	IOPCIDevice *		VCard;
	IOMemoryMap *		mmio;
	
	bool				addSensor(const char* key, const char* type, unsigned char size, int index);
	
public:
    virtual bool		start(IOService *provider);
	virtual bool		init(OSDictionary *properties=0);
	virtual void		free(void);
	virtual void		stop(IOService *provider);
	
	virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};
