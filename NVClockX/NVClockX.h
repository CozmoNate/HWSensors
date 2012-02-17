/*
 *  NVClockX.h
 *  HWSensors
 *
 *  Created by mozo on 15/10/10.
 *  Copyright 2010 usr-sse2. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include "FakeSMCPlugin.h"

#define kGenericPCIDevice "IOPCIDevice"
#define kNVGraphicsDevice "IONDRVDevice"

//NVClock nvclock;
//NVCard* nv_card;

class NVClockX : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(NVClockX)    
	
private:
	OSDictionary *	sensors;
	
	IOMemoryMap *	nvio;
	
	bool			addSensor(const char* key, const char* type, unsigned char size, int index);
	int				addTachometer(int index);
	
public:
	virtual bool		init(OSDictionary *properties=0);
    virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual void		stop(IOService *provider);
	virtual void		free(void);
	
	virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
	
};