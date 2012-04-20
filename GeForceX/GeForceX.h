/*
 *  GeForceX.h
 *  HWSensors
 *
 *  Created by kozlek on 19/04/12.
 *  Copyright 2010 kozlek. All rights reserved.
 *
 */

/*
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

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>

#include "FakeSMCPlugin.h"

enum NVBiosType {
    NVBIOS_BIT,
    NVBIOS_BMP
};

struct NVBios {
   	uint8_t *data;
	uint32_t length;
    NVBiosType type;
    uint32_t offset;
};

struct NVBitEntry {
	uint8_t  id;
	uint8_t  version;
	uint16_t length;
	uint16_t offset;
	uint8_t *data;
};

struct NVSensorConstants {
	UInt16 offset_constant;
	SInt16 offset_mult;
	SInt16 offset_div;
	SInt16 slope_mult;
	SInt16 slope_div;
    SInt16 temp_critical;
    SInt16 temp_down_clock;
    SInt16 temp_fan_boost;
    SInt16 fan_min_duty;
    SInt16 fan_max_duty;
    SInt16 fan_pwm_freq;
};

class GeForceX : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(GeForceX)    
	
private:
    IOPCIDevice*    device;
    UInt32          chipset;
    UInt32          card_type;
    
    volatile UInt8* PMC;
    
    NVBios          bios;
    
    struct NVSensorConstants sensor_constants;
    
    int             nv40_sensor_setup();
    int             nv40_temp_get();
    int             nv84_temp_get();
    
    int             score_vbios(const bool writeable);
    void            bios_shadow_pramin();
    void            bios_shadow_prom();
    
protected:
    virtual float       getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
    virtual void		free(void);
};