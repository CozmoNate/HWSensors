/*
 *  NVClockX.cpp
 *  HWSensors
 *
 *  Based on NVClock Darwin port by alphamerik (C) 2010
 *  
 *
 *  Created by mozo on 15/10/10.
 *  Copyright 2010 usr-sse2. All rights reserved.
 *
 */

#include "NVClockX.h"
#include "FakeSMC.h"
#include "FakeSMCUtils.h"

#include <stdarg.h>
#include <string.h>

#include "nvclock.h"
#include "backend.h"

/*#define Debug FALSE

#define LogPrefix "NVClockX: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)*/

NVClock nvclock;
NVCard* nv_card;

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(NVClockX, FakeSMCPlugin)

inline bool is_digit(char c)
{
	if (((c>='0')&&(c<='9'))||((c>='a')&&(c<='f'))||((c>='A')&&(c<='F')))
		return true;
	
	return false;
}

bool NVClockX::addSensor(const char* key, const char* type, unsigned char size, int index)
{
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, true, (void *)key, (void *)type, (void *)size, (void *)this)) {
		if (sensors->setObject(key, OSNumber::withNumber(index, 32))) {
            return true;
        } else {
            WarningLog("%s key sensor not set", key);
            return 0;
        }
    }
            
	WarningLog("%s key sensor not added", key);
    
	return 0;
}

int NVClockX::addTachometer(int index)
{
	UInt8 length = 0;
	void * data = 0;
	
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, false, (void *)KEY_FAN_NUMBER, (void *)&length, (void *)&data, 0)) {
		length = 0;
		
		bcopy(data, &length, 1);
		
		char name[5];
		
		snprintf(name, 5, KEY_FORMAT_FAN_SPEED, length); 
		
		if (addSensor(name, TYPE_FPE2, 2, index)) {
		
			length++;
			
			if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, false, (void *)KEY_FAN_NUMBER, (void *)1, (void *)&length, 0))
				WarningLog("error updating FNum value");
			
			return length-1;
		}
	}
	else WarningLog("error reading FNum value");
		
	return -1;
}

bool NVClockX::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
    if (!super::init(properties))
		return false;
	
	if (!(sensors = OSDictionary::withCapacity(0)))
		return false;
	
	return true;
}

IOService* NVClockX::probe(IOService *provider, SInt32 *score)
{
    if (super::probe(provider, score) != this) 
        return 0;
    
    IOPCIDevice* device = (IOPCIDevice*)provider;
    
    if (!device) 
        return 0;
    
    device->setMemoryEnable(true);
    
    nvio = device->mapDeviceMemoryWithIndex(0);
    
#if __LP64__
    mach_vm_address_t addr = (mach_vm_address_t)nvio->getVirtualAddress();
#else
    vm_address_t addr = (vm_address_t)nvio->getVirtualAddress();
#endif
    
    if (OSData * data = OSDynamicCast(OSData, device->getProperty("device-id"))) {
        nvclock.card[nvclock.num_cards].device_id=*(UInt32*)data->getBytesNoCopy();
        nvclock.card[nvclock.num_cards].arch = get_gpu_arch(nvclock.card[nvclock.num_cards].device_id);			
        nvclock.card[nvclock.num_cards].number = nvclock.num_cards;
        nvclock.card[nvclock.num_cards].card_name = (char*)get_card_name(nvclock.card[nvclock.num_cards].device_id, &nvclock.card[nvclock.num_cards].gpu);
        nvclock.card[nvclock.num_cards].state = 0;
        nvclock.card[nvclock.num_cards].reg_address = addr;
        
        //map_mem_card(&nvclock.card[nvclock.num_cards], addr);
        // Map the registers of the nVidia chip 
        // normally pmc is till 0x2000 but extended it for nv40 
        nvclock.card[nvclock.num_cards].PEXTDEV = (volatile unsigned int*)addr + 0x101000;
        nvclock.card[nvclock.num_cards].PFB     = (volatile unsigned int*)addr + 0x100000;
        nvclock.card[nvclock.num_cards].PMC     = (volatile unsigned int*)addr + 0x000000;
        nvclock.card[nvclock.num_cards].PCIO    = (volatile unsigned char*)addr + 0x601000;
        nvclock.card[nvclock.num_cards].PDISPLAY= (volatile unsigned int*)addr + NV_PDISPLAY_OFFSET;
        nvclock.card[nvclock.num_cards].PRAMDAC = (volatile unsigned int*)addr + 0x680000;
        nvclock.card[nvclock.num_cards].PRAMIN  = (volatile unsigned int*)addr + NV_PRAMIN_OFFSET;
        nvclock.card[nvclock.num_cards].PROM    = (volatile unsigned char*)addr + 0x300000;
        
        // On Geforce 8xxx cards it appears that the pci config header has been moved 
        if(nvclock.card[nvclock.num_cards].arch & NV5X)
            nvclock.card[nvclock.num_cards].PBUS = (volatile unsigned int*)addr + 0x88000;
        else
            nvclock.card[nvclock.num_cards].PBUS = nvclock.card[nvclock.num_cards].PMC + 0x1800/4;
        
        nvclock.card[nvclock.num_cards].mem_mapped = 1;
        
        InfoLog("Card: %d, Device ID: %x, Architecture: %x, %s", 
                nvclock.num_cards,
                nvclock.card[nvclock.num_cards].device_id, 
                nvclock.card[nvclock.num_cards].arch,
                nvclock.card[nvclock.num_cards].card_name);
        
        nvclock.num_cards++;
        
        return this;
    }
	
	return 0;
}

