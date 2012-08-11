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

#include "NouveauSensors.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(NouveauSensors, FakeSMCPlugin)

float NouveauSensors::getSensorValue(FakeSMCSensor *sensor)
{
    /*switch (sensor->getGroup()) {
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
    }*/
    
    return 0;
}

bool NouveauSensors::start(IOService * provider)
{
	HWSensorsDebugLog("Starting...");
	
	if (!super::start(provider)) 
        return false;
    
    /*if ((device = (IOPCIDevice*)provider)) {
        
        device->setMemoryEnable(true);
        
        if ((card.mmio = device->mapDeviceMemoryWithIndex(0))) {
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
    }*/

        
   /* if (card.card_type) {
        
        if (OSData * data = OSDynamicCast(OSData, device->getProperty("device-id")))
            card.device_id = *(UInt16*)data->getBytesNoCopy(0, 2);
        
        if (OSData * data = OSDynamicCast(OSData, device->getProperty("vendor-id")))
            card.vendor_id = *(UInt16*)data->getBytesNoCopy(0, 2);
        
           }
    else HWSensorsWarningLog("NV%02X unsupported (chipset:0x%08x)", card.chipset, reg0);*/
    
    return false;
}

void NouveauSensors::free(void)
{
    super::free();
}
