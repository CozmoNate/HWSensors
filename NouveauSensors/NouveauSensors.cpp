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

#include "FakeSMCDefinitions.h"
#include "NouveauDefinitions.h"
#include "nouveau.h"
#include "nvclock.h"
#include "i2c.h"

#include "NouveauSensors.h"

#define kNouveauPWMSensor  1000

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(NouveauSensors, FakeSMCPlugin)

static uint16_t findstr(uint8_t *data, int n, const uint8_t *str, int len)
{
	int i, j;
    
	for (i = 0; i <= (n - len); i++) {
		for (j = 0; j < len; j++)
			if (data[i + j] != str[j])
				break;
		if (j == len)
			return i;
	}
    
	return 0;
}

void NouveauSensors::bios_shadow()
{
    //try to load bios from "vbios" property created by Chameleon boolloader
    
    if (OSData *vbios = OSDynamicCast(OSData, device->getProperty("vbios"))) {
        card.bios.length = vbios->getLength();
        card.bios.data = (UInt8 *)IOMalloc(card.bios.length);
        memcpy(card.bios.data, vbios->getBytesNoCopy(), card.bios.length);
    }

    if (3 != score_vbios(&card, false)) {

        bios_shadow_pramin(&card);
        
        if (3 != score_vbios(&card, true)) {
            
            bios_shadow_prom(&card);
                        
            if (3 != score_vbios(&card, false)) {
                HWSensorsWarningLog("failed to read VBIOS");
                
                /*if (!bios.data) {
                    bios.length = 65536;
                    bios.data = (UInt8 *)IOMalloc(bios.length);
                }*/
            } else HWSensorsInfoLog("VBIOS successfully read from PROM");
        } else HWSensorsInfoLog("VBIOS successfully read from PRAMIN");
    } else HWSensorsInfoLog("VBIOS successfully read from I/O registry");
    
    
    //Parse bios
    if (card.bios.data) {
        const uint8_t bit_signature[] = { 0xff, 0xb8, 'B', 'I', 'T' };
        const uint8_t bmp_signature[] = { 0xff, 0x7f, 'N', 'V', 0x0 };
        int offset = findstr(card.bios.data, card.bios.length,
                             bit_signature, sizeof(bit_signature));
        if (offset) {
            HWSensorsInfoLog("BIT VBIOS found");
            card.bios.type = NVBIOS_BIT;
            card.bios.offset = offset;
            //return parse_bit_structure(bios, offset + 6);
        }
        
        offset = findstr(card.bios.data, card.bios.length,
                         bmp_signature, sizeof(bmp_signature));
        if (offset) {
            HWSensorsInfoLog("BMP VBIOS found");
            card.bios.type = NVBIOS_BMP;
            card.bios.offset = offset;
            //return parse_bmp_structure(dev, bios, offset);
        }
    }
}

// Driver ===

float NouveauSensors::getSensorValue(FakeSMCSensor *sensor)
{
    switch (sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            if (card.i2c_sensor) {
                switch (sensor->getIndex()) {
                    case 0:
                        return card.i2c_get_gpu_temperature(card.i2c_sensor);
                        
                    case 1:
                        return card.i2c_get_board_temperature(card.i2c_sensor);
                }
            }
            else {
                switch (card.chipset & 0xf0) {
                    case 0x40:
                    case 0x60:
                    case 0x50:
                    case 0x80:
                    case 0x90:
                    case 0xa0:
                        if (card.chipset >= 0x84 && !card.fallback_temperature)
                            return nv84_get_temperature(&card);
                        else
                            return nv40_get_temperature(&card);

                    case 0xc0:
                    case 0xd0:
                    case 0xe0: // testing
                        return nv84_get_temperature(&card);
                }
            }
            break;
            
        case kFakeSMCFrequencySensor:
            switch (card.chipset & 0xf0) {
                case 0x40:
                case 0x60:
                    return nv40_get_clock(&card, (NVClockSource)sensor->getIndex());

                case 0x50:
                case 0x80:
                case 0x90:
                case 0xa0:
                    switch (card.chipset) {
                        case 0x84:
                        case 0x86:
                        case 0x92:
                        case 0x94:
                        case 0x96:
                        case 0x98:
                        case 0xa0:
                        case 0xaa:
                        case 0xac:
                        case 0x50:
                            return nv50_get_clock(&card, (NVClockSource)sensor->getIndex());

                        default:
                            return nva3_get_clock(&card, (NVClockSource)sensor->getIndex());
                    }
                    break;
                case 0xc0:
                case 0xd0:
                    return nvc0_get_clock(&card, (NVClockSource)sensor->getIndex());
            }
            break;
            
        case kNouveauPWMSensor:
            return nouveau_pwmfan_get(&card);
        
        case kFakeSMCTachometerSensor:
            return nouveau_rpmfan_get(&card, 500); // count ticks for 500ms
            
        case kFakeSMCVoltageSensor:
            return nouveau_voltage_get(&card);
    }
    
    return 0;
}

