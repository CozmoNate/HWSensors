/*
 *  FakeSMCDevice.cpp
 *  FakeSMC
 *
 *  Created by Vladimir on 20.08.09.
 *  Copyright 2009 netkas. All rights reserved.
 *
 */

#include "FakeSMCDevice.h"
#include "FakeSMCDefinitions.h"

#define Debug FALSE

#define LogPrefix "FakeSMCDevice: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super IOACPIPlatformDevice
OSDefineMetaClassAndStructors (FakeSMCDevice, IOACPIPlatformDevice)

void FakeSMCDevice::applesmc_io_cmd_writeb(void *opaque, uint32_t addr, uint32_t val)
{
    struct AppleSMCStatus *s = (struct AppleSMCStatus *)opaque;
    //DebugLog("CMD Write B: %#x = %#x", addr, val);
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
	if (FakeSMCKey *key = getKey((char*)s->key)) {
		bcopy(key->getValue(), s->value, key->getSize());
		return;
	}
	
	if(debug) 
		WarningLog("key not found %c%c%c%c, length - %x\n", s->key[0], s->key[1], s->key[2], s->key[3],  s->data_len);
	
	s->status_1e=0x84;
}

const char * FakeSMCDevice::applesmc_get_key_by_index(uint32_t index, struct AppleSMCStatus *s)
{
	if (FakeSMCKey *key = getKey(index))
		return key->getName();
	
	if (debug)
		WarningLog("key by count %x is not found",index);
	
	s->status_1e=0x84;
	s->status = 0x00;
	
	return 0;
}

