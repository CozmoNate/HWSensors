/*
 *  Radeon.cpp
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "RadeonMonitor.h"
#include "RadeonChipinfo.h"

#include "FakeSMCDefinitions.h"

#define INVID8(offset)		(mmio_base[offset])
#define INVID16(offset)		OSReadLittleInt16((mmio_base), offset)
#define INVID(offset)		OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val)	OSWriteLittleInt32((mmio_base), offset, val)

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(RadeonMonitor, FakeSMCPlugin)

bool RadeonMonitor::initCard()
{
	rinfo = (RADEONCardInfo*)IOMalloc(sizeof(RADEONCardInfo));
	VCard->setMemoryEnable(true);
	/*	
	 // PCI dump
	 for (int i=0; i<0xff; i +=16) {
     IOLog("%02lx: ", (long unsigned int)i);
     for (int j=0; j<16; j += 4) {
     IOLog("%08lx ", (long unsigned int)VCard->configRead32(i+j));
     }
     IOLog("\n");
	 }
	 */
	for (UInt32 i = 0; (mmio = VCard->mapDeviceMemoryWithIndex(i)); i++)
	{
		long unsigned int mmio_base_phys = mmio->getPhysicalAddress();
		// Make sure we  select MMIO registers
		if (((mmio->getLength()) <= 0x00020000) && (mmio_base_phys != 0))
			break;
	}
	if (mmio)
	{
		mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
	} 
	else
	{
		HWSensorsInfoLog("have no mmio");
		return false;
	}
	
	if(!getRadeonInfo())
		return false;
    
	switch (rinfo->ChipFamily) {
        case CHIP_FAMILY_R600:    /* r600 */
        case CHIP_FAMILY_RV610:
        case CHIP_FAMILY_RV630:
        case CHIP_FAMILY_RV670:
        case CHIP_FAMILY_RV620:
        case CHIP_FAMILY_RV635:
        case CHIP_FAMILY_RS780:
        case CHIP_FAMILY_RS880:
            //setup_R6xx();
            tempFamily = R6xx;
            break;
        case CHIP_FAMILY_RV770:   /* r700 */
        case CHIP_FAMILY_RV730:
        case CHIP_FAMILY_RV710:
        case CHIP_FAMILY_RV740:
            //setup_R7xx();
            tempFamily = R7xx;
            break;
        case CHIP_FAMILY_CEDAR:   /* evergreen */
        case CHIP_FAMILY_REDWOOD:
        case CHIP_FAMILY_JUNIPER:
        case CHIP_FAMILY_CYPRESS:
        case CHIP_FAMILY_HEMLOCK:
        case CHIP_FAMILY_PALM:
        case CHIP_FAMILY_SUMO:
        case CHIP_FAMILY_SUMO2:
        case CHIP_FAMILY_BARTS:
        case CHIP_FAMILY_TURKS:
        case CHIP_FAMILY_CAICOS:
        case CHIP_FAMILY_CAYMAN:
        case CHIP_FAMILY_ARUBA:
        case CHIP_FAMILY_TAHITI:
        case CHIP_FAMILY_PITCAIRN:
        case CHIP_FAMILY_VERDE:
            //setup_Evergreen();
            tempFamily = R8xx;
            break;
            
        default:
            HWSensorsInfoLog("sorry, but your card %04lx is not supported!\n", (long unsigned int)(rinfo->device_id));
            return false;
    }
	
	return true;
}

bool RadeonMonitor::getRadeonInfo()
{
	UInt16 devID = chipID & 0xffff;
	RADEONCardInfo *devices = radeon_device_list;
	//rinfo = new RADEONCardInfo;
	while (devices->device_id != NULL) {
		//IOLog("check %d/n", devices->device_id ); //Debug
		if ((devices->device_id & 0xffff) == devID ) {
			//			rinfo->device_id = devID;
			rinfo->device_id = devices->device_id;
			rinfo->ChipFamily = devices->ChipFamily;
			family = devices->ChipFamily;
			rinfo->igp = devices->igp;
			rinfo->is_mobility = devices->is_mobility;
			HWSensorsInfoLog("found ATI Radeon %04lx", (long unsigned int)devID);
			return true;
		}
		devices++;
	}
    
	HWSensorsErrorLog("unknown device id");
    
	return false;
}

