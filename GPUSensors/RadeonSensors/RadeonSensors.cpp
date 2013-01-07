/*
 *  Radeon.cpp
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "RadeonSensors.h"
#include "FakeSMCDefinitions.h"
#include "radeon_chipinfo_gen.h"

#include "r600.h"
#include "rv770.h"
#include "evergreen.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(RadeonMonitor, FakeSMCPlugin)

float RadeonMonitor::getSensorValue(FakeSMCSensor *sensor)
{
    switch (sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            return card.get_core_temp(&card);
            
        case kFakeSMCFrequencySensor:
            //
            // to do
            //
            break;
    }
    
    return 0;
}

bool RadeonMonitor::start(IOService * provider)
{
    HWSensorsDebugLog("Starting...");
    
	if (!super::start(provider)) 
        return false;
    
    if (!(card.pdev = (IOPCIDevice*)provider))
        return false;
    
    if (OSData *data = OSDynamicCast(OSData, provider->getProperty("device-id"))) {
        card.chip_id = *(UInt32*)data->getBytesNoCopy();
    }
    else {
        HWSensorsFatalLog("device-id property not found");
        return false;
    }

	card.pdev->setMemoryEnable(true);

	for (UInt32 i = 0; (card.mmio = card.pdev->mapDeviceMemoryWithIndex(i)); i++) {
		long unsigned int mmio_base_phys = card.mmio->getPhysicalAddress();
		// Make sure we  select MMIO registers
		if (((card.mmio->getLength()) <= 0x00020000) && (mmio_base_phys != 0))
			break;
	}
    
	if (!card.mmio) {
		HWSensorsInfoLog("failed to map device memory");
		return false;
	}
	
	RADEONCardInfo *devices = RADEONCards;
    
	while (devices->device_id != NULL) {
		if ((devices->device_id & 0xffff) == (card.chip_id & 0xffff)) {
			
   			card.family = devices->ChipFamily;
            
            card.info.device_id = devices->device_id;
			card.info.ChipFamily = devices->ChipFamily;
			card.info.igp = devices->igp;
			card.info.is_mobility = devices->is_mobility;
            
			HWSensorsInfoLog("found ATI Radeon 0x%04x", card.chip_id & 0xffff);
            
			break;
		}
		devices++;
	}
    
	switch (card.family) {
        case CHIP_FAMILY_R600:    /* r600 */
        case CHIP_FAMILY_RV610:
        case CHIP_FAMILY_RV630:
        case CHIP_FAMILY_RV670:
        case CHIP_FAMILY_RV620:
        case CHIP_FAMILY_RV635:
        case CHIP_FAMILY_RS780:
        case CHIP_FAMILY_RS880:
            card.get_core_temp = rv6xx_get_temp;
            break;
            
        case CHIP_FAMILY_RV770:   /* r700 */
        case CHIP_FAMILY_RV730:
        case CHIP_FAMILY_RV710:
        case CHIP_FAMILY_RV740:
            card.get_core_temp = rv770_get_temp;
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
            card.get_core_temp = evergreen_get_temp;
            break;
            
        default:
            HWSensorsFatalLog("card 0x%04x is unsupported", card.chip_id & 0xffff);
            return false;
    }
	    
    char key[5];
    
    //Take up card number
    card.card_index = takeVacantGPUIndex();
    
    if (card.card_index < 0) {
        HWSensorsFatalLog("failed to obtain vacant GPU index");
        return false;
    }
    
    if (card.get_core_temp) {
        snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card.card_index);
        if (!addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0))
            HWSensorsErrorLog("failed to register temperature sensor for key %s", key);
    }
    
    registerService();
    
    return true;
}
