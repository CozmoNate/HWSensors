/*
 *  Radeon.h
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>
#include "ATICard.h"
#include "FakeSMCPlugin.h"

class RadeonMonitor : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(RadeonMonitor)    
	
private:
	OSDictionary*		sensors;    
	int					numCard;  //numCard=0 if only one Video, but may be any other value

	bool				addSensor(const char* key, const char* type, unsigned char size, int index);
    
protected:	
	ATICard*			Card; 
	
public:
	virtual bool		init(OSDictionary *properties);
    virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual void		stop(IOService *provider);
   	virtual void		free(void);
	
	virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};
