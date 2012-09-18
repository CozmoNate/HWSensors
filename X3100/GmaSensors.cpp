/*
 *  X3100.cpp
 *  HWSensors
 *
 *  Created by Sergey on 19.12.10.
 *  Copyright 2010 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 */

#include "GmaSensors.h"
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

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(GmaSensors, FakeSMCPlugin)

float GmaSensors::getSensorValue(FakeSMCSensor *sensor)
{    
    if (sensor->getGroup() == kFakeSMCTemperatureSensor) {
        short value = 0;
        
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
        
        return 150 - value;
    }
    
    return 0;
}

IOService* GmaSensors::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) 
        return 0;
	
    if (!(VCard = (IOPCIDevice*)provider))
        return 0;
    
	IOPhysicalAddress bar = (IOPhysicalAddress)((VCard->configRead32(kMCHBAR)) & ~0xf);
    
	HWSensorsDebugLog("Fx3100: register space=%08lx", (long unsigned int)bar);
	
	if(IOMemoryDescriptor * theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x2000, kIODirectionOutIn)) {
		if ((mmio = theDescriptor->map())) {
            
			mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();

			/*HWSensorsDebugLog("MCHBAR mapped");
            
			for (int i = 0; i < 0x2f; i += 16) {
				HWSensorsDebugLog("%04lx: ", (long unsigned int)i+0x1000);
				for (int j=0; j<16; j += 1) {
					HWSensorsDebugLog("%02lx ", (long unsigned int)INVID8(i+j+0x1000));
				}
            HWSensorsDebugLog("");
			}*/
	
		}
		else {
            HWSensorsInfoLog("MCHBAR failed to map");
            return 0;
        }
    }
    
	return this;
}

bool GmaSensors::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;
	
	//Find card number
    SInt8 cardIndex = getVacantGPUIndex();
    
    if (cardIndex < 0) {
        HWSensorsWarningLog("failed to obtain vacant GPU index");
        return false;
    }
    
    char key[5];
    
    snprintf(key, 5, KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE, cardIndex);
    
    if (!addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0)) {
        HWSensorsWarningLog("failed to register temperature sensor");
        return false;
    }
    
    registerService();
	
	return true;	
}