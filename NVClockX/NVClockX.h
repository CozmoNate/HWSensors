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

#define kNVCLockBoardTemperatureSensor  1201
#define kNVCLockDiodeTemperatureSensor  1202

class NVClockX : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(NVClockX)    
	
private:
	IOMemoryMap *       nvio;

protected:
    virtual float       getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
};