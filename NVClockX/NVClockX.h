/*
 *  NVClockX.h
 *  HWSensors
 *
 *  Created by mozo on 15/10/10.
 *  Copyright 2010 usr-sse2. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>

#include "FakeSMCPlugin.h"

#define kGenericPCIDevice "IOPCIDevice"
#define kNVGraphicsDevice "IONDRVDevice"

#define kNVCLockBoardTemperatureSensor  1201
#define kNVCLockDiodeTemperatureSensor  1202

#define kNVCLockCoreFrequencySensor     1302
#define kNVCLockMemoryFrequencySensor   1303
#define kNVCLockShaderFrequencySensor   1304

class NVClockX : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(NVClockX)    
	
private:
    IOPCIDevice         *videoCard;

protected:
    virtual float       getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
};