/*
 *  X3100.h
 *  HWSensors
 *
 *  Created by Sergey on 19.12.10 with templates of Natan Zalkin <natan.zalkin@me.com>.
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
	IOPCIDevice *		VCard;
	IOMemoryMap *		mmio;
	
protected:	
    virtual float       getSensorValue(FakeSMCSensor *sensor);
	
public:
    IOService*          probe(IOService *provider, SInt32 *score);
    bool                start(IOService * provider);
};
