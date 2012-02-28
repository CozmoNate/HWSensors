/*
 *  ACPIMonitor.cpp
 *  HWSensors
 *
 *  Created by mozo on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "ACPISensors.h"

#include "FakeSMCDefinitions.h"
#include "FakeSMCValueEncoder.h"

#define Debug FALSE

#define LogPrefix "ACPISensors: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPIMonitor, FakeSMCPlugin)

bool ACPIMonitor::addSensor(const char* method, const char* key, const char* type, unsigned char size)
{
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)key, (void *)type, (void *)size, (void *)this))
		if (sensors->setObject(key, OSString::withCString(method))) {
			InfoLog("%s registered", method);
			return true;
		}
	
	return false;
}

bool ACPIMonitor::addTachometer(const char* method, const char* caption)
{
	UInt8 length = 0;
	void * data = 0;
	
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)KEY_FAN_NUMBER, (void *)&length, (void *)&data, 0)) {
		length = 0;
		
		bcopy(data, &length, 1);
		
		char name[5];
		
		snprintf(name, 5, KEY_FORMAT_FAN_SPEED, length); 
		
		if (addSensor(method, name, TYPE_FPE2, 2)) {
			if (caption) {
				snprintf(name, 5, KEY_FORMAT_FAN_ID, length); 
				
				if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyValue, false, (void *)name, (void *)TYPE_CH8, (void *)((UInt64)strlen(caption)), (void *)caption))
					WarningLog("error adding tachometer id value");
			}			
			
			length++;
			
			if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void *)KEY_FAN_NUMBER, (void *)1, (void *)&length, 0))
				WarningLog("error updating FNum value");
			
			return true;
		}
	}
	else WarningLog("error reading FNum value");
	
	return false;
}

bool ACPIMonitor::init(OSDictionary *properties)
{
    if (!super::init(properties))
		return false;
	
	if (!(sensors = OSDictionary::withCapacity(0)))
		return false;
	
	return true;
}

bool ACPIMonitor::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;

	acpiDevice = (IOACPIPlatformDevice *)provider;
	
	char key[5];
	
	//Here is Fan in ACPI	
	OSArray* fanNames = OSDynamicCast(OSArray, getProperty("Fan Names"));
	
	for (int i=0; i<10; i++) 
	{
		snprintf(key, 5, "FAN%X", i);
		
		if (kIOReturnSuccess == acpiDevice->validateObject(key)){
			OSString* name = NULL;
			
			if (fanNames )
				name = OSDynamicCast(OSString, fanNames->getObject(i));
			
			if (!addTachometer(key, name ? name->getCStringNoCopy() : 0))
				WarningLog("Can't add tachometer sensor, key %s", key);
		} 
		else {
			snprintf(key, 5, "FTN%X", i);
			if (kIOReturnSuccess == acpiDevice->validateObject(key)){
				OSString* name = NULL;
				
				if (fanNames )
					name = OSDynamicCast(OSString, fanNames->getObject(i));
				
				if (!addTachometer(key, name ? name->getCStringNoCopy() : 0))
					WarningLog("Can't add tachometer sensor, key %s", key);
			} 
			else
			break;
		}
	}

	//Next step - temperature keys
	if (kIOReturnSuccess == acpiDevice->validateObject("TCPU"))
		addSensor("TCPU", KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("TSYS"))
		addSensor("TSYS", KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("TDIM"))
		addSensor("TDIM", KEY_DIMM_TEMPERATURE, TYPE_SP78, 2);
		
	if (kIOReturnSuccess == acpiDevice->validateObject("TAMB"))
		addSensor("TAMB", KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2);
	
    if (kIOReturnSuccess == acpiDevice->validateObject("TCPP"))
        addSensor("TCPP", KEY_CPU_PROXIMITY_TEMPERATURE, TYPE_SP78, 2);
	// We should add also GPU reading stuff for those who has no supported plug in but have the value on EC registers
	
	
	
	//Voltage
	if (kIOReturnSuccess == acpiDevice->validateObject("VCPU"))
		addSensor("VSN0", KEY_CPU_VOLTAGE, TYPE_FP2E, 2);
	
	if (kIOReturnSuccess == acpiDevice->validateObject("VMEM"))
		addSensor("VSN0", KEY_MEMORY_VOLTAGE, TYPE_FP2E, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("VSN1"))
		addSensor("VSN1", "Vp0C", TYPE_FP2E, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("VSN2"))
		addSensor("VSN2", "Vp1C", TYPE_FP2E, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("VSN3"))
		addSensor("VSN3", "Vp2C", TYPE_FP2E, 2);
	
	//Amperage
	if (kIOReturnSuccess == acpiDevice->validateObject("ISN0"))
		addSensor("ISN0", "ICAC", TYPE_UI16, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("ISN1"))
		addSensor("ISN1", "Ip0C", TYPE_UI16, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("ISN2"))
		addSensor("ISN2", "Ip1C", TYPE_UI16, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("ISN3"))
		addSensor("ISN3", "Ip2C", TYPE_UI16, 2);
	
	//Power
	if (kIOReturnSuccess == acpiDevice->validateObject("PSN0"))
		addSensor("PSN0", "PC0C", TYPE_UI16, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("PSN1"))
		addSensor("PSN1", "PC1C", TYPE_UI16, 2);
	
	// AC Power/Battery
    if (kIOReturnSuccess == acpiDevice->validateObject("ACDC")) // Power Source Read AC/Battery
	{ 
		addSensor("ACDC", "ACEN", TYPE_UI8, 1);
		addSensor("ACDC", "ACFP", TYPE_FLAG, 1);
		addSensor("ACDC", "ACIN", TYPE_FLAG, 1);
	}
	// TODO real SMC returns ACID only when AC is plugged, if not is zeroed, so hardcoding it in plist is not OK IMHO
	// Same goes for ACIC, but no idea how we can get the AC current value..
	
	// Here if ACDC returns 0 we need to set the on battery BATP flag
	
	// Battery stuff, need to implement rest of the keys once i figure those
    if (kIOReturnSuccess == acpiDevice->validateObject("BAK0")) // Battery 0 Current
        addSensor("BAK0", "B0AC", TYPE_SI16, 2);
	
    if (kIOReturnSuccess == acpiDevice->validateObject("BAK1")) // Battery 0 Voltage
        addSensor("BAK1", "B0AV", TYPE_UI16, 2);
	
	//Keys from info.plist
	OSString *tmpString = 0;
	OSData   *tmpObj = 0;
	
//	UInt32 tmpUI32;
//	char tmpCString[7];
	char acpiName[5];
	char aKey[5];
	
	OSIterator *iter = 0;
	const OSSymbol *dictKey = 0;
	OSDictionary *keysToAdd = 0;
	
	keysToAdd = OSDynamicCast(OSDictionary, getProperty("keysToAdd"));
	if (keysToAdd) {	
		iter = OSCollectionIterator::withCollection(keysToAdd);
		if (iter) {
			while ((dictKey = (const OSSymbol *)iter->getNextObject())) {
				tmpObj = 0;
				snprintf(acpiName, 5, "%s", dictKey->getCStringNoCopy());	
				//WarningLog(" Found key %s", acpiName);
				tmpString = OSDynamicCast(OSString, keysToAdd->getObject(dictKey));
				if (tmpString) {
					snprintf(aKey, 5, "%s", tmpString->getCStringNoCopy());
					InfoLog("Custom name=%s key=%s", acpiName, aKey);
					if (kIOReturnSuccess == acpiDevice->validateObject(acpiName)) {
						if (aKey[0] == 'F') {
							if (!addTachometer(aKey, acpiName))
								WarningLog("Can't add tachometer sensor, key %s", aKey);

						} else {
							addSensor(acpiName, aKey, TYPE_UI16, 2);
						}	
					}
				} 
				else {
					WarningLog(" no value for key %s", acpiName);
				}

			}
            
            iter->release();
		} else {
			WarningLog(" can't interate keysToAdd");
		}

	} else {
		WarningLog(" keysToAdd not found");
	}

	registerService(0);

	return true;	
}

void ACPIMonitor::stop (IOService* provider)
{
    super::stop(provider);
    
    sensors->flushCollection();
}

void ACPIMonitor::free ()
{
	sensors->release();
	
	super::free();
}

#define MEGA10 10000000ull
IOReturn ACPIMonitor::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	const char* name = (const char*)param1;
	void * data = param2;
//	UInt64 size = (UInt64)param3;
	OSString* key;
#if __LP64__
	UInt64 value;
#else
	UInt32 value;
#endif
	UInt16 val;
	
	if (functionName->isEqualTo(kFakeSMCSetValueCallback)) {
		if (name && data) {
			if (key = OSDynamicCast(OSString, sensors->getObject(name))) {
				InfoLog("Writing key=%s by method=%s value=%x", name, key->getCStringNoCopy(), *(UInt16*)data);
				OSObject * params[1];
				if (key->getChar(0) == 'F') {
					//val = decode_fpe2(*(UInt16*)data);
                    val = decode_16bit_fractional(TYPE_FPE2, *(UInt16*)data);
				} else {
					val = *(UInt16*)data;
				}
				params[0] = OSDynamicCast(OSObject, OSNumber::withNumber((unsigned long long)val, 32));
				return acpiDevice->evaluateInteger(key->getCStringNoCopy(), &value, params, 1);
				
		/*
		 virtual IOReturn evaluateInteger( const OSSymbol * objectName,
		 UInt32 *         resultInt32,
		 OSObject *       params[]   = 0,
		 IOItemCount      paramCount = 0,
		 IOOptionBits     options    = 0 );
		 flags_num = OSNumber::withNumber((unsigned long long)flags, 32);
		 */
				
			}
			return kIOReturnBadArgument;
		}
		return kIOReturnBadArgument;
		
	}
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		
		if (name && data) {
			if (key = OSDynamicCast(OSString, sensors->getObject(name))) {
				if (kIOReturnSuccess == acpiDevice->evaluateInteger(key->getCStringNoCopy(), &value)) {
				
					val = 0;
					
					if (key->getChar(0) == 'V') {
						//val = encode_fp2e(value);
                        val = encode_16bit_fractional(TYPE_FP2E, value);
					}
					else if (key->getChar(0) == 'F') {
						if (key->getChar(1) == 'A') {
							//val = encode_fpe2(value);
                            val = encode_16bit_fractional(TYPE_FPE2, value);
						} else 
							if (key->getChar(1) == 'T') {
								//val = encode_fpe2(MEGA10 / value);
                                val = encode_16bit_fractional(TYPE_FPE2, MEGA10 / value);
							} else {
							val = value;
						}
					}
					else val = value;
					
					bcopy(&val, data, 2);					
					return kIOReturnSuccess;
				}
			}
			
			return kIOReturnBadArgument;
		}
		
		//DebugLog("bad argument key name or data");
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
