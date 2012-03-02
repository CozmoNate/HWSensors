/*
 *  X3100.cpp
 *  HWSensors
 *
 *  Created by Sergey on 19.12.10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "X3100.h"
#include "FakeSMCDefinitions.h"

/*#define kGenericPCIDevice "IOPCIDevice"
#define kTimeoutMSecs 1000
#define fVendor "vendor-id"
#define fDevice "device-id"
#define kIOPCIConfigBaseAddress0 0x10*/
#define kMCHBAR	0x48
#define TSC1	0x1001
#define TSS1	0x1004
#define TR1		0x1006
#define RTR1	0x1008
#define TIC1	0x100B
#define TSC2	0x1041
#define TSS2	0x1044
#define TR2		0x1046
#define RTR2	0x1048
#define TIC2	0x104B


#define INVID8(offset) (mmio_base[offset])
#define INVID16(offset) OSReadLittleInt16((mmio_base), offset)
#define INVID(offset) OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((mmio_base), offset, val)

#define Debug FALSE

#define LogPrefix "X3100monitor: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(X3100monitor, FakeSMCPlugin)

float X3100monitor::getSensorValue(FakeSMCSensor *sensor)
{    
    if (sensor->getGroup() == kFakeSMCTemperatureSensor) {
        short value;
        
        if (mmio_base) {
            
            OUTVID(TIC1, 3);
            
            //		if ((INVID16(TSC1) & (1<<15)) && !(INVID16(TSC1) & (1<<8)))//enabled and ready
            for (int i=0; i<1000; i++) {  //attempts to ready
                
                if (INVID16(TSS1) & (1<<10))   //valid?
                    break;
                
                IOSleep(10);
            }	
			
            value = INVID8(TR1);
        }				
        
        return 140 - value;
    }
    
    return 0;
}

IOService* X3100monitor::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) 
        return 0;
	
    VCard = (IOPCIDevice*)provider;
    
    if (!VCard) 
        return this;
    
	IOPhysicalAddress bar = (IOPhysicalAddress)((VCard->configRead32(kMCHBAR)) & ~0xf);
    
	DebugLog("Fx3100: register space=%08lx", (long unsigned int)bar);
    
	IOMemoryDescriptor * theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x2000, kIODirectionOutIn); // | kIOMapInhibitCache);
    
	if(theDescriptor != NULL)
	{
		mmio = theDescriptor->map();
        
		if (mmio != NULL)
		{
			mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
#if DEBUG				
			DebugLog("MCHBAR mapped");
            
			for (int i=0; i<0x2f; i +=16) {
				DebugLog("%04lx: ", (long unsigned int)i+0x1000);
				for (int j=0; j<16; j += 1) {
					DebugLog("%02lx ", (long unsigned int)INVID8(i+j+0x1000));
				}
				DebugLog("");
			}
#endif				
		}
		else
		{
			InfoLog(" MCHBAR failed to map");
			return this;
		}			
	}	
    
	isActive = true;
    
	return this;
}

bool X3100monitor::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;
	
	char name[5];
    
    for (UInt8 i = 0; i < 0xf; i++) {
        snprintf(name, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, i); 
        
        IOService *handler = 0;
        
        if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCGetKeyHandler, true, (void *)name, (void *)&handler, 0, 0)) {
            snprintf(name, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, i); 
            
            if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCGetKeyHandler, true, (void *)name, (void *)&handler, 0, 0)) {
                if (!addSensor(name, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0))
                    WarningLog("Can't add temperature sensor for key %s", name);
                break;
            }
        }
    }
    
    registerService(0);
	
	return true;	
}