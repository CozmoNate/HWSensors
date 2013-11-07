/*
 *  FakeSMCDevice.cpp
 *  FakeSMC
 *
 *  Created by Vladimir on 20.08.09.
 *  Copyright 2009 netkas. All rights reserved.
 *
 */

#include "FakeSMCKey.h"
#include "FakeSMC.h"
#include "FakeSMCDevice.h"
#include "FakeSMCPlugin.h"
#include "FakeSMCDefinitions.h"

#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOKitKeys.h>

#define FakeSMCTraceLog(string, args...) do { if (trace) { IOLog ("%s: [Trace] " string "\n",getName() , ## args); } } while(0)
#define FakeSMCDebugLog(string, args...) do { if (debug) { IOLog ("%s: [Debug] " string "\n",getName() , ## args); } } while(0)

#define FakeSMCSetProperty(key, value)	do { if (!this->setProperty(key, value)) {HWSensorsErrorLog("failed to set '%s' property", key); return false; } } while(0)

#define super IOACPIPlatformDevice
OSDefineMetaClassAndStructors (FakeSMCDevice, IOACPIPlatformDevice)

#pragma mark -
#pragma mark Internal I/O methods

void FakeSMCDevice::applesmc_io_cmd_writeb(void *opaque, uint32_t addr, uint32_t val)
{
    struct AppleSMCStatus *s = (struct AppleSMCStatus *)opaque;
    
    FakeSMCTraceLog("CMD Write B: %#x = %#x", addr, val);
    
    switch(val) {
        case APPLESMC_READ_CMD:
            s->status = 0x0c;
            break;
		case APPLESMC_WRITE_CMD:
            s->status = 0x0c;
			break;
		case APPLESMC_GET_KEY_BY_INDEX_CMD:
			s->status = 0x0c;
			break;
		case APPLESMC_GET_KEY_TYPE_CMD:
			s->status = 0x0c;
			break;
    }
    s->cmd = val;
    s->read_pos = 0;
    s->data_pos = 0;
	s->key_index = 0;
    //	bzero(s->key_info, 6);
}

void FakeSMCDevice::applesmc_fill_data(struct AppleSMCStatus *s)
{
	if (FakeSMCKey *key = keyStore->getKey((char*)s->key)) {
		bcopy(key->getValue(), s->value, key->getSize());
		return;
	}
    
    FakeSMCTraceLog("key not found %c%c%c%c, length - %x\n", s->key[0], s->key[1], s->key[2], s->key[3],  s->data_len);
    
	s->status_1e=0x84;
}

const char * FakeSMCDevice::applesmc_get_key_by_index(uint32_t index, struct AppleSMCStatus *s)
{
	if (FakeSMCKey *key = keyStore->getKey(index))
		return key->getKey();
    
    FakeSMCTraceLog("key by count %x is not found",index);
    
	s->status_1e=0x84;
	s->status = 0x00;
    
	return 0;
}

void FakeSMCDevice::applesmc_fill_info(struct AppleSMCStatus *s)
{
	if (FakeSMCKey *key = keyStore->getKey((char*)s->key)) {
		s->key_info[0] = key->getSize();
		s->key_info[5] = 0;
        
		const char* typ = key->getType();
		UInt64 len = strlen(typ);
        
		for (UInt8 i=0; i<4; i++)
		{
			if (i<len)
			{
				s->key_info[i+1] = typ[i];
			}
			else
			{
				s->key_info[i+1] = 0;
			}
		}
        
		return;
	}
    
	FakeSMCTraceLog("key info not found %c%c%c%c, length - %x", s->key[0], s->key[1], s->key[2], s->key[3],  s->data_len);
    
	s->status_1e=0x84;
}

void FakeSMCDevice::applesmc_io_data_writeb(void *opaque, uint32_t addr, uint32_t val)
{
    struct AppleSMCStatus *s = (struct AppleSMCStatus *)opaque;
    //    IOLog("APPLESMC: DATA Write B: %#x = %#x\n", addr, val);
    switch(s->cmd) {
        case APPLESMC_READ_CMD:
            if(s->read_pos < 4) {
                s->key[s->read_pos] = val;
                s->status = 0x04;
            } else if(s->read_pos == 4) {
                s->data_len = val;
                s->status = 0x05;
                s->data_pos = 0;
                //                IOLog("APPLESMC: Key = %c%c%c%c Len = %d\n", s->key[0], s->key[1], s->key[2], s->key[3], val);
                applesmc_fill_data(s);
            }
            s->read_pos++;
            break;
		case APPLESMC_WRITE_CMD:
            //			IOLog("FakeSMC: attempting to write(WRITE_CMD) to io port value %x ( %c )\n", val, val);
			if(s->read_pos < 4) {
                s->key[s->read_pos] = val;
                s->status = 0x04;
			} else if(s->read_pos == 4) {
				s->status = 0x05;
				s->data_pos=0;
				s->data_len = val;
                //				IOLog("FakeSMC: System Tried to write Key = %c%c%c%c Len = %d\n", s->key[0], s->key[1], s->key[2], s->key[3], val);
			} else if( s->data_pos < s->data_len ) {
				s->value[s->data_pos] = val;
				s->data_pos++;
				s->status = 0x05;
				if(s->data_pos == s->data_len) {
					s->status = 0x00;
                    
                    // Add or update key
                    char name[5]; name[4] = 0; memcpy(name, s->key, 4);
                    
                    FakeSMCDebugLog("system writing key %s, length %d", name, s->data_len);
                    
                    FakeSMCKey* key = keyStore->addKeyWithValue(name, 0, s->data_len, s->value);

                    bzero(s->value, 255);
                    
                    if (key) keyStore->saveKeyToNVRAM(key);
				}
			};
			s->read_pos++;
			break;
		case APPLESMC_GET_KEY_BY_INDEX_CMD:
            //			IOLog("FakeSMC: System Tried to write GETKEYBYINDEX = %x (%c) at pos %x\n",val , val, s->read_pos);
			if(s->read_pos < 4) {
                s->key_index += val << (24 - s->read_pos * 8);
                s->status = 0x04;
				s->read_pos++;
			};
			if(s->read_pos == 4) {
				s->status = 0x05;
                //				IOLog("FakeSMC: trying to find key by index %x\n", s->key_index);
				if(const char * key = applesmc_get_key_by_index(s->key_index, s))
					bcopy(key, s->key, 4);
			}
            
			break;
		case APPLESMC_GET_KEY_TYPE_CMD:
            //			IOLog("FakeSMC: System Tried to write GETKEYTYPE = %x (%c) at pos %x\n",val , val, s->read_pos);
			if(s->read_pos < 4) {
                s->key[s->read_pos] = val;
                s->status = 0x04;
            };
			s->read_pos++;
			if(s->read_pos == 4) {
				s->data_len = 6;  ///s->data_len = val ; ? val should be 6 here too
				s->status = 0x05;
				s->data_pos=0;
				applesmc_fill_info(s);
			}
			break;
    }
}

uint32_t FakeSMCDevice::applesmc_io_data_readb(void *opaque, uint32_t addr1)
{
    struct AppleSMCStatus *s = (struct AppleSMCStatus *)opaque;
    uint8_t retval = 0;
    switch(s->cmd) {
        case APPLESMC_READ_CMD:
            if(s->data_pos < s->data_len) {
                retval = s->value[s->data_pos];
                //			        IOLog("APPLESMC: READ_DATA[%d] = %#hhx\n", s->data_pos, retval);
                s->data_pos++;
                if(s->data_pos == s->data_len) {
                    s->status = 0x00;
                    bzero(s->value, 255);
                    //			            IOLog("APPLESMC: EOF\n");
                } else
                    s->status = 0x05;
            }
            break;
        case APPLESMC_WRITE_CMD:
            //				HWSensorsInfoLog("attempting to read(WRITE_CMD) from io port");
            s->status = 0x00;
            break;
        case APPLESMC_GET_KEY_BY_INDEX_CMD:  ///shouldnt be here if status == 0
            //				IOLog("FakeSMC:System Tried to read GETKEYBYINDEX = %x (%c) , at pos %d\n", retval, s->key[s->data_pos], s->key[s->data_pos], s->data_pos);
            if(s->status == 0) return 0; //sanity check
            if(s->data_pos < 4) {
                retval = s->key[s->data_pos];
                s->data_pos++;
            }
            if (s->data_pos == 4)
                s->status = 0x00;
            break;
        case APPLESMC_GET_KEY_TYPE_CMD:
            //				IOLog("FakeSMC:System Tried to read GETKEYTYPE = %x , at pos %d\n", s->key_info[s->data_pos], s->data_pos);
            if(s->data_pos < s->data_len) {
                retval = s->key_info[s->data_pos];
                s->data_pos++;
                if(s->data_pos == s->data_len) {
                    s->status = 0x00;
                    bzero(s->key_info, 6);
                    //			            IOLog("APPLESMC: EOF\n");
                } else
                    s->status = 0x05;
            }
            break;
            
    }
    //    IOLog("APPLESMC: DATA Read b: %#x = %#x\n", addr1, retval);
    return retval;
}

uint32_t FakeSMCDevice::applesmc_io_cmd_readb(void *opaque, uint32_t addr1)
{
    //		IOLog("APPLESMC: CMD Read B: %#x\n", addr1);
    return ((struct AppleSMCStatus*)opaque)->status;
}

#pragma mark -
#pragma mark Custom init method

bool FakeSMCDevice::initAndStart(IOService *platform, IOService *provider)
{
	if (!provider || !super::init(platform, 0, 0))
		return false;

    if (!(keyStore = OSDynamicCast(FakeSMCKeyStore, waitForMatchingService(serviceMatching(kFakeSMCKeyStoreService), kFakeSMCDefaultWaitTimeout)))) {
		HWSensorsFatalLog("failed to locate FakeSMCKeyStore service!");
        return false;
    }
    
	status = (ApleSMCStatus *) IOMalloc(sizeof(struct AppleSMCStatus));
	bzero((void*)status, sizeof(struct AppleSMCStatus));
    
	interrupt_handler = 0;
    
    // Start SMC device
    
    if (!super::start(platform))
        return false;

    OSDictionary *properties = OSDynamicCast(OSDictionary, provider->getProperty("Configuration"));

    if (!properties)
        return false;
    
	this->setName("SMC");
    
    FakeSMCSetProperty("name", "APP0001");
    
	if (OSString *compatibleKey = OSDynamicCast(OSString, properties->getObject("smc-compatible")))
		FakeSMCSetProperty("compatible", (const char *)compatibleKey->getCStringNoCopy());
	else
		FakeSMCSetProperty("compatible", "smc-napa");
    
	if (!this->setProperty("_STA", (unsigned long long)0x0000000b, 32)) {
        HWSensorsErrorLog("failed to set '_STA' property");
        return false;
    }
    
	if (OSBoolean *debugKey = OSDynamicCast(OSBoolean, properties->getObject("debug")))
		debug = debugKey->getValue();
    else
        debug = false;
    
    if (OSBoolean *traceKey = OSDynamicCast(OSBoolean, properties->getObject("trace")))
		trace = traceKey->getValue();
    else
        trace = false;
    
	IODeviceMemory::InitElement	rangeList[1];
    
	rangeList[0].start = 0x300;
	rangeList[0].length = 0x20;
//    rangeList[1].start = 0xfef00000;
//	rangeList[1].length = 0x10000;
    
	if(OSArray *array = IODeviceMemory::arrayFromList(rangeList, 1)) {
		this->setDeviceMemory(array);
		OSSafeRelease(array);
	}
	else
	{
		HWSensorsFatalLog("failed to create Device memory array");
		return false;
	}
    
	OSArray *controllers = OSArray::withCapacity(1);
    
    if(!controllers) {
		HWSensorsFatalLog("failed to create controllers array");
        return false;
    }
    
    controllers->setObject((OSSymbol *)OSSymbol::withCStringNoCopy("io-apic-0"));
    
	OSArray *specifiers  = OSArray::withCapacity(1);
    
    if(!specifiers) {
		HWSensorsFatalLog("failed to create specifiers array");
        return false;
    }
    
	UInt64 line = 0x06;
    
    OSData *tmpData = OSData::withBytes(&line, sizeof(line));
    
    if (!tmpData) {
		HWSensorsFatalLog("failed to create specifiers data");
        return false;
    }
    
    specifiers->setObject(tmpData);
    
	this->setProperty(gIOInterruptControllersKey, controllers) && this->setProperty(gIOInterruptSpecifiersKey, specifiers);
	this->attachToParent(platform, gIOServicePlane);

    registerService();
    
	HWSensorsInfoLog("successfully initialized");
    
	return true;
}


