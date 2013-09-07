/*
 *  Radeon.cpp
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *  Copyright 2013 kozlek. All rights reserved.
 *
 */

#include "RadeonSensors.h"
#include "FakeSMCDefinitions.h"

#include "radeon_chipinfo_gen.h"
#include "radeon_definitions.h"
#include "radeon_atombios.h"
#include "r600.h"
#include "rv770.h"
#include "si.h"
#include "evergreen.h"

#define super GPUSensors
OSDefineMetaClassAndStructors(RadeonSensors, GPUSensors)

float RadeonSensors::getSensorValue(FakeSMCSensor *sensor)
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

bool RadeonSensors::shouldWaitForAccelerator()
{
    return true;
}

bool RadeonSensors::managedStart(IOService *provider)
{
    if (!(card.pdev = pciDevice)) {
        HWSensorsFatalLog("failed to assign PCI device");
        return false;
    }
    
    if ((card.card_index = takeVacantGPUIndex()) < 0) {
        radeon_info(&card, "failed to take GPU index\n");
        return false;
    }
    
    if (OSData *data = OSDynamicCast(OSData, provider->getProperty("device-id"))) {
        card.chip_id = *(UInt32*)data->getBytesNoCopy();
    }
    else {
        radeon_fatal(&card, "device-id property not found");
        return false;
    }
    
    card.pdev->setMemoryEnable(true);
    
//    IOMemoryMap *mmio;
//
//    for (UInt32 i = 0; (mmio = card.pdev->mapDeviceMemoryWithIndex(i)); i++) {
//        long unsigned int mmio_base_phys = mmio->getPhysicalAddress();
//        // Make sure we select MMIO registers
//        if (((mmio->getLength()) <= 0x00040000) && (mmio_base_phys != 0)) {
//            card.mmio = mmio;
//            break;
//        }
//    }
    
    card.mmio = card.pdev->mapDeviceMemoryWithIndex(1);
    
    if (!card.mmio || 0 == card.mmio->getPhysicalAddress()) {
        HWSensorsInfoLog("failed to map device memory");
        return false;
    }
    
    card.family = CHIP_FAMILY_UNKNOW;
    card.int_thermal_type = THERMAL_TYPE_NONE;
    
    RADEONCardInfo *devices = RADEONCards;
    
    while (devices->device_id != NULL) {
        if ((devices->device_id & 0xffff) == (card.chip_id & 0xffff)) {
            
            card.family = devices->ChipFamily;
            
            card.info.device_id = devices->device_id;
            card.info.ChipFamily = devices->ChipFamily;
            card.info.igp = devices->igp;
            card.info.is_mobility = devices->is_mobility;

            break;
        }
        devices++;
    }
    
    if (card.family == CHIP_FAMILY_UNKNOW) {
        radeon_fatal(&card, "unknown card 0x%04x\n", card.chip_id & 0xffff);
        //return false;
    }
    
    //try to load bios from ATY,bin_image property of GPU registry node
    if (OSData *vbios = OSDynamicCast(OSData, card.pdev->getProperty("ATY,bin_image"))) {
        card.bios_size = vbios->getLength();
        card.bios = (UInt8*)IOMalloc(vbios->getLength());
        
        memcpy(card.bios, vbios->getBytesNoCopy(), card.bios_size);
        
        if (card.bios[0] == 0x55 && card.bios[1] == 0xaa) {
            radeon_device *rdev = &card;
            UInt16 tmp = RBIOS16(0x18);
            
            if (RBIOS8(tmp + 0x14) == 0x0) {
                if ((card.bios_header_start = RBIOS16(0x48))) {
                    tmp = card.bios_header_start + 4;
                    if (!memcmp(card.bios + tmp, "ATOM", 4) ||
                        !memcmp(card.bios + tmp, "MOTA", 4)) {
                        card.is_atom_bios = true;
                    } else {
                        card.is_atom_bios = false;
                    }
                    
                    //radeon_info(&card, "%sBIOS detected\n", card.is_atom_bios ? "ATOM" : "COM");
                }
                
            }
            else radeon_error(&card, "not an x86 BIOS ROM, not using\n");
        }
        else radeon_error(&card, "BIOS signature incorrect %x %x\n", card.bios[0], card.bios[1]);
    }
    else radeon_error(&card, "unable to locate ATY,bin_image\n");
    
    if (!card.bios_header_start) {
        // Free memory for bios image if it was allocated
        if (card.bios && card.bios_size) {
            IOFree(card.bios, card.bios_size);
            card.bios = 0;
            card.bios_size = 0;
        }
    }
    else if (atom_parse(&card)) {
        radeon_atombios_get_power_modes(&card);
    }
    
    radeon_info(&card, "found ATI Radeon ID: 0x%04x, %s BIOS: %s\n", card.chip_id & 0xffff, card.bios && card.bios_size ? card.is_atom_bios ? "ATOM" : "COM" : "", card.bios && card.bios_size ? card.bios_name : "undefined");
    
    // Use temperature sensor type based on BIOS name
    if (card.int_thermal_type == THERMAL_TYPE_NONE && card.bios && card.bios_size) {
        if (!strncasecmp("R600", card.bios_name, 64) ||
            !strncasecmp("RV610", card.bios_name, 64) ||
            !strncasecmp("RV630", card.bios_name, 64) ||
            !strncasecmp("RV620", card.bios_name, 64) ||
            !strncasecmp("RV635", card.bios_name, 64) ||
            !strncasecmp("RV670", card.bios_name, 64) ||
            !strncasecmp("RS780", card.bios_name, 64) ||
            !strncasecmp("RS880", card.bios_name, 64)) {
            card.int_thermal_type = THERMAL_TYPE_RV6XX;
        }
        else if (!strncasecmp("RV770", card.bios_name, 64) ||
                 !strncasecmp("RV730", card.bios_name, 64) ||
                 !strncasecmp("RV710", card.bios_name, 64) ||
                 !strncasecmp("RV740", card.bios_name, 64)) {
            card.int_thermal_type = THERMAL_TYPE_RV770;
        }
        else if (!strncasecmp("CEDAR", card.bios_name, 64) ||
                 !strncasecmp("REDWOOD", card.bios_name, 64) ||
                 !strncasecmp("JUNIPER", card.bios_name, 64) ||
                 !strncasecmp("CYPRESS", card.bios_name, 64) ||
                 !strncasecmp("PALM", card.bios_name, 64) ||
                 !strncasecmp("Wrestler", card.bios_name, 64)) {
            card.int_thermal_type = THERMAL_TYPE_EVERGREEN;
        }
        else if (!strncasecmp("SUMO", card.bios_name, 64) ||
                 !strncasecmp("SUMO2", card.bios_name, 64)) {
            card.int_thermal_type = THERMAL_TYPE_SUMO;
        }
        else if (!strncasecmp("ARUBA", card.bios_name, 64) ||
                 !strncasecmp("BARTS", card.bios_name, 64) ||
                 !strncasecmp("TURKS", card.bios_name, 64) ||
                 !strncasecmp("CAICOS", card.bios_name, 64) ||
                 !strncasecmp("CAYMAN", card.bios_name, 64)) {
            card.int_thermal_type = THERMAL_TYPE_NI;
        }
        else if (!strncasecmp("CAPE VERDE", card.bios_name, 64) ||
                 !strncasecmp("PITCAIRN", card.bios_name, 64) ||
                 !strncasecmp("TAHITI", card.bios_name, 64) ||
                 !strncasecmp("OLAND", card.bios_name, 64) ||
                 !strncasecmp("HAINAN", card.bios_name, 64)) {
            card.int_thermal_type = THERMAL_TYPE_SI;
        }
    }
    
    // Use driver's configuration to resolve temperature sensor type
    if (card.int_thermal_type == THERMAL_TYPE_NONE) {
        radeon_info(&card, "using device-id to resolve temperature sensor type\n");
        
        // Enable temperature monitoring
        switch (card.family) {
            case CHIP_FAMILY_R600:    /* r600 */
            case CHIP_FAMILY_RV610:
            case CHIP_FAMILY_RV630:
            case CHIP_FAMILY_RV670:
            case CHIP_FAMILY_RV620:
            case CHIP_FAMILY_RV635:
            case CHIP_FAMILY_RS780:
            case CHIP_FAMILY_RS880:
                card.int_thermal_type = THERMAL_TYPE_RV6XX;
                break;
                
            case CHIP_FAMILY_RV770:   /* r700 */
            case CHIP_FAMILY_RV730:
            case CHIP_FAMILY_RV710:
            case CHIP_FAMILY_RV740:
                card.int_thermal_type = THERMAL_TYPE_RV770;
                break;
                
            case CHIP_FAMILY_CEDAR:   /* evergreen */
            case CHIP_FAMILY_REDWOOD:
            case CHIP_FAMILY_JUNIPER:
            case CHIP_FAMILY_CYPRESS:
            case CHIP_FAMILY_HEMLOCK:
            case CHIP_FAMILY_PALM:
                card.int_thermal_type = THERMAL_TYPE_EVERGREEN;
                break;
                
            case CHIP_FAMILY_BARTS:
            case CHIP_FAMILY_TURKS:
            case CHIP_FAMILY_CAICOS:
            case CHIP_FAMILY_CAYMAN:
            case CHIP_FAMILY_ARUBA:
                card.int_thermal_type = THERMAL_TYPE_NI;
                break;
                
            case CHIP_FAMILY_SUMO:
            case CHIP_FAMILY_SUMO2:
                card.int_thermal_type = THERMAL_TYPE_SUMO;
                break;
                
            case CHIP_FAMILY_TAHITI:
            case CHIP_FAMILY_PITCAIRN:
            case CHIP_FAMILY_VERDE:
            case CHIP_FAMILY_OLAND:
            case CHIP_FAMILY_HAINAN:    
                card.int_thermal_type = THERMAL_TYPE_SI;
                break;
                
                //             default:
                //                 radeon_fatal(&card, "card 0x%04x is unsupported\n", card.chip_id & 0xffff);
                //                 return false;
        }
    }
    
    // Setup temperature sensor
    if (card.int_thermal_type != THERMAL_TYPE_NONE ) {
        switch (card.int_thermal_type) {
            case THERMAL_TYPE_RV6XX:
                card.get_core_temp = rv6xx_get_temp;
                radeon_info(&card, "adding rv6xx thermal sensor\n");
                break;
            case THERMAL_TYPE_RV770:
                card.get_core_temp = rv770_get_temp;
                radeon_info(&card, "adding rv770 thermal sensor\n");
                break;
            case THERMAL_TYPE_EVERGREEN:
            case THERMAL_TYPE_NI:
                card.get_core_temp = evergreen_get_temp;
                radeon_info(&card, "adding EverGreen thermal sensor\n");
                break;
            case THERMAL_TYPE_SUMO:
                card.get_core_temp = sumo_get_temp;
                radeon_info(&card, "adding Sumo thermal sensor\n");
                break;
            case THERMAL_TYPE_SI:
                card.get_core_temp = si_get_temp;
                radeon_info(&card, "adding Southern Islands thermal sensor\n");
                break;
            default:
                radeon_fatal(&card, "card 0x%04x is unsupported\n", card.chip_id & 0xffff);
                releaseGPUIndex(card.card_index);
                card.card_index = -1;
                return false;
        }
    }
    
    char key[5];
    
    enableExclusiveAccessMode();
    
    if (card.get_core_temp) {
        snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card.card_index);
        if (!addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0)) {
            //radeon_error(&card, "failed to register temperature sensor for key %s\n", key);
            radeon_fatal(&card, "failed to register temperature sensor for key %s\n", key);
            releaseGPUIndex(card.card_index);
            card.card_index = -1;
            return false;
        }
    }
    
    disableExclusiveAccessMode();
    
    registerService();
    
    radeon_info(&card, "started\n");
    
    return true;
}

void RadeonSensors::stop(IOService *provider)
{
    if (card.mmio)
        OSSafeRelease(card.mmio);
    
    if (card.bios && card.bios_size > 0) {
        IOFree(card.bios, card.bios_size);
        card.bios = 0;
    }
    
    if (card.card_index >= 0) {
        if (!releaseGPUIndex(card.card_index))
            HWSensorsFatalLog("failed to release GPU index");
    }
    
    super::stop(provider);
}
