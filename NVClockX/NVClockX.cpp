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

#include <stdarg.h>
#include <string.h>

#include "nvclock.h"
#include "backend.h"

NVClock nvclock;
NVCard* nv_card;

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(NVClockX, FakeSMCPlugin)

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
            
        case kNVCLockCoreFrequencySensor:
            return nv_card->get_gpu_speed();
            break;
        
        case kNVCLockMemoryFrequencySensor:
            return nv_card->get_memory_speed();
            break;
            
        case kNVCLockShaderFrequencySensor:
            return nv_card->get_shader_speed();
            break;
    }
    
    return 0;
}

bool NVClockX::start(IOService * provider)
{
	HWSensorsDebugLog("Starting...");
	
	if (!super::start(provider)) 
        return false;
    
    if ((videoCard = (IOPCIDevice*)provider)) 
    {
        if (videoCard->setMemoryEnable(true)) 
        {
            if ((nvio = videoCard->mapDeviceMemoryWithIndex(0))) 
            {
                IOVirtualAddress addr = nvio->getVirtualAddress();
                
                if (OSData * data = OSDynamicCast(OSData, videoCard->getProperty("device-id"))) 
                {
                    nvclock.card[nvclock.num_cards].device_id=*(UInt32*)data->getBytesNoCopy();
                    
                    
                    nvclock.card[nvclock.num_cards].arch = get_gpu_arch(nvclock.card[nvclock.num_cards].device_id);
                    nvclock.card[nvclock.num_cards].number = nvclock.num_cards;
                    nvclock.card[nvclock.num_cards].card_name = (char*)get_card_name(nvclock.card[nvclock.num_cards].device_id, &nvclock.card[nvclock.num_cards].gpu);
                    nvclock.card[nvclock.num_cards].state = 0;
                    
                    //nvclock.card[nvclock.num_cards].reg_address = addr;
                    
                    //map_mem_card(&nvclock.card[nvclock.num_cards], addr);
                    // Map the registers of the nVidia chip 
                    // normally pmc is till 0x2000 but extended it for nv40 
                    nvclock.card[nvclock.num_cards].PEXTDEV = (volatile unsigned int*)(addr + 0x101000);
                    nvclock.card[nvclock.num_cards].PFB     = (volatile unsigned int*)(addr + 0x100000);
                    nvclock.card[nvclock.num_cards].PMC     = (volatile unsigned int*)(addr + 0x000000);
                    nvclock.card[nvclock.num_cards].PCIO    = (volatile unsigned char*)(addr + 0x601000);
                    nvclock.card[nvclock.num_cards].PDISPLAY= (volatile unsigned int*)(addr + NV_PDISPLAY_OFFSET);
                    nvclock.card[nvclock.num_cards].PRAMDAC = (volatile unsigned int*)(addr + 0x680000);
                    nvclock.card[nvclock.num_cards].PRAMIN  = (volatile unsigned int*)(addr + NV_PRAMIN_OFFSET);
                    nvclock.card[nvclock.num_cards].PROM    = (volatile unsigned char*)(addr + 0x300000);
                    
                    // On Geforce 8xxx cards it appears that the pci config header has been moved 
                    if(nvclock.card[nvclock.num_cards].arch & NV5X)
                        nvclock.card[nvclock.num_cards].PBUS = (volatile unsigned int*)(addr + 0x88000);
                    else
                        nvclock.card[nvclock.num_cards].PBUS = nvclock.card[nvclock.num_cards].PMC + 0x1800/4;
                    
                    nvclock.card[nvclock.num_cards].mem_mapped = 1;
                    
                    HWSensorsInfoLog("%s device-id=0x%x arch=0x%x", 
                                     nvclock.card[nvclock.num_cards].card_name, 
                                     nvclock.card[nvclock.num_cards].device_id, 
                                     nvclock.card[nvclock.num_cards].arch);
                    
                    nvclock.num_cards++;
                }
                else HWSensorsWarningLog("device-id property not found");                
            }
            else {
                HWSensorsWarningLog("failed to map device's memory");
                return false;
            }
        }
    }else {
        HWSensorsWarningLog("failed to assign PCI device");
        return false;
    }
    
	char key[7];
	
	nvclock.dpy = NULL;
	
	for (int index = 0; index < nvclock.num_cards; index++) {
		/* set the card object to the requested card */
		if (!set_card(index)){
			char buffer[256];
			HWSensorsWarningLog("%s", get_error(buffer, 256));
			return false;
		}

        OSData *bios = OSDynamicCast(OSData, videoCard->getProperty("vbios"));

        nvclock.card[index].bios = read_bios(bios ? bios->getBytesNoCopy() : NULL);
        
		/* Check if the card is supported, if not print a message. */
		if(nvclock.card[index].gpu == UNKNOWN){
			HWSensorsWarningLog("it seems your card isn't officialy supported yet");
			HWSensorsWarningLog("please tell the author the pci_id of the card for further investigation");
			HWSensorsWarningLog("continuing anyway");
		}
        
        SInt8 cardIndex = getVacantGPUIndex();
        
        if (cardIndex < 0) {
            HWSensorsWarningLog("failed to obtain vacant GPU index");
            return false;
        }
        
		if(nv_card->caps & (GPU_TEMP_MONITORING)) {
            HWSensorsInfoLog("registering temperature sensors");
            
            if(nv_card->caps & BOARD_TEMP_MONITORING) {
                snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, cardIndex);
                addSensor(key, TYPE_SP78, 2, kNVCLockDiodeTemperatureSensor, index);
                
				snprintf(key, 5, KEY_FORMAT_GPU_HEATSINK_TEMPERATURE, cardIndex);
				addSensor(key, TYPE_SP78, 2, kNVCLockBoardTemperatureSensor, index);
			}
            else {
                snprintf(key, 5, KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE, cardIndex);
                addSensor(key, TYPE_SP78, 2, kNVCLockBoardTemperatureSensor, index);
            }
		}
		
		if (nv_card->caps & I2C_FANSPEED_MONITORING || nv_card->caps & GPU_FANSPEED_MONITORING){
            HWSensorsInfoLog("registering tachometer sensors");
            
            char title[6]; 
            
            snprintf (title, 6, "GPU %X", cardIndex);
            
			addTachometer(index, title);
		}
		
        HWSensorsInfoLog("registering frequency sensors");
        
        snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_FREQUENCY, index);
        addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kNVCLockCoreFrequencySensor, index);
        
        snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_MEMORY_FREQUENCY, index);
        addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kNVCLockMemoryFrequencySensor, index);
        
        snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_SHADER_FREQUENCY, index);
        addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kNVCLockMemoryFrequencySensor, index);
		
		/*OSNumber* fanKey = OSDynamicCast(OSNumber, getProperty("FanSpeedPercentage"));
         
         if((fanKey!=NULL)&(nv_card->set_fanspeed!=NULL)) {
         HWSensorsInfoLog("Changing fan speed to %d", fanKey->unsigned8BitValue());
         nv_card->set_fanspeed(fanKey->unsigned8BitValue());
         }
         
         OSNumber* speedKey=OSDynamicCast(OSNumber, getProperty("GPUSpeed"));
         
         if ((speedKey!=NULL)&(nv_card->caps&GPU_OVERCLOCKING)) {
         HWSensorsInfoLog("Default speed %d", (UInt16)nv_card->get_gpu_speed());
         //HWSensorsInfoLog("%d", speedKey->unsigned16BitValue());
         nv_card->set_gpu_speed(speedKey->unsigned16BitValue());
         HWSensorsInfoLog("Overclocked to %d", (UInt16)nv_card->get_gpu_speed());
         }*/
	}
    
    registerService();
	
	return true;
}

void NVClockX::free(void)
{
    nvio->release();
    nvio = 0;
    
    super::free();
}
