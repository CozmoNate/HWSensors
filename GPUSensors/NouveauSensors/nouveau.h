//
//  nouveau.h
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

#ifndef HWSensors_nouveau_h
#define HWSensors_nouveau_h

#include <IOKit/pci/IOPCIDevice.h>

#include "nouveau_definitions.h"
#include "nouveau_bios.h"
#include "nouveau_gpio.h"
#include "nouveau_volt.h"
#include "nouveau_i2c.h"

enum nouveau_clock_source {
    nouveau_clock_core      = 1,
    nouveau_clock_shader    = 2,
    nouveau_clock_memory    = 3,
    nouveau_clock_rop       = 4,
	nouveau_clock_copy      = 5,
	nouveau_clock_daemon    = 6,
	nouveau_clock_vdec      = 7,
	nouveau_clock_dom6      = 8,
	nouveau_clock_unka0    	= 9,	/* nva3:nvc0 */
	nouveau_clock_hub01     = 10,	/* nvc0- */
	nouveau_clock_hub06    	= 11,	/* nvc0- */
	nouveau_clock_hub07    	= 12	/* nvc0- */
};

struct nouveau_pm_temp_sensor_constants {
	u16 offset_constant;
	u16 offset_mult;
	u16 offset_div;
	u16 slope_mult;
	u16 slope_div;
};

enum nouveau_card_type {
    NV_NA    = 0x00,
    NV_04    = 0x04,
    NV_40    = 0x40,
    NV_50    = 0x50,
    NV_C0    = 0xc0,
    NV_D0    = 0xd0,
    NV_E0    = 0xe0,
};

typedef struct _I2CDevRec *I2CDevPtr;

struct nouveau_device {
    IOPCIDevice *pcidev;
    IOMemoryMap *mmio;
    
    const char *name;
	const char *cname;
    
	nouveau_card_type card_type;
    s8 card_index;
    
	u32 chipset;
	u32 crystal;
    
    nouveau_bios bios;
    nvbios vbios;
    
    nouveau_pm_temp_sensor_constants sensor_constants;
    nouveau_pm_voltage voltage;
    nouveau_i2c i2c;
    
    I2CDevPtr nvclock_i2c_sensor;
    
    dcb_gpio_func fan_pwm;
    dcb_gpio_func fan_tach;
        
    int (*gpio_sense)(struct nouveau_device *, int line);
    int (*gpio_find)(struct nouveau_device *, int idx, u8 tag, u8 line, struct dcb_gpio_func *);
    int (*gpio_get)(struct nouveau_device *, int idx, u8 tag, u8 line);
    
	int (*pwm_get)(struct nouveau_device *, int line, u32*, u32*);
    
    int (*clocks_get)(struct nouveau_device *, u8 source);
	int (*voltage_get)(struct nouveau_device *);
    int (*temp_get)(struct nouveau_device *);
	int (*core_temp_get)(struct nouveau_device *);
    int (*board_temp_get)(struct nouveau_device *);
    int (*fan_sense)(struct nouveau_device *);
    int (*fan_pwm_get)(struct nouveau_device *);
    int (*fan_rpm_get)(struct nouveau_device *);
};

inline u8 nv_rd08(nouveau_device *device, u32 addr)
{
	u8 data = *(volatile u8 *)(device->mmio->getVirtualAddress() + addr);
	nv_spam(device, "nv_rd08 0x%06x 0x%02x\n", addr, data);
	return data;
}

inline u16 nv_rd16(nouveau_device *device, u32 addr)
{
	u16 data = _OSReadInt16((volatile void *)device->mmio->getVirtualAddress(), addr);
	nv_spam(device, "nv_rd16 0x%06x 0x%04x\n", addr, data);
	return data;
}

inline u32 nv_rd32(nouveau_device *device, u32 addr)
{
	u32 data = _OSReadInt32((volatile void *)device->mmio->getVirtualAddress(), addr);
	nv_spam(device, "nv_rd32 0x%06x 0x%08x\n", addr, data);
	return data;
}

inline void nv_wr08(nouveau_device *device, u32 addr, u8 data)
{
	nv_spam(device, "nv_wr08 0x%06x 0x%02x\n", addr, data);
	*(volatile u8 *)(device->mmio->getVirtualAddress() + addr) = data;
}

inline void nv_wr16(nouveau_device *device, u32 addr, u16 data)
{
	nv_spam(device, "nv_wr16 0x%06x 0x%04x\n", addr, data);
	_OSWriteInt16((volatile void *)device->mmio->getVirtualAddress(), addr, data);
}

inline void nv_wr32(nouveau_device *device, u32 addr, u32 data)
{
	nv_spam(device, "nv_wr32 0x%06x 0x%08x\n", addr, data);
	_OSWriteInt32((volatile void *)device->mmio->getVirtualAddress(), addr, data);
}

inline u32 nv_mask(nouveau_device *device, u32 addr, u32 mask, u32 data)
{
	u32 temp = nv_rd32(device, addr);
	nv_wr32(device, addr, (temp & ~mask) | data);
	return temp;
}

bool nouveau_identify(struct nouveau_device *device);
bool nouveau_init(struct nouveau_device *device);

#endif
