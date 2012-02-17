/*
 *  Radeon.cpp
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "RadeonX.h"


#define kGenericPCIDevice "IOPCIDevice"
#define kTimeoutMSecs 1000
#define fVendor "vendor-id"
#define fATYVendor "ATY,VendorID"
#define fDevice "device-id"
#define fClass	"class-code"
#define kIOPCIConfigBaseAddress0 0x10

#define INVID8(offset)		(mmio_base[offset])
#define INVID16(offset)		OSReadLittleInt16((mmio_base), offset)
#define INVID(offset)		OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val)	OSWriteLittleInt32((mmio_base), offset, val)

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(RadeonMonitor, FakeSMCPlugin)

bool RadeonMonitor::addSensor(const char* key, const char* type, unsigned char size, int index)
{
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)key, (void *)type, (void *)size, (void *)this))
		return sensors->setObject(key, OSNumber::withNumber(index, 32));	
	return false;
}

bool RadeonMonitor::init(OSDictionary *properties)
{
    if (!super::init(properties))
		return false;
	
	if (!(sensors = OSDictionary::withCapacity(0)))
		return false;
    
    Card = new ATICard();
	
	return true;
}

IOService* RadeonMonitor::probe(IOService *provider, SInt32 *score)
{
    if (super::probe(provider, score) != this) 
        return 0;
    
    IOPCIDevice* device = (IOPCIDevice*)provider;
    
    if (!device) 
        return 0;
    
    Card->VCard = device;
    
    return this;
}

bool RadeonMonitor::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;
	
    OSData *data = OSDynamicCast(OSData, provider->getProperty(fDevice));
    
	Card->chipID = data ? *(UInt32*)data->getBytesNoCopy() : 0;	
    
	if(Card->initialize()) {
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
			WarningLog("Can't add key to fake SMC device");
			return false;
		}
        
        registerService(0);
		
		return true;	
	}
	
    return false;
}

void RadeonMonitor::stop (IOService* provider)
{
    super::stop(provider);
    
	sensors->flushCollection();
}

void RadeonMonitor::free ()
{
	sensors->release();
    Card->release();
	
	super::free();
}

IOReturn RadeonMonitor::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	UInt16 t;
	
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void* data = param2;
		
		if (name && data) {
			if (OSNumber *number = OSDynamicCast(OSNumber, sensors->getObject(name))) {				
				UInt32 index = number->unsigned16BitValue();
				if (index != numCard) {  //TODO - multiple card support
					return kIOReturnBadArgument;
				}
			}
			
			switch (Card->tempFamily) {
				case R6xx:
					Card->R6xxTemperatureSensor(&t);
					break;
				case R7xx:
					Card->R7xxTemperatureSensor(&t);
					break;
				case R8xx:
					Card->EverTemperatureSensor(&t);
					break;
				default:
					break;
			}
			//t = Card->tempSensor->readTemp(index);
			bcopy(&t, data, 2);
			
			return kIOReturnSuccess;
		}
		
		//DebugLog("bad argument key name or data");
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