void FakeSMCDevice::applesmc_fill_info(struct AppleSMCStatus *s)
{
	if (FakeSMCKey *key = getKey((char*)s->key)) {
		s->key_info[0] = key->getSize();
		s->key_info[5] = 0;
		
		const char* typ = key->getType();
		UInt64 len = strlen(typ);
		
		for (int i=0; i<4; i++)
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
		
	if (debug)
		WarningLog("key info not found %c%c%c%c, length - %x", s->key[0], s->key[1], s->key[2], s->key[3],  s->data_len);
	
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
					char name[5];
					
					snprintf(name, 5, "%c%c%c%c", s->key[0], s->key[1], s->key[2], s->key[3]);
					
//					IOLog("FakeSMC: adding Key = %c%c%c%c Len = %d\n", s->key[0], s->key[1], s->key[2], s->key[3], s->data_len);
					addKeyWithValue(name, 0, s->data_len, s->value);
					bzero(s->value, 255);
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
//				InfoLog("attempting to read(WRITE_CMD) from io port");
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

UInt32 FakeSMCDevice::ioRead32( UInt16 offset, IOMemoryMap * map )
{
    UInt32  value=0;
    UInt16  base = 0;
	
    if (map) base = map->getPhysicalAddress();
	
	//DebugLog("ioread32 called");
	
    return (value);
}

UInt16 FakeSMCDevice::ioRead16( UInt16 offset, IOMemoryMap * map )
{
    UInt16  value=0;
    UInt16  base = 0;
	
    if (map) base = map->getPhysicalAddress();
	
	//DebugLog("ioread16 called");

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
	
	//DebugLog("ioread8 called");
	
	return (value);
}

void FakeSMCDevice::ioWrite32( UInt16 offset, UInt32 value, IOMemoryMap * map )
{
    UInt16 base = 0;
	
    if (map) base = map->getPhysicalAddress();
	
	//DebugLog("iowrite32 called");
}

void FakeSMCDevice::ioWrite16( UInt16 offset, UInt16 value, IOMemoryMap * map )
{
    UInt16 base = 0;
	
    if (map) base = map->getPhysicalAddress();
	
	//DebugLog("iowrite16 called");
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
	
	//DebugLog("iowrite8 called");
}

bool FakeSMCDevice::init(IOService *platform, OSDictionary *properties)
{
	if (!super::init(platform, 0, 0))
		return false;
	
	status = (ApleSMCStatus *) IOMalloc(sizeof(struct AppleSMCStatus));
	bzero((void*)status, sizeof(struct AppleSMCStatus));
	
	debug = false;
	interrupt_handler=0;
	
	keys = OSArray::withCapacity(0);
    values = OSDictionary::withCapacity(0);
	
	sharpKEY = FakeSMCKey::withValue("#KEY", "ui8", 4, "\1");
	keys->setObject(sharpKEY);
	
	loadKeysFromDictionary(OSDynamicCast(OSDictionary, properties->getObject("Keys")));
	
	this->setName("SMC");
	
	const char * nodeName = "APP0001";
	this->setProperty("name",(void *)nodeName, (UInt32)strlen(nodeName)+1);
	
	if(OSString *smccomp = OSDynamicCast(OSString, properties->getObject("smc-compatible"))) {
		this->setProperty("compatible",(void *)smccomp->getCStringNoCopy(), smccomp->getLength()+1);
	} else {
		const char * nodeComp = "smc-napa";
		this->setProperty("compatible",(void *)nodeComp, (UInt32)strlen(nodeComp)+1);
	}
	
	this->setProperty("_STA", (unsigned long long)0x0000000b, 32);
	
	if (OSBoolean *debugkey = OSDynamicCast(OSBoolean, properties->getObject("debug")))
		this->setDebug(debugkey->getValue());
	else
		this->setDebug(true);
	
	IODeviceMemory::InitElement	rangeList[1];
	
	rangeList[0].start = 0x300;
	rangeList[0].length = 0x20;
	
	if(OSArray *array = IODeviceMemory::arrayFromList(rangeList, 1)) {
		this->setDeviceMemory(array);
		array->release();
	}
	else
	{
		WarningLog("failed to create Device memory array");
		return false;
	}
	
	OSArray *controllers = OSArray::withCapacity(1);
	if(!controllers)
		WarningLog("failed to create controllers array");
	
	OSArray *specifiers  = OSArray::withCapacity(1);
	if(!specifiers)
		WarningLog("failed to create specifiers array");
	
	UInt64 line = 0x06;
	OSData *tmpData = OSData::withBytes( &line, sizeof(line) );
	if (!tmpData)
		WarningLog("failed to create tmpdata");
	
	OSSymbol *gIntelPICName = (OSSymbol *) OSSymbol::withCStringNoCopy("io-apic-0");
	specifiers->setObject( tmpData );
	controllers->setObject( gIntelPICName );
	this->setProperty( gIOInterruptControllersKey, controllers ) && this->setProperty( gIOInterruptSpecifiersKey,  specifiers );
	
	this->attachToParent(platform, gIOServicePlane);
	
	InfoLog("successfully initialized");
	
	return true;
}

IOReturn FakeSMCDevice::setProperties(OSObject * properties)
{
    if (OSDictionary * message = OSDynamicCast(OSDictionary, properties)) {
        if (OSString * name = OSDynamicCast(OSString, message->getObject(kFakeSMCDeviceUpdateKeyValue))) {
            if (FakeSMCKey * key = getKey(name->getCStringNoCopy())) {
                
                OSArray *info = OSArray::withCapacity(0);
                
                info->setObject(OSString::withCString(key->getType()));
                info->setObject(OSData::withBytes(key->getValue(), key->getSize()));
                
                values->setObject(key->getName(), info);
                
                this->setProperty(kFakeSMCDeviceValues, OSDictionary::withDictionary(values));
                
                return kIOReturnSuccess;
            }
        }
        else if (OSArray * list = OSDynamicCast(OSArray, message->getObject(kFakeSMCDevicePopulateValues))) {
            if (OSIterator *iterator = OSCollectionIterator::withCollection(list)) {
                while (const OSSymbol *name = (const OSSymbol *)iterator->getNextObject())
                    if (FakeSMCKey * key = getKey(name->getCStringNoCopy())) { 
                        
                        OSArray *info = OSArray::withCapacity(0);
                        
                        info->setObject(OSString::withCString(key->getType()));
                        info->setObject(OSData::withBytes(key->getValue(), key->getSize()));
                        
                        values->setObject(key->getName(), info);
                    }
                
                this->setProperty(kFakeSMCDeviceValues, OSDictionary::withDictionary(values));
                
                iterator->release();
                
                return kIOReturnSuccess;
            }
        }
    }
	
	return kIOReturnUnsupported;
}


void FakeSMCDevice::loadKeysFromDictionary(OSDictionary *dictionary)
{
	if (dictionary) {
		if (OSIterator *iterator = OSCollectionIterator::withCollection(dictionary)) {
			while (const OSSymbol *key = (const OSSymbol *)iterator->getNextObject()) {
				if (OSArray *array = OSDynamicCast(OSArray, dictionary->getObject(key))) {
					if (OSIterator *aiterator = OSCollectionIterator::withCollection(array)) {
						
						OSString *type = OSDynamicCast(OSString, aiterator->getNextObject());
						OSData *value = OSDynamicCast(OSData, aiterator->getNextObject());
						
						if (type && value)
							this->addKeyWithValue(key->getCStringNoCopy(), type->getCStringNoCopy(), value->getLength(), value->getBytesNoCopy());
						
						aiterator->release();
					}
				}
				key = 0;
			}
			
			iterator->release();
		}
		
		InfoLog("%d preconfigured key(s) added", keys->getCount());
	}
	else {
		WarningLog("no preconfigured keys found");
	}
}

UInt32 FakeSMCDevice::getCount() { return keys->getCount(); }

void FakeSMCDevice::updateSharpKey()
{	
	UInt32 count = keys->getCount();
	
	char value[] = { count << 24, count << 16, count << 8, count };
	
	sharpKEY->setValueFromBuffer(value, 4);
}

FakeSMCKey *FakeSMCDevice::addKeyWithValue(const char *name, const char *type, unsigned char size, const void *value)
{	
	if (FakeSMCKey *key = getKey(name)) {
		key->setValueFromBuffer(value, size);
		
		DebugLog("updating value for key %s, type: %s, size: %d", name, type, size);
		
		return key;
	}
	
	DebugLog("adding key %s with value, type: %s, size: %d", name, type, size);
	
	if (FakeSMCKey *key = FakeSMCKey::withValue(name, type, size, value)) {		
		keys->setObject(key);
		updateSharpKey();
		return key;
	}
	
	WarningLog("can't create key %s", name);
	
	return 0;
}

FakeSMCKey *FakeSMCDevice::addKeyWithHandler(const char *name, const char *type, unsigned char size, IOService *handler)
{	
	if (FakeSMCKey *key = getKey(name)) {
		key->setHandler(handler);
		
		DebugLog("changing handler for key %s, type: %s, size: %d", name, type, size);
		
		return key;
	}
	
	DebugLog("adding key %s with handler, type: %s, size: %d", name, type, size);
	
	if (FakeSMCKey *key = FakeSMCKey::withHandler(name, type, size, handler)) {
		keys->setObject(key);
		updateSharpKey();
		return key;
	}
	
	WarningLog("can't create key %s", name);
	
	return 0;
}

inline uint32_t key_to_int(const char *name)
{
    return *((uint32_t*)name);
}

FakeSMCKey *FakeSMCDevice::getKey(const char *name)
{
	if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(keys)) {
		while (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, iterator->getNextObject())) {
            UInt32 key1 = key_to_int(name);
			UInt32 key2 = key_to_int(key->getName());
			if (key1 == key2) {
				iterator->release();
				return key;
			}
		}
		iterator->release();
	}
	
	DebugLog("key %s not found", name);
		
	return 0;
}

FakeSMCKey *FakeSMCDevice::getKey(unsigned int index)
{
	if (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, keys->getObject(index)))
		return key;
	
	DebugLog("key with index %d not found", index);
	
	return 0;
}

void FakeSMCDevice::setDebug(bool debug_val)
{
	debug = debug_val;
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

IOReturn FakeSMCDevice::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	if (functionName->isEqualTo(kFakeSMCSetKeyValue)) {
		const char *name = (const char *)param1;
		UInt8 size = (UInt64)param2;
		const void *data = (const void *)param3;
		
		if (name && data && size > 0) {
            
			if (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, getKey(name)))
				if (key->setValueFromBuffer(data, size))
					return kIOReturnSuccess;
			
			return kIOReturnError;
		}
		
		return kIOReturnBadArgument;
	}
    if (functionName->isEqualTo(kFakeSMCGetKeyHandler)) {
		const char *name = (const char *)param1;
		IOService *handler = (IOService *)param2;
		
		if (name && handler) {
            
			if (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, getKey(name))) {
				if (key->getHandler()) {
                    bcopy(key->getHandler(), handler, sizeof(handler));
                    return kIOReturnSuccess;
                }
                
                return kIOReturnError;
            }
		}
		
		return kIOReturnBadArgument;
	}
	else if (functionName->isEqualTo(kFakeSMCAddKeyHandler)) {
		const char *name = (const char *)param1;
		const char *type = (const char *)param2;
		UInt8 size = (UInt64)param3;
		IOService *handler = (IOService *)param4;
		
		if (name && type && size > 0) {
			
			DebugLog("adding key %s with handler, type %s, size %d", name, type, size);
			
			if (addKeyWithHandler(name, type, size, handler))
				return kIOReturnSuccess;
            
			return kIOReturnError;
		}
		
		return kIOReturnBadArgument;
	}
	else if (functionName->isEqualTo(kFakeSMCAddKeyValue)) {
		const char *name = (const char *)param1;
		const char *type = (const char *)param2;
		UInt8 size = (UInt64)param3;
		const void *value = (const void *)param4;
		
		if (name && type && size > 0) {
			
			DebugLog("adding key %s with value, type %s, size %d", name, type, size);
			
			if (addKeyWithValue(name, type, size, value))
				return kIOReturnSuccess;
            
			return kIOReturnError;
		}
		
		return kIOReturnBadArgument;
	}
	else if (functionName->isEqualTo(kFakeSMCGetKeyValue)) {
		const char *name = (const char *)param1;
		UInt8 *size = (UInt8*)param2;
		const void **value = (const void **)param3;
		
		if (name) {
			if (FakeSMCKey *key = getKey(name)) {
				*size = key->getSize();
				*value = key->getValue();
				
				return kIOReturnSuccess;
			}
            
			return kIOReturnError;
		}
		
		return kIOReturnBadArgument;
	}
    else if (functionName->isEqualTo(kFakeSMCRemoveHandler)) {
        if (IOService *handler = (IOService *)param1)
            if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(keys)) {
                while (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, iterator->getNextObject()))
                    if (key->getHandler() == handler) {
                        key->setHandler(NULL);
                        break;
                    }
                iterator->release();
            }
    }
	
	return kIOReturnUnsupported;
}
