/*
 *  GeForceX.cpp
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

#include "GeForceX.h"

#include "FakeSMCDefinitions.h"

#include "nouveau.h"
#include <IOKit/IOLib.h>

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(GeForceX, FakeSMCPlugin)

#define kGeForceCoreTemperatureSensor    83001

#define ROM16(x) OSSwapLittleToHostInt16(*(UInt16 *)&(x))
#define ROMPTR(d,x) ({            \
ROM16(x) ? &d[ROM16(x)] : NULL; \
})


/* register access */
static inline UInt32 nv_rd32(const volatile UInt8* mmio, unsigned reg)
{
	return OSReadLittleInt32(mmio, reg);
}

static inline void nv_wr32(volatile UInt8* mmio, unsigned reg, UInt32 val)
{
    OSWriteLittleInt32(mmio, reg, val);
}

static inline UInt32 nv_mask(volatile UInt8* mmio, UInt32 reg, UInt32 mask, UInt32 val)
{
	UInt32 tmp = nv_rd32(mmio, reg);
	nv_wr32(mmio, reg, (tmp & ~mask) | val);
	return tmp;
}

static inline UInt8 nv_rd08(const volatile UInt8* mmio, unsigned reg)
{
	return *(volatile UInt8 *)((uintptr_t)mmio + reg);
}

static inline void nv_wr08(volatile UInt8* mmio, unsigned reg, UInt8 val)
{
    *(volatile UInt8 *)((uintptr_t)mmio + reg) = val;
}

static bool nv_cksum(const uint8_t *data, unsigned int length)
{
	/*
	 * There's a few checksums in the BIOS, so here's a generic checking
	 * function.
	 */
	int i;
	uint8_t sum = 0;
    
	for (i = 0; i < length; i++)
		sum += data[i];
    
	if (sum)
		return true;
    
	return false;
}

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

static bool bit_table(struct NVBios bios, UInt8 id, struct NVBitEntry *bit)
{
	UInt8 entries, *entry;
    
	if (bios.type != NVBIOS_BIT)
		return false;
    
	entries = bios.data[bios.offset + 10];
	entry   = &bios.data[bios.offset + 12];
	while (entries--) {
		if (entry[0] == id) {
			bit->id = entry[0];
			bit->version = entry[1];
			bit->length = ROM16(entry[2]);
			bit->offset = ROM16(entry[4]);
			bit->data = ROMPTR(bios.data, entry[4]);
			return true;
		}
        
		entry += bios.data[bios.offset + 9];
	}
    
	return false;
}

int GeForceX::nv40_sensor_setup()
{
	SInt32 offset = sensor_constants.offset_mult / sensor_constants.offset_div;
	SInt32 sensor_calibration;
    
	/* set up the sensors */
	sensor_calibration = 120 - offset - sensor_constants.offset_constant;
	sensor_calibration = sensor_calibration * sensor_constants.slope_div / sensor_constants.slope_mult;
    
	if (chipset >= 0x46)
		sensor_calibration |= 0x80000000;
	else
		sensor_calibration |= 0x10000000;
    
	nv_wr32(PMC, 0x0015b0, sensor_calibration);
    
	/* Wait for the sensor to update */
	IOSleep(5);
    
	/* read */
	return nv_rd32(PMC, 0x0015b4) & 0x1fff;
}

int GeForceX::nv40_temp_get()
{
	int offset = sensor_constants.offset_mult / sensor_constants.offset_div;
	int core_temp;
    
	if (card_type >= NV_50) {
		core_temp = nv_rd32(PMC, 0x20008);
	} else {
		core_temp = nv_rd32(PMC, 0x0015b4) & 0x1fff;
		/* Setup the sensor if the temperature is 0 */
		if (core_temp == 0)
			core_temp = nv40_sensor_setup();
	}
    
	core_temp = core_temp * sensor_constants.slope_mult / sensor_constants.slope_div;
	core_temp = core_temp + offset + sensor_constants.offset_constant;
    
	return core_temp;
}

