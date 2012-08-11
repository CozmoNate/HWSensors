/*
 *  NouveauSensors.cpp
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

#include "NouveauSensors.h"

#include "FakeSMCDefinitions.h"

#include "nouveau.h"
#include "nouveau_temp.h"

#define kNouveauPWMSensor           1000
#define kNouveauTemperatureSensor   1001

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(NouveauSensors, FakeSMCPlugin)

float NouveauSensors::getSensorValue(FakeSMCSensor *sensor)
{
    switch (sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            return card.diode_temp_get(&card);
        
        case kNouveauTemperatureSensor:
            return card.board_temp_get(&card);
            
        case kFakeSMCFrequencySensor:
            return card.clocks_get(&card, sensor->getIndex()) / 1000.0f;
            
        case kNouveauPWMSensor:
            return card.pwm_fan_get(&card);
            
        case kFakeSMCTachometerSensor:
            return card.rpm_fan_get(&card, 500); // count ticks for 500ms
            
        case kFakeSMCVoltageSensor:
            return (float)card.voltage_get(&card) / 1000000.0f;
    }
    
    return 0;
}

bool NouveauSensors::start(IOService * provider)
{
	HWSensorsDebugLog("Starting...");
	
	if (!super::start(provider)) 
        return false;
        
    struct nouveau_device *device = &card;
    
    //Find card number
    card.card_index = getVacantGPUIndex();
    if (card.card_index < 0) {
        nv_error(device, "failed to obtain vacant GPU index\n");
        return false;
    }
    
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
    if (!nouveau_bios_shadow(device)) {
        nv_error(device, "unable to shadow VBIOS\n");
        //return false;
    }
    else nouveau_bios_parse(device);
    
    // initialize funcs and variables
    if (!nouveau_init(device)) {
        nv_error(device, "unable to initialize monitoring driver\n");
        return false;
    }
        
    nv_debug(device, "registering sensors...\n");
    
    // Register sensors
    char key[5];
    
    if (card.diode_temp_get || card.board_temp_get) {
        nv_debug(device, "registering temperature sensors...\n");
        
        if (card.diode_temp_get && card.board_temp_get) {
            snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0);
            
            snprintf(key, 5, KEY_FORMAT_GPU_HEATSINK_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kNouveauTemperatureSensor, 0);
        }
        else if (card.diode_temp_get) {
            snprintf(key, 5, KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0);
        }
        else if (card.board_temp_get) {
            snprintf(key, 5, KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kNouveauTemperatureSensor, 0);
        }
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
    
    if (card.pwm_fan_get || card.rpm_fan_get) {
        nv_debug(device, "registering PWM sensors...\n");
        
        if (card.pwm_fan_get && card.pwm_fan_get(device) > 0) {
            snprintf(key, 5, KEY_FAKESMC_FORMAT_FAN_PWM, card.card_index);
            addSensor(key, TYPE_UI8, TYPE_UI8_SIZE, kNouveauPWMSensor, 0);
        }
        
        if (card.rpm_fan_get && card.rpm_fan_get(device, 100) > 0) {
            char title[6];
            snprintf (title, 6, "GPU %X", card.card_index);
            addTachometer(card.card_index, title);
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

void NouveauSensors::free(void)
{
    if (card.mmio) {
        card.mmio->release();
        card.mmio = 0;
    }
    
    if (card.bios.data) {
        IOFree(card.bios.data, card.bios.size);
        card.bios.data = 0;
    }
    
    super::free();
}