#pragma mark -
#pragma mark Virtual methods

UInt32 FakeSMCDevice::ioRead32( UInt16 offset, IOMemoryMap * map )
{
    UInt32  value=0;
    UInt16  base = 0;
    
    if (map) base = map->getPhysicalAddress();
    
	//HWSensorsDebugLog("ioread32 called");
    
    return (value);
}

UInt16 FakeSMCDevice::ioRead16( UInt16 offset, IOMemoryMap * map )
{
    UInt16  value=0;
    UInt16  base = 0;
    
    if (map) base = map->getPhysicalAddress();
    
	//HWSensorsDebugLog("ioread16 called");
    
    return (value);
}

UInt8 FakeSMCDevice::ioRead8( UInt16 offset, IOMemoryMap * map )
{
    UInt8  value =0;
    UInt16  base = 0;
	struct AppleSMCStatus *s = (struct AppleSMCStatus *)status;
    //	IODelay(10);
    
    if (map) base = map->getPhysicalAddress();
	if((base+offset) == APPLESMC_DATA_PORT) value=applesmc_io_data_readb(status, base+offset);
	if((base+offset) == APPLESMC_CMD_PORT) value=applesmc_io_cmd_readb(status, base+offset);
    
    if((base+offset) == APPLESMC_ERROR_CODE_PORT)
	{
		if(s->status_1e != 0)
		{
			value = s->status_1e;
			s->status_1e = 0x00;
            //			IOLog("generating error %x\n", value);
		}
		else value = 0x0;
	}
    //	if(((base+offset) != APPLESMC_DATA_PORT) && ((base+offset) != APPLESMC_CMD_PORT)) IOLog("ioread8 to port %x.\n", base+offset);
    
	//HWSensorsDebugLog("ioread8 called");
    
	return (value);
}

void FakeSMCDevice::ioWrite32( UInt16 offset, UInt32 value, IOMemoryMap * map )
{
    UInt16 base = 0;
    
    if (map) base = map->getPhysicalAddress();
    
	//HWSensorsDebugLog("iowrite32 called");
}

void FakeSMCDevice::ioWrite16( UInt16 offset, UInt16 value, IOMemoryMap * map )
{
    UInt16 base = 0;
    
    if (map) base = map->getPhysicalAddress();
    
	//HWSensorsDebugLog("iowrite16 called");
}

void FakeSMCDevice::ioWrite8( UInt16 offset, UInt8 value, IOMemoryMap * map )
{
    UInt16 base = 0;
	IODelay(10);
    if (map) base = map->getPhysicalAddress();
    
	if((base+offset) == APPLESMC_DATA_PORT) applesmc_io_data_writeb(status, base+offset, value);
	if((base+offset) == APPLESMC_CMD_PORT) applesmc_io_cmd_writeb(status, base+offset,value);
	//    outb( base + offset, value );
    //	if(((base+offset) != APPLESMC_DATA_PORT) && ((base+offset) != APPLESMC_CMD_PORT)) IOLog("iowrite8 to port %x.\n", base+offset);
    
	//HWSensorsDebugLog("iowrite8 called");
}

IOReturn FakeSMCDevice::registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refCon)
{
	interrupt_refcon = refCon;
	interrupt_target = target;
	interrupt_handler = handler;
	interrupt_source = source;
    //	IOLog("register interrupt called for source %x\n", source);
	return kIOReturnSuccess;
}

IOReturn FakeSMCDevice::unregisterInterrupt(int source)
{
	return kIOReturnSuccess;
}

IOReturn FakeSMCDevice::getInterruptType(int source, int *interruptType)
{
	return kIOReturnSuccess;
}

IOReturn FakeSMCDevice::enableInterrupt(int source)
{
	return kIOReturnSuccess;
}

IOReturn FakeSMCDevice::disableInterrupt(int source)
{
	return kIOReturnSuccess;
}

IOReturn FakeSMCDevice::causeInterrupt(int source)
{
	if(interrupt_handler)
		interrupt_handler(interrupt_target, interrupt_refcon, this, interrupt_source);
    
	return kIOReturnSuccess;
}