int GeForceX::nv84_temp_get()
{
	return nv_rd32(PMC, 0x20400);
}

int GeForceX::score_vbios(const bool writeable)
{
	if (!bios.data || bios.data[0] != 0x55 || bios.data[1] != 0xAA) {
		return 0;
	}
    
	if (nv_cksum(bios.data, bios.data[2] * 512)) {
		/* if a ro image is somewhat bad, it's probably all rubbish */
		return writeable ? 2 : 1;
	}
    
	return 3;
}

void GeForceX::bios_shadow_pramin()
{
	UInt32 bar0 = 0;
	int i;
    
	if (card_type >= NV_50) {
		UInt64 addr = (UInt64)(nv_rd32(PMC, 0x619f04) & 0xffffff00) << 8;
		if (!addr) {
			addr  = (UInt64)nv_rd32(PMC, 0x001700) << 16;
			addr += 0xf0000;
		}
        
		bar0 = nv_mask(PMC, 0x001700, 0xffffffff, addr >> 16);
	}
    
	/* bail if no rom signature */
	if (nv_rd08(PMC, NV_PRAMIN_OFFSET + 0) != 0x55 ||
	    nv_rd08(PMC, NV_PRAMIN_OFFSET + 1) != 0xaa)
		goto out;
    
	bios.length = nv_rd08(PMC, NV_PRAMIN_OFFSET + 2) * 512;
	bios.data = (uint8_t*)IOMalloc(bios.length);
	if (bios.data) {
		for (i = 0; i < bios.length; i++)
			bios.data[i] = nv_rd08(PMC, NV_PRAMIN_OFFSET + i);
	}
    
out:
	if (card_type >= NV_50)
		nv_wr32(PMC, 0x001700, bar0);
}

void GeForceX::bios_shadow_prom()
{
	UInt32 pcireg, access;
	UInt16 pcir;
	int i;
    
	/* enable access to rom */
	if (card_type >= NV_50)
		pcireg = 0x088050;
	else
		pcireg = NV_PBUS_PCI_NV_20;
    
	access = nv_mask(PMC, pcireg, 0x00000001, 0x00000000);
    
	/* bail if no rom signature, with a workaround for a PROM reading
	 * issue on some chipsets.  the first read after a period of
	 * inactivity returns the wrong result, so retry the first header
	 * byte a few times before giving up as a workaround
	 */
	i = 16;
	do {
		if (nv_rd08(PMC, NV_PROM_OFFSET + 0) == 0x55)
			break;
	} while (i--);
    
	if (!i || nv_rd08(PMC, NV_PROM_OFFSET + 1) != 0xaa)
		goto out;
    
	/* additional check (see note below) - read PCI record header */
	pcir = nv_rd08(PMC, NV_PROM_OFFSET + 0x18) |
    nv_rd08(PMC, NV_PROM_OFFSET + 0x19) << 8;
	if (nv_rd08(PMC, NV_PROM_OFFSET + pcir + 0) != 'P' ||
	    nv_rd08(PMC, NV_PROM_OFFSET + pcir + 1) != 'C' ||
	    nv_rd08(PMC, NV_PROM_OFFSET + pcir + 2) != 'I' ||
	    nv_rd08(PMC, NV_PROM_OFFSET + pcir + 3) != 'R')
		goto out;
    
	/* read entire bios image to system memory */
	bios.length = nv_rd08(PMC, NV_PROM_OFFSET + 2) * 512;
	bios.data = (uint8_t*)IOMalloc(bios.length);
	if (bios.data) {
		for (i = 0; i < bios.length; i++)
			bios.data[i] = nv_rd08(PMC, NV_PROM_OFFSET + i);
	}
    
out:
	/* disable access to rom */
	nv_wr32(PMC, pcireg, access);
}

