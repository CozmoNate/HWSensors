/*
 *  X3100.cpp
 *  HWSensors
 *
 *  Created by Sergey on 19.12.10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "X3100.h"
#include "FakeSMC.h"

#define kGenericPCIDevice "IOPCIDevice"
#define kTimeoutMSecs 1000
#define fVendor "vendor-id"
#define fDevice "device-id"
#define kIOPCIConfigBaseAddress0 0x10
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

bool X3100monitor::addSensor(const char* key, const char* type, unsigned char size, int index)
{
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)key, (void *)type, (void *)size, (void *)this))
		return sensors->setObject(key, OSNumber::withNumber(index, 32));	
	return false;
}

bool X3100monitor::init(OSDictionary *properties)
{
    if (!super::init(properties))
		return false;
	
	if (!(sensors = OSDictionary::withCapacity(0)))
		return false;
	
	return true;
}

/*IOService* X3100monitor::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	UInt32 vendor_id, device_id;
	if (OSDictionary * dictionary = serviceMatching(kGenericPCIDevice)) {
		if (OSIterator * iterator = getMatchingServices(dictionary)) {
			
			IOPCIDevice* device = 0;
			
			while (device = OSDynamicCast(IOPCIDevice, iterator->getNextObject())) {
				OSData *data = OSDynamicCast(OSData, device->getProperty("vendor-id"));
				if (data)
					vendor_id = *(UInt32*)data->getBytesNoCopy();
				
				data = OSDynamicCast(OSData, device->getProperty("device-id"));				
				if (data)
					device_id = *(UInt32*)data->getBytesNoCopy();
				
				if ((vendor_id==0x8086) && (device_id==0x2a00)){
					InfoLog("found %lx chip", (long unsigned int)device_id);
					VCard = device;
				}
			}
		}
	}	
	return this;
}*/

bool X3100monitor::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;
    
    VCard = (IOPCIDevice*)provider;
    
    if (!VCard) 
        return false;

	IOMemoryDescriptor * theDescriptor;
	IOPhysicalAddress bar = (IOPhysicalAddress)((VCard->configRead32(kMCHBAR)) & ~0xf);
	DebugLog("Fx3100: register space=%08lx\n", (long unsigned int)bar);
	theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x2000, kIODirectionOutIn); // | kIOMapInhibitCache);
	if(theDescriptor != NULL)
	{
		mmio = theDescriptor->map();
		if(mmio != NULL)
		{
			mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
#if DEBUG				
			DebugLog(" MCHBAR mapped\n");
			for (int i=0; i<0x2f; i +=16) {
				DebugLog("%04lx: ", (long unsigned int)i+0x1000);
				for (int j=0; j<16; j += 1) {
					DebugLog("%02lx ", (long unsigned int)INVID8(i+j+0x1000));
				}
				DebugLog("\n");
			}
#endif				
		}
		else
		{
			InfoLog(" MCHBAR failed to map\n");
			return -1;
		}			
	}	
	
	char name[5];
	//try to find empty key
	for (int i = 0; i < 0x10; i++) {
						
		snprintf(name, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, i); 
			
		UInt8 length = 0;
		void * data = 0;
			
		IOReturn result = fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)name, (void *)&length, (void *)&data, 0);
			
		if (kIOReturnSuccess == result) {
			continue;
		}
		if (addSensor(name, TYPE_SP78, 2, i)) {
			numCard = i;
			break;
		}
	}
		
	if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)name, (void *)TYPE_SP78, (void *)2, this)) {
		WarningLog("Can't add key to fake SMC device, kext will not load");
		return false;
	}
    
    registerService(0);
	
	return true;	
}

void X3100monitor::stop (IOService* provider)
{
    super::stop(provider);
    
	sensors->flushCollection();
}

void X3100monitor::free ()
{
	sensors->release();
	
	super::free();
}

IOReturn X3100monitor::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	UInt16 t;

	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void* data = param2;
		
		if (name && data) {
			if (OSNumber *number = OSDynamicCast(OSNumber, sensors->getObject(name))) {				
				UInt32 index = number->unsigned16BitValue();
				if (index != numCard) {
					return kIOReturnBadArgument;
				}
			}
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
			
			t = 140 - value;
			bcopy(&t, data, 2);
			
			return kIOReturnSuccess;
		}
		
		//DebugLog("bad argument key name or data");
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
