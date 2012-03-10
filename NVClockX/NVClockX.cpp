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

#include "FakeSMCDefinitions.h"

#include <IOKit/pci/IOPCIDevice.h>

#include <stdarg.h>
#include <string.h>

#include "nvclock.h"
#include "backend.h"

#define Debug FALSE

#define LogPrefix "NVClockX: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

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

float NVClockX::getSensorValue(FakeSMCSensor *sensor)
{
    switch (sensor->getGroup()) {
        case kNVCLockDiodeTemperatureSensor:
            if (set_card(sensor->getIndex()) && nv_card->caps & GPU_TEMP_MONITORING)
                return nv_card->get_gpu_temp(nv_card->sensor);
            break;
        case kNVCLockBoardTemperatureSensor:
            if (set_card(sensor->getIndex())) {
                if (nv_card->caps & BOARD_TEMP_MONITORING)
                    return nv_card->get_board_temp(nv_card->sensor);
                else 
                    return nv_card->get_gpu_temp(nv_card->sensor);
            }
            break;
                  
        case kFakeSMCTachometerSensor:
            if (set_card(sensor->getIndex())) {
                if (nv_card->caps & I2C_FANSPEED_MONITORING)
                    return nv_card->get_i2c_fanspeed_rpm(nv_card->sensor);
                else if (nv_card->caps & GPU_FANSPEED_MONITORING)
                    return nv_card->get_fanspeed();
            }
            break;
            
        case kFakeSMCFrequencySensor:
            return nv_card->get_gpu_speed();
    }
    
    return 0;
}

IOService* NVClockX::probe(IOService *provider, SInt32 *score)
{
    DebugLog("Probing...");
    
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
		if (!set_card(index)){
			char buf[80];
			WarningLog("%s", get_error(buf, 80));
			return this;
		}
        
		nvbios* bios=read_bios("");
        
		nvclock.card[index].bios=bios;
		
		/* Check if the card is supported, if not print a message. */
		if(nvclock.card[index].gpu == UNKNOWN){
			WarningLog("it seems your card isn't officialy supported in FakeSMCnVclockPort yet");
			WarningLog("please tell the author the pci_id of the card for further investigation");
			WarningLog("continuing anyway");
		}
        
        UInt8 cardIndex = 0;
        char name[5];
        
        for (UInt8 i = 0; i < 0xf; i++) {
            
            snprintf(name, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, i); 
            
            if (!isKeyHandled(name)) {
                
                snprintf(name, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, i); 
                
                if (!isKeyHandled(name)) {
                    cardIndex = i;
                    break;
                }
            }
        }
        
		if(nv_card->caps & (GPU_TEMP_MONITORING)) {
            InfoLog("Adding temperature sensors");
            
            if(nv_card->caps & BOARD_TEMP_MONITORING) {
                snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, cardIndex);
                addSensor(key, TYPE_SP78, 2, kNVCLockDiodeTemperatureSensor, index);
                
				snprintf(key, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, cardIndex);
				addSensor(key, TYPE_SP78, 2, kNVCLockBoardTemperatureSensor, index);
			}
            else {
                snprintf(key, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, cardIndex);
                addSensor(key, TYPE_SP78, 2, kNVCLockBoardTemperatureSensor, index);
            }
		}
		
		if (nv_card->caps & (I2C_FANSPEED_MONITORING | GPU_FANSPEED_MONITORING)){
            InfoLog("Adding tachometer sensor");
            
            char title[6]; 
            
            snprintf (title, 6, "GPU %X", cardIndex);
            
			addTachometer(index, title);
		}
		
        /*InfoLog("Adding frequency sensor");
		snprintf(key, 5, KEY_FORMAT_NON_APPLE_GPU_FREQUENCY, index);
		this->addSensor(key, TYPE_UI16, 2, index);*/
		
		/*OSNumber* fanKey = OSDynamicCast(OSNumber, getProperty("FanSpeedPercentage"));
		
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
		}*/
	}
    
    registerService();
	
	return true;
}