bool NVClockX::start(IOService * provider)
{
	DebugLog("Starting...");
	
	if (!super::start(provider)) 
        return false;

	char key[7];
	
	nvclock.dpy = NULL;
	
	for (int index = 0; index < nvclock.num_cards; index++) {
		/* set the card object to the requested card */
		if(!set_card(index)){
			char buf[80];
			WarningLog("%s", get_error(buf, 80));
			return 0;
		}
        
		nvbios* bios=read_bios("");
        
		nvclock.card[index].bios=bios;
		
		/* Check if the card is supported, if not print a message. */
		if(nvclock.card[index].gpu == UNKNOWN){
			WarningLog("it seems your card isn't officialy supported in FakeSMCnVclockPort yet");
			WarningLog("please tell the author the pci_id of the card for further investigation");
			WarningLog("continuing anyway");
		}
        
		if(nv_card->caps & (GPU_TEMP_MONITORING)) {
            InfoLog("Adding temperature sensors");
            
            if(nv_card->caps & BOARD_TEMP_MONITORING) {
                snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, index);
                this->addSensor(key, TYPE_SP78, 2, index);
                
				snprintf(key, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, index);
				this->addSensor(key, TYPE_SP78, 2, index);
			}
            else {
                snprintf(key, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, index);
                this->addSensor(key, TYPE_SP78, 2, index);
            }
		}
		
		if (nv_card->caps & (I2C_FANSPEED_MONITORING | GPU_FANSPEED_MONITORING)){
            InfoLog("Adding tachometer sensor");
            
			int fanIndex = addTachometer(index);
			
			if (fanIndex > -1) {
				snprintf(key, 5, KEY_FORMAT_FAN_ID, fanIndex);
				
				char name[6]; 
				
				snprintf (name, 6, "GPU %X", index);
				
				fakeSMC->callPlatformFunction(kFakeSMCAddKeyValue, false, (void *)key, (void *)TYPE_CH8, (void *)strlen(name), (void *)name);
			}
		}
		
        InfoLog("Adding frequency sensor");
		snprintf(key, 5, KEY_FORMAT_NON_APPLE_GPU_FREQUENCY, index);
		this->addSensor(key, TYPE_UI16, 2, index);
		
		OSNumber* fanKey = OSDynamicCast(OSNumber, getProperty("FanSpeedPercentage"));
		
		if((fanKey!=NULL)&(nv_card->set_fanspeed!=NULL)) {
            InfoLog("Changing fan speed to %d", fanKey->unsigned8BitValue());
			nv_card->set_fanspeed(fanKey->unsigned8BitValue());
        }
		
		OSNumber* speedKey=OSDynamicCast(OSNumber, getProperty("GPUSpeed"));
		
		if ((speedKey!=NULL)&(nv_card->caps&GPU_OVERCLOCKING)) {
			InfoLog("Default speed %d", (UInt16)nv_card->get_gpu_speed());
			//InfoLog("%d", speedKey->unsigned16BitValue());
			nv_card->set_gpu_speed(speedKey->unsigned16BitValue());
			InfoLog("Overclocked to %d", (UInt16)nv_card->get_gpu_speed());
		}
	}
    
    registerService(0);
	
	return true;
}

void NVClockX::stop (IOService* provider)
{
	DebugLog("Stoping...");
    
    fakeSMC->callPlatformFunction(kFakeSMCRemoveHandler, true, this, NULL, NULL, NULL);
	
	sensors->flushCollection();
	
	super::stop(provider);
}

void NVClockX::free ()
{
	DebugLog("Freeing...");
	
	sensors->release();
	
	super::free();
}

IOReturn NVClockX::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* key = (const char*)param1;
		char * data = (char*)param2;
		//UInt32 size = (UInt64)param3;
		
		if (key && data) {
			if (OSNumber *number = OSDynamicCast(OSNumber, sensors->getObject(key))) {
				
				UInt32 index = number->unsigned16BitValue();
				
				if (index < nvclock.num_cards) {
					
					if (!set_card(index)){
						char buf[80];
						WarningLog("%s", get_error(buf, 80));
						return kIOReturnSuccess;
					}
					
					UInt16 value = 0;
					
					switch (key[0]) {
						case 'T':
							switch (key[3]) {
								case 'D':
									if (nv_card->caps & GPU_TEMP_MONITORING)
                                        value = nv_card->get_gpu_temp(nv_card->sensor);
									break;
								case 'H':
									if (nv_card->caps & BOARD_TEMP_MONITORING)
										value = nv_card->get_board_temp(nv_card->sensor);
                                    else 
                                        value = nv_card->get_gpu_temp(nv_card->sensor);
									break;
							}
							
							//bcopy(&value, data, 2);
							memcpy(data, &value, 2);
							
							break;
						case 'F':
							switch (key[2]) {
								case 'A':
									if (nv_card->caps & I2C_FANSPEED_MONITORING)
										value = encode_long(TYPE_FP2E, nv_card->get_i2c_fanspeed_rpm(nv_card->sensor));
									else if(nv_card->caps & GPU_FANSPEED_MONITORING)
										value = encode_long(TYPE_FP2E, (UInt16)nv_card->get_fanspeed());
									else value = 0;
									
									//bcopy(&value, data, 2);
									memcpy(data, &value, 2);
									
									break;
								case 'C':
									value=(UInt16)nv_card->get_gpu_speed();
									
									//bcopy(&value, data, 2);
									memcpy(data, &value, 2);
									
									break;
							}
					}
					return kIOReturnSuccess;					
				}
			}
			
			return kIOReturnBadArgument;
		}
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
