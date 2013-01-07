/*
 *  GeforceSensors.cpp
 *  HWSensors
 *
 *  Created by kozlek on 19/04/12.
 *  Copyright 2010 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 */

/*
 * Copyright 2007-2008 Nouveau Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "GeforceSensors.h"

#include "FakeSMCDefinitions.h"

#include "nouveau.h"
#include "nvclock_i2c.h"

#define kNouveauPWMSensor               1000

enum nouveau_temp_source {
    nouveau_temp_core       = 1,
    nouveau_temp_board      = 2,
    nouveau_temp_diode      = 3
};

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(GeforceSensors, FakeSMCPlugin)

float GeforceSensors::getSensorValue(FakeSMCSensor *sensor)
{   
    switch (sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            switch (sensor->getIndex()) {
                case nouveau_temp_core:
                    return card.core_temp_get(&card);
                    
                case nouveau_temp_board:
                    return card.board_temp_get(&card);
                    
                case nouveau_temp_diode:
                    return card.temp_get(&card);
            }
            
        case kFakeSMCFrequencySensor:
            return card.clocks_get(&card, sensor->getIndex()) / 1000.0f;
            
        case kNouveauPWMSensor:
            return card.fan_pwm_get(&card);
            
        case kFakeSMCTachometerSensor:
            return card.fan_rpm_get(&card); // count ticks for 500ms
            
        case kFakeSMCVoltageSensor:
            return (float)card.voltage_get(&card) / 1000000.0f;
    }
    
    return 0;
}

bool GeforceSensors::start(IOService * provider)
{
	HWSensorsDebugLog("Starting...");
	
	if (!super::start(provider)) 
        return false;
        
    struct nouveau_device *device = &card;
    
    //Check if we have available card number, and use it in initialization process without taking it up
    card.card_index = getVacantGPUIndex();
    
    if (card.card_index < 0)
        return false;
    
    // map device memory
    if ((device->pcidev = (IOPCIDevice*)provider)) {
        
        device->pcidev->setMemoryEnable(true);
        
        if ((device->mmio = device->pcidev->mapDeviceMemoryWithIndex(0))) {
            nv_debug(device, "memory mapped successfully\n");
        }
        else {
            nv_error(device, "failed to map memory\n");
            return false;
        }
    }
    else {
        nv_error(device, "failed to assign PCI device\n");
        return false;
    }
    
    // identify chipset
    if (!nouveau_identify(device))
        return false;
    
    // shadow and parse bios
    
    //try to load bios from registry first from "vbios" property created by Chameleon boolloader
    if (OSData *vbios = OSDynamicCast(OSData, provider->getProperty("vbios"))) {
        device->bios.size = vbios->getLength();
        device->bios.data = (u8*)IOMalloc(card.bios.size);
        memcpy(device->bios.data, vbios->getBytesNoCopy(), device->bios.size);
    }
    
    if (!device->bios.data || !device->bios.size || nouveau_bios_score(device, true) < 1)
        if (!nouveau_bios_shadow(device)) {
            if (device->bios.data && device->bios.size) {
                IOFree(card.bios.data, card.bios.size);
                device->bios.data = NULL;
                device->bios.size = 0;
            }
            
            nv_error(device, "unable to shadow VBIOS\n");
            
            return false;
        }
    
    nouveau_vbios_init(device);
    nouveau_bios_parse(device);
    
    // initialize funcs and variables
    if (!nouveau_init(device)) {
        nv_error(device, "unable to initialize monitoring driver\n");
        return false;
    }
    
    nv_info(device, "chipset: %s (NV%02X) bios: %02x.%02x.%02x.%02x\n", device->cname, device->chipset, device->bios.version.major, device->bios.version.chip, device->bios.version.minor, device->bios.version.micro);
    
    if (device->card_type < NV_C0) {
        // init i2c structures
        nouveau_i2c_create(device);
        
        // setup nouveau i2c sensors
        nouveau_i2c_probe(device);
    }
    
    // Register sensors
    char key[5];
    
    // Try to take up available GPU index
    card.card_index = takeVacantGPUIndex();
    
    if (card.card_index < 0)
        return false;
    
    if (card.core_temp_get || card.board_temp_get) {
        nv_debug(device, "registering i2c temperature sensors...\n");
        
        if (card.core_temp_get && card.board_temp_get) {
            snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, nouveau_temp_core);
            
            snprintf(key, 5, KEY_FORMAT_GPU_HEATSINK_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, nouveau_temp_board);
        }
        else if (card.core_temp_get) {
            snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, nouveau_temp_core);
        }
        else if (card.board_temp_get) {
            snprintf(key, 5, KEY_FORMAT_GPU_HEATSINK_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, nouveau_temp_board);
        }
    }
    else if (card.temp_get)
    {
        nv_debug(device, "registering temperature sensors...\n");
        
        snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card.card_index);
        addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, nouveau_temp_diode);
    }
    
    if (card.clocks_get) {
        nv_debug(device, "registering clocks sensors...\n");
        
        if (card.clocks_get(&card, nouveau_clock_core) > 0) {
            snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_FREQUENCY, card.card_index);
            addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, nouveau_clock_core);
        }
        
        if (card.clocks_get(&card, nouveau_clock_shader) > 0) {
            snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_SHADER_FREQUENCY, card.card_index);
            addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, nouveau_clock_shader);
        }
        
        if (card.clocks_get(&card, nouveau_clock_rop) > 0) {
            snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_ROP_FREQUENCY, card.card_index);
            addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, nouveau_clock_rop);
        }
        
        if (card.clocks_get(&card, nouveau_clock_memory) > 0) {
            snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_MEMORY_FREQUENCY, card.card_index);
            addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, nouveau_clock_memory);
        }
    }
    
    if (card.fan_pwm_get || card.fan_rpm_get) {
        nv_debug(device, "registering PWM sensors...\n");
        
        if (card.fan_rpm_get && card.fan_rpm_get(device) > 0) {
            char title[6];
            snprintf (title, 6, "GPU %X", card.card_index + 1);
            addTachometer(card.card_index, title);
        }
        
        if (card.fan_pwm_get && card.fan_pwm_get(device) > 0) {
            snprintf(key, 5, KEY_FAKESMC_FORMAT_GPUPWM, card.card_index);
            addSensor(key, TYPE_UI8, TYPE_UI8_SIZE, kNouveauPWMSensor, 0);
        }
    }
    
    if (card.voltage_get && card.voltage.supported) {
        nv_debug(device, "registering voltage sensors...\n");
        snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, card.card_index);
        addSensor(key, TYPE_FP2E, TYPE_FPXX_SIZE, kFakeSMCVoltageSensor, 0);
    }
    
    nv_info(device, "started\n");
    
    return true;
}

void GeforceSensors::free(void)
{
    if (card.mmio)
        OSSafeRelease(card.mmio);
    
    if (card.bios.data) {
        IOFree(card.bios.data, card.bios.size);
        card.bios.data = 0;
    }
    
    super::free();
}