bool NouveauSensors::start(IOService * provider)
{
	HWSensorsDebugLog("Starting...");
	
	if (!super::start(provider)) 
        return false;
    
    if ((device = (IOPCIDevice*)provider)) {
        
        device->setMemoryEnable(true);
        
        if ((card.mmio = device->mapDeviceMemoryWithIndex(0))) {
            
            card.PMC = (volatile unsigned int*)(card.mmio->getVirtualAddress() + NV_PMC_OFFSET);
            card.PCIO = (volatile unsigned char*)(card.mmio->getVirtualAddress() + NV_PRMCIO0_OFFSET);
            
            HWSensorsDebugLog("memory mapped successfully");
        }
        else {
            HWSensorsWarningLog("failed to map memory");
            return false;
        }
    }
    else {
        HWSensorsWarningLog("failed to assign PCI device");
        return false;
    }

    nouveau_card = &card;
    
    UInt32 reg0 = nv_rd32(&card, NV03_PMC_BOOT_0);
    
    if ((reg0 & 0x0f000000) > 0) {
        
        card.chipset = (reg0 & 0xff00000) >> 20;
        
        switch (card.chipset & 0xf0) {
            case 0x10:
            case 0x20:
            case 0x30:
                card.card_type = card.chipset & 0xf0;
                break;
            case 0x40:
            case 0x60:
                card.card_type = NV_40;
                break;
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
                card.card_type = NV_50;
                break;
            case 0xc0:
                card.card_type = NV_C0;
                break;
            case 0xd0:
                card.card_type = NV_D0;
                break;
            case 0xe0:
                card.card_type = NV_E0;
                break;
        }
    } else if ((reg0 & 0xff00fff0) == 0x20004000) {
        if (reg0 & 0x00f00000)
            card.chipset = 0x05;
        else
            card.chipset = 0x04;
        
        card.card_type = NV_04;
    }
    
    if (card.card_type) {
        
        if (OSData * data = OSDynamicCast(OSData, device->getProperty("device-id")))
            card.device_id = *(UInt16*)data->getBytesNoCopy(0, 2);
        
        if (OSData * data = OSDynamicCast(OSData, device->getProperty("vendor-id")))
            card.vendor_id = *(UInt16*)data->getBytesNoCopy(0, 2);
        
        /* determine frequency of timing crystal */
        UInt32 strap = nv_rd32(&card, 0x101000);
        if (card.chipset < 0x17 || (card.chipset >= 0x20 && card.chipset <= 0x25))
            strap &= 0x00000040;
        else
            strap &= 0x00400040;
        
        switch (strap) {
            case 0x00000000: card.crystal = 13500; break;
            case 0x00000040: card.crystal = 14318; break;
            case 0x00400000: card.crystal = 27000; break;
            case 0x00400040: card.crystal = 25000; break;
        }
        
        HWSensorsDebugLog("crystal freq: %u KHz", card.crystal);
        
        bios_shadow();
        
        nouveau_vram_init(&card);
        nouveau_volt_init(&card);
        nouveau_temp_init(&card);
        
        HWSensorsInfoLog("detected an NV%2X generation card (0x%08x) with %lld Mib of %s memory", card.card_type, reg0, card.vram_size / 1024 / 1024, NVVRAMTypeMap[(int)card.vram_type].name);
        
        //Setup sensors
        
        //Find card number
        card.card_index = getVacantGPUIndex();
        
        if (card.card_index < 0) {
            HWSensorsWarningLog("failed to obtain vacant GPU index");
            return false;
        }
        
        char key[5];
        
        //I2C temperature setup
        if (i2c_sensor_init()) {
            snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0);
            snprintf(key, 5, KEY_FORMAT_GPU_HEATSINK_TEMPERATURE, card.card_index);
            addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 1);
        }
        else {
            //Core temperature setup
            switch (card.chipset & 0xf0) {
                case 0x40:
                case 0x60:
                case 0x50:
                case 0x80:
                case 0x90:
                case 0xa0: {
                    int temp = nv84_get_temperature(&card);
                    card.fallback_temperature = card.chipset == 0x92 && (temp <= 0 || temp > 125);
                    break;
                }
                case 0xc0:
                case 0xd0:
                case 0xe0: // testing
                    card.fallback_temperature = false;
                    break;
            }
            
            //Core temperature sensor
            switch (card.chipset & 0xf0) {
                case 0x40:
                case 0x60:
                case 0x50:
                case 0x80:
                case 0x90:
                case 0xa0:
                case 0xc0:
                case 0xd0:
                case 0xe0: // testing
                    snprintf(key, 5, KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE, card.card_index);
                    addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0);
                    break;
            }
        }
        
        //Clocks
        switch (card.chipset & 0xf0) {
            case 0x40:
            case 0x60:
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_FREQUENCY, card.card_index);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockCore);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_SHADER_FREQUENCY, card.card_index);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockShader);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_MEMORY_FREQUENCY, card.card_index);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVCLockMemory);
                break;
            case 0xc0:
            case 0xd0:
            //case 0xe0: // testing
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_FREQUENCY, card.card_index);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockCore);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_SHADER_FREQUENCY, card.card_index);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockShader);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_ROP_FREQUENCY, card.card_index);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockRop);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_MEMORY_FREQUENCY, card.card_index);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVCLockMemory);
                break;
        }
        
        // Fans
        switch (card.chipset & 0xf0) {
            case 0x40:
            case 0x60:
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
            case 0xc0:
            case 0xd0:
            case 0xe0: // testing
            {
                char title[16]; 
                
                if (nouveau_pwmfan_get(&card) > 0) {
                    snprintf(key, 5, KEY_FAKESMC_FORMAT_FAN_PWM, card.card_index);
                    addSensor(key, TYPE_UI8, TYPE_UI8_SIZE, kNouveauPWMSensor, 0);
                }
                
                if (nouveau_rpmfan_get(&card, 100) > 0) {
                    snprintf (title, 16, "GPU %X", card.card_index);
                    addTachometer(1, title);
                }
                
                break;
            }
        }
        
        // Voltages
        switch (card.chipset & 0xf0) {
            case 0x30:
            case 0x40:
            case 0x60:
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
            case 0xc0:
            case 0xd0:
            //case 0xe0: // testing
                if (card.voltage.supported) {
                    snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, card.card_index);
                    addSensor(key, TYPE_FP2E, TYPE_FPXX_SIZE, kFakeSMCVoltageSensor, 0);
                }
                break;
        }
        
        registerService();
        
        return true;
    }
    else HWSensorsWarningLog("NV%02X unsupported (chipset:0x%08x)", card.chipset, reg0);
    
    return false;
}

void NouveauSensors::free(void)
{
    if (card.bios.data) {
        IOFree(card.bios.data, card.bios.length);
        card.bios.data = 0;
        card.bios.length = 0;
    }
    
    if (card.mmio) {
        card.mmio->release();
        card.mmio = 0;
    }
    
    super::free();
}