float GeForceX::getSensorValue(FakeSMCSensor *sensor)
{
    switch (sensor->getGroup()) {
        case kGeForceCoreTemperatureSensor:
            switch (chipset & 0xf0) {
                case 0x40:
                case 0x60:
                case 0x50:
                case 0x80:
                case 0x90:
                case 0xa0:
                    return nv40_temp_get();
                    break;
                case 0xc0:
                case 0xd0:
                    return nv84_temp_get();
                    break;
            }
            break;
    }
    
    return 0;
}

IOService* GeForceX::probe(IOService *provider, SInt32 *score)
{
    HWSensorsDebugLog("Probing...");
    
    if (super::probe(provider, score) != this) 
        return 0;
    
    if ((device = (IOPCIDevice*)provider)) {
        
        device->setMemoryEnable(true);
        
        if (IOMemoryMap *mmio = device->mapDeviceMemoryWithIndex(0)) {
            
            PMC = (volatile UInt8 *)mmio->getVirtualAddress();
            
            mmio->release();
            
            return this;
        }
        else HWSensorsWarningLog("failed to map memory");
    }
    else HWSensorsWarningLog("failed to assign PCI device");
    
	return 0;
}

bool GeForceX::start(IOService * provider)
{
	HWSensorsDebugLog("Starting...");
	
	if (!super::start(provider)) 
        return false;
    
    UInt32 reg0 = ~0;
    
    reg0 = nv_rd32(PMC, NV03_PMC_BOOT_0);
    
    if ((reg0 & 0x0f000000) > 0) {
        
        chipset = (reg0 & 0xff00000) >> 20;
        
        switch (chipset & 0xf0) {
            case 0x10:
            case 0x20:
            case 0x30:
                card_type = chipset & 0xf0;
                break;
            case 0x40:
            case 0x60:
                card_type = NV_40;
                break;
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
                card_type = NV_50;
                break;
            case 0xc0:
                card_type = NV_C0;
                break;
            case 0xd0:
                card_type = NV_D0;
                break;
            case 0xe0:
                card_type = NV_E0;
                break;
            default:
                break;
        }
    } else if ((reg0 & 0xff00fff0) == 0x20004000) {
        if (reg0 & 0x00f00000)
            chipset = 0x05;
        else
            chipset = 0x04;
        
        card_type = NV_04;
    }
    
    if (card_type) {
        HWSensorsInfoLog("detected an NV%2X generation card (0x%08x)", card_type, reg0);
        
        //Copy bios to ram
        bios_shadow_pramin();
        
        if (!score_vbios(true)) {
            bios_shadow_prom();
            
            if (!score_vbios(false)) {
                HWSensorsWarningLog("failed to read VBIOS from PRAMIN or PROM, continue anyway");
                IOFree(bios.data, bios.length);
                bios.data = 0;
                bios.length = 0;
            } else HWSensorsInfoLog("VBIOS successfully read from PROM");
        } else HWSensorsInfoLog("VBIOS successfully read from PRAMIN");
        
        
        //Parse bios
        if (bios.data) {
            const uint8_t bit_signature[] = { 0xff, 0xb8, 'B', 'I', 'T' };
            const uint8_t bmp_signature[] = { 0xff, 0x7f, 'N', 'V', 0x0 };
            int offset = findstr(bios.data, bios.length,
                             bit_signature, sizeof(bit_signature));
            if (offset) {
                HWSensorsInfoLog("BIT VBIOS found");
                bios.type = NVBIOS_BIT;
                bios.offset = offset;
                //return parse_bit_structure(bios, offset + 6);
            }
            
            offset = findstr(bios.data, bios.length,
                             bmp_signature, sizeof(bmp_signature));
            if (offset) {
                HWSensorsInfoLog("BMP VBIOS found");
                bios.type = NVBIOS_BMP;
                bios.offset = offset;
                //return parse_bmp_structure(dev, bios, offset);
            }
            
            if (bios.type == NVBIOS_BIT) {
                struct NVBitEntry P;
                
                if (bit_table(bios, 'P', &P)) {
                    
                    UInt8 *temp = NULL;
                    
                    if (P.version == 1)
                        temp = ROMPTR(bios.data, P.data[12]);
                    else if (P.version == 2)
                        temp = ROMPTR(bios.data, P.data[16]);
                    else
                        HWSensorsWarningLog("unknown temp for BIT P %d", P.version);
                    
                    /* Set the default sensor's contants */
                    sensor_constants.offset_constant = 0;
                    sensor_constants.offset_mult = 0;
                    sensor_constants.offset_div = 1;
                    sensor_constants.slope_mult = 1;
                    sensor_constants.slope_div = 1;
                    
                    /* Set the default temperature thresholds */
                    sensor_constants.temp_critical = 110;
                    sensor_constants.temp_down_clock = 100;
                    sensor_constants.temp_fan_boost = 90;
                    
                    /* Set the default range for the pwm fan */
                    sensor_constants.fan_min_duty = 30;
                    sensor_constants.fan_max_duty = 100;
                    
                    /* Set the known default values to setup the temperature sensor */
                    if (card_type >= NV_40) {
                        switch (chipset) {
                            case 0x43:
                                sensor_constants.offset_mult = 32060;
                                sensor_constants.offset_div = 1000;
                                sensor_constants.slope_mult = 792;
                                sensor_constants.slope_div = 1000;
                                break;
                                
                            case 0x44:
                            case 0x47:
                            case 0x4a:
                                sensor_constants.offset_mult = 27839;
                                sensor_constants.offset_div = 1000;
                                sensor_constants.slope_mult = 780;
                                sensor_constants.slope_div = 1000;
                                break;
                                
                            case 0x46:
                                sensor_constants.offset_mult = -24775;
                                sensor_constants.offset_div = 100;
                                sensor_constants.slope_mult = 467;
                                sensor_constants.slope_div = 10000;
                                break;
                                
                            case 0x49:
                                sensor_constants.offset_mult = -25051;
                                sensor_constants.offset_div = 100;
                                sensor_constants.slope_mult = 458;
                                sensor_constants.slope_div = 10000;
                                break;
                                
                            case 0x4b:
                                sensor_constants.offset_mult = -24088;
                                sensor_constants.offset_div = 100;
                                sensor_constants.slope_mult = 442;
                                sensor_constants.slope_div = 10000;
                                break;
                                
                            case 0x50:
                                sensor_constants.offset_mult = -22749;
                                sensor_constants.offset_div = 100;
                                sensor_constants.slope_mult = 431;
                                sensor_constants.slope_div = 10000;
                                break;
                                
                            case 0x67:
                                sensor_constants.offset_mult = -26149;
                                sensor_constants.offset_div = 100;
                                sensor_constants.slope_mult = 484;
                                sensor_constants.slope_div = 10000;
                                break;
                        }
                    }
                    
                    if (temp) {
                        int i, headerlen, recordlen, entries;
                        
                        headerlen = temp[1];
                        recordlen = temp[2];
                        entries = temp[3];
                        temp = temp + headerlen;
                        
                        /* Read the entries from the table */
                        for (i = 0; i < entries; i++) {
                            SInt16 value = OSSwapLittleToHostInt16(temp[1]);
                            
                            switch (temp[0]) {
                                case 0x01:
                                    if ((value & 0x8f) == 0)
                                        sensor_constants.offset_constant = (value >> 9) & 0x7f;
                                    break;
                                    
                                case 0x04:
                                    if ((value & 0xf00f) == 0xa000) /* core */
                                        sensor_constants.temp_critical = (value&0x0ff0) >> 4;
                                    break;
                                    
                                case 0x07:
                                    if ((value & 0xf00f) == 0xa000) /* core */
                                        sensor_constants.temp_down_clock = (value&0x0ff0) >> 4;
                                    break;
                                    
                                case 0x08:
                                    if ((value & 0xf00f) == 0xa000) /* core */
                                        sensor_constants.temp_fan_boost = (value&0x0ff0) >> 4;
                                    break;
                                    
                                case 0x10:
                                    sensor_constants.offset_mult = value;
                                    break;
                                    
                                case 0x11:
                                    sensor_constants.offset_div = value;
                                    break;
                                    
                                case 0x12:
                                    sensor_constants.slope_mult = value;
                                    break;
                                    
                                case 0x13:
                                    sensor_constants.slope_div = value;
                                    break;
                                case 0x22:
                                    sensor_constants.fan_min_duty = value & 0xff;
                                    sensor_constants.fan_max_duty = (value & 0xff00) >> 8;
                                    break;
                                case 0x26:
                                    sensor_constants.fan_pwm_freq = value;
                                    break;
                            }
                            temp += recordlen;
                        }            
                        
                    } else HWSensorsWarningLog("temperature table pointer invalid");
                }
            }
        }
        
        //Setup sensors
        UInt8 cardIndex = 0;
        char key[5];
        
        //Find out this card number
        for (UInt8 i = 0; i < 0xf; i++) {
            
            snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, i); 
            
            if (!isKeyHandled(key)) {
                
                snprintf(key, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, i); 
                
                if (!isKeyHandled(key)) {
                    cardIndex = i;
                    break;
                }
            }
        }
        
        //Core temperature
        switch (chipset & 0xf0) {
            case 0x40:
            case 0x60:
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
            case 0xc0:
            case 0xd0:
                snprintf(key, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, cardIndex);
                addSensor(key, TYPE_SP78, 2, kGeForceCoreTemperatureSensor, 0);
                break;
        }
        
        switch (chipset & 0xf0) {
            case 0x00:
                //engine->pm.clocks_get		= nv04_pm_clocks_get;
                break;
            case 0x10:
                //engine->gpio.drive        = nv10_gpio_drive;
                //engine->gpio.sense        = nv10_gpio_sense;
                //engine->pm.clocks_get     = nv04_pm_clocks_get;
                break;
            case 0x20:
                /*engine->gpio.drive          = nv10_gpio_drive;
                engine->gpio.sense          = nv10_gpio_sense;
                engine->pm.clocks_get		= nv04_pm_clocks_get;*/
                break;
            case 0x30:
                /*engine->gpio.drive		= nv10_gpio_drive;
                engine->gpio.sense		= nv10_gpio_sense;
                engine->pm.clocks_get		= nv04_pm_clocks_get;
                engine->pm.clocks_pre		= nv04_pm_clocks_pre;
                engine->pm.clocks_set		= nv04_pm_clocks_set;
                engine->pm.voltage_get		= nouveau_voltage_gpio_get;
                engine->pm.voltage_set		= nouveau_voltage_gpio_set;*/
                break;
            case 0x40:
            case 0x60:
                /*engine->gpio.init		= nv10_gpio_init;
                engine->gpio.fini		= nv10_gpio_fini;
                engine->gpio.drive		= nv10_gpio_drive;
                engine->gpio.sense		= nv10_gpio_sense;
                engine->gpio.irq_enable		= nv10_gpio_irq_enable;
                engine->pm.clocks_get		= nv40_pm_clocks_get;
                engine->pm.clocks_pre		= nv40_pm_clocks_pre;
                engine->pm.clocks_set		= nv40_pm_clocks_set;
                engine->pm.voltage_get		= nouveau_voltage_gpio_get;
                engine->pm.voltage_set		= nouveau_voltage_gpio_set;
                engine->pm.temp_get		= nv40_temp_get;
                engine->pm.pwm_get		= nv40_pm_pwm_get;
                engine->pm.pwm_set		= nv40_pm_pwm_set;*/
                break;
            case 0x50:
            case 0x80: /* gotta love NVIDIA's consistency.. */
            case 0x90:
            case 0xa0:
                /*engine->gpio.init		= nv50_gpio_init;
                engine->gpio.fini		= nv50_gpio_fini;
                engine->gpio.drive		= nv50_gpio_drive;
                engine->gpio.sense		= nv50_gpio_sense;
                engine->gpio.irq_enable		= nv50_gpio_irq_enable;
                switch (dev_priv->chipset) {
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
                        engine->pm.clocks_get	= nv50_pm_clocks_get;
                        engine->pm.clocks_pre	= nv50_pm_clocks_pre;
                        engine->pm.clocks_set	= nv50_pm_clocks_set;
                        break;
                    default:
                        engine->pm.clocks_get	= nva3_pm_clocks_get;
                        engine->pm.clocks_pre	= nva3_pm_clocks_pre;
                        engine->pm.clocks_set	= nva3_pm_clocks_set;
                        break;
                }
                engine->pm.voltage_get		= nouveau_voltage_gpio_get;
                engine->pm.voltage_set		= nouveau_voltage_gpio_set;
                if (dev_priv->chipset >= 0x84)
                    engine->pm.temp_get	= nv84_temp_get;
                else
                    engine->pm.temp_get	= nv40_temp_get;
                engine->pm.pwm_get		= nv50_pm_pwm_get;
                engine->pm.pwm_set		= nv50_pm_pwm_set;*/
                break;
            case 0xc0:
                /*engine->gpio.init		= nv50_gpio_init;
                engine->gpio.fini		= nv50_gpio_fini;
                engine->gpio.drive		= nv50_gpio_drive;
                engine->gpio.sense		= nv50_gpio_sense;
                engine->gpio.irq_enable		= nv50_gpio_irq_enable;
                engine->pm.temp_get		= nv84_temp_get;
                engine->pm.clocks_get		= nvc0_pm_clocks_get;
                engine->pm.clocks_pre		= nvc0_pm_clocks_pre;
                engine->pm.clocks_set		= nvc0_pm_clocks_set;
                engine->pm.voltage_get		= nouveau_voltage_gpio_get;
                engine->pm.voltage_set		= nouveau_voltage_gpio_set;
                engine->pm.pwm_get		= nv50_pm_pwm_get;
                engine->pm.pwm_set		= nv50_pm_pwm_set;*/
                break;
            case 0xd0:
                /*engine->gpio.init		= nv50_gpio_init;
                engine->gpio.fini		= nv50_gpio_fini;
                engine->gpio.drive		= nvd0_gpio_drive;
                engine->gpio.sense		= nvd0_gpio_sense;
                engine->gpio.irq_enable		= nv50_gpio_irq_enable;
                engine->pm.temp_get		= nv84_temp_get;
                engine->pm.clocks_get		= nvc0_pm_clocks_get;
                engine->pm.clocks_pre		= nvc0_pm_clocks_pre;
                engine->pm.clocks_set		= nvc0_pm_clocks_set;
                engine->pm.voltage_get		= nouveau_voltage_gpio_get;
                engine->pm.voltage_set		= nouveau_voltage_gpio_set;*/
                break;
            case 0xe0:
                /*engine->gpio.init		= nv50_gpio_init;
                engine->gpio.fini		= nv50_gpio_fini;
                engine->gpio.drive		= nvd0_gpio_drive;
                engine->gpio.sense		= nvd0_gpio_sense;
                engine->gpio.irq_enable		= nv50_gpio_irq_enable;*/
                break;
            default:
                HWSensorsWarningLog("NV%02X unsupported", chipset);
                return 1;
        }
        
        registerService();
        
        return true;
    }
    else HWSensorsWarningLog("unsupported chipset found 0x%08x\n", reg0);
    
    return false;
}

void GeForceX::free(void)
{
    if (bios.data) 
        IOFree(bios.data, bios.length);
    
    super::free();
}