/*
 void ATICard::setup_R6xx()
 {
 char key[5];
 int id = GetNextUnusedKey(KEY_FORMAT_GPU_DIODE_TEMPERATURE, key);
 if (id == -1) {
 HWSensorsInfoLog("No new GPU SMC key!\n");
 return;
 }
 card_number = id;
 tempSensor = new R6xxTemperatureSensor(this, id, key, TYPE_SP78, 2);
 Caps = GPU_TEMP_MONITORING;	
 }
 
 void ATICard::setup_R7xx()
 {
 char key[5];
 int id = GetNextUnusedKey(KEY_FORMAT_GPU_DIODE_TEMPERATURE, key);
 if (id == -1) {
 HWSensorsInfoLog("No new GPU SMC key!\n");
 return;
 }
 card_number = id;
 tempSensor = new R7xxTemperatureSensor(this, id, key, TYPE_SP78, 2);
 Caps = GPU_TEMP_MONITORING;
 }
 
 void ATICard::setup_Evergreen()
 {
 char key[5];
 int id = GetNextUnusedKey(KEY_FORMAT_GPU_DIODE_TEMPERATURE, key);
 if (id == -1) {
 HWSensorsInfoLog("No new GPU SMC key!\n");
 return;
 }
 card_number = id;
 tempSensor = new EverTemperatureSensor(this, id, key, TYPE_SP78, 2);
 Caps = GPU_TEMP_MONITORING;
 }
 */

UInt32 RadeonMonitor::read32(UInt32 reg)
{
	return INVID(reg);
}

IOReturn RadeonMonitor::R6xxTemperatureSensor(UInt16* data)
{
	UInt32 temp, actual_temp = 0;
	for (int i=0; i<1000; i++) {  //attempts to ready
		temp = (read32(CG_THERMAL_STATUS) & ASIC_T_MASK) >> ASIC_T_SHIFT;	
		if ((temp >> 7) & 1)
			actual_temp = 0;
		else {
			actual_temp = temp & 0xff; //(temp >> 1)
			break;
		}
		IOSleep(10);
	}
	*data = (UInt16)(actual_temp & 0xfff);
	//data[1] = 0;
	return kIOReturnSuccess; 
	
}

IOReturn RadeonMonitor::R7xxTemperatureSensor(UInt16* data)
{
	UInt32 temp, actual_temp = 0;
	for (int i=0; i<1000; i++) {  //attempts to ready
		temp = (read32(CG_MULT_THERMAL_STATUS) & ASIC_TM_MASK) >> ASIC_TM_SHIFT;	
		if ((temp >> 9) & 1)
			actual_temp = 0;
		else {
			actual_temp = (temp >> 1) & 0xff;
			break;
		}
		IOSleep(10);
	}
	
	*data = (UInt16)(actual_temp & 0xfff);
	//data[1] = 0;
	return kIOReturnSuccess;
}

IOReturn RadeonMonitor::EverTemperatureSensor(UInt16* data)
{
	UInt32 temp, actual_temp = 0;
	for (int i=0; i<1000; i++) {  //attempts to ready
		temp = (read32(CG_MULT_THERMAL_STATUS) & ASIC_TM_MASK) >> ASIC_TM_SHIFT;	
		if ((temp >> 10) & 1)
			actual_temp = 0;
		else if ((temp >> 9) & 1)
			actual_temp = 255;
		else {
			actual_temp = (temp >> 1) & 0xff;
			break;
		}
		IOSleep(10);
	}
	
	*data = (UInt16)(actual_temp & 0xfff);
	//data[1] = 0;
	return kIOReturnSuccess;
}

float RadeonMonitor::getSensorValue(FakeSMCSensor *sensor)
{
    if (sensor->getGroup() == kFakeSMCTemperatureSensor) {
        UInt16 t = 0;
        
        switch (tempFamily) {
            case R6xx:
                R6xxTemperatureSensor(&t);
                break;
                
            case R7xx:
                R7xxTemperatureSensor(&t);
                break;
                
            case R8xx:
                EverTemperatureSensor(&t);
                break;
        }
        
        return t;
    }
    
    return 0;
}

IOService* RadeonMonitor::probe(IOService *provider, SInt32 *score)
{
    if (super::probe(provider, score) != this) 
        return 0;
    
    VCard = (IOPCIDevice*)provider;
    
    if (!VCard) 
        return 0;
    
    if (OSData *data = OSDynamicCast(OSData, provider->getProperty("device-id"))) {
        chipID = *(UInt32*)data->getBytesNoCopy();
    }
    else {
        HWSensorsFatalLog("device-id property not found");
        return 0;
    }
    
    return this;
}

bool RadeonMonitor::start(IOService * provider)
{
	if (!super::start(provider)) 
        return false;
    
    if (!initCard()) {
        HWSensorsFatalLog("can't initialize driver");
        return false;
    }
    
    //Find card number
    SInt8 cardIndex = getVacantGPUIndex();
    
    if (cardIndex < 0) {
        HWSensorsFatalLog("failed to obtain vacant GPU index");
        return false;
    }
    
    char key[5];
    
    snprintf(key, 5, KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE, cardIndex);
	
    if (!addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0)) {
        HWSensorsFatalLog("failed to register temperature sensor for key %s", key);
        return false;
    }
    
    registerService();
    
    return true;
}
