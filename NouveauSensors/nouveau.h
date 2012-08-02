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

#ifndef NOUVEAU_H
#define NOUVEAU_H

#include <architecture/i386/pio.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOMemoryDescriptor.h>

#include "xf86i2c.h"

enum NVBiosType {
    NVBIOS_BIT,
    NVBIOS_BMP
};

enum NVClockSource{
    NVClockCore     = 1,
    NVClockShader   = 2,
    NVClockRop      = 3,
    NVClockCopy     = 4,
    NVClockDaemon   = 5,
    NVClockVdec     = 6,
    NVCLockMemory   = 8
};

struct NVBios {
   	uint8_t*    data;
	uint32_t    length;
    NVBiosType  type;
    uint32_t    offset;
};

struct NVBitEntry {
	uint8_t     id;
	uint8_t     version;
	uint16_t    length;
	uint16_t    offset;
	uint8_t*    data;
};

struct NVSensorConstants {
	UInt16 offset_constant;
	SInt32 offset_mult;
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

enum NV50ClockSource {
	nv50_clk_src_crystal,
	nv50_clk_src_href,
	nv50_clk_src_hclk,
	nv50_clk_src_hclkm3,
	nv50_clk_src_hclkm3d2,
	nv50_clk_src_host,
	nv50_clk_src_nvclk,
	nv50_clk_src_sclk,
	nv50_clk_src_mclk,
	nv50_clk_src_vdec,
	nv50_clk_src_dom6
};

/* VRAM/fb configuration */
enum NVVRAMType {
    NV_MEM_TYPE_UNKNOWN = 0,
    NV_MEM_TYPE_STOLEN  = 1,
    NV_MEM_TYPE_SGRAM   = 2,
    NV_MEM_TYPE_SDRAM   = 3,
    NV_MEM_TYPE_DDR1    = 4,
    NV_MEM_TYPE_DDR2    = 5,
    NV_MEM_TYPE_DDR3    = 6,
    NV_MEM_TYPE_GDDR2   = 7,
    NV_MEM_TYPE_GDDR3   = 8,
    NV_MEM_TYPE_GDDR4   = 9,
    NV_MEM_TYPE_GDDR5   = 10
};

static const struct NVVRAMTypes {
	int value;
	const char *name;
} NVVRAMTypeMap[] = {
    { NV_MEM_TYPE_UNKNOWN, "unknown" },
	{ NV_MEM_TYPE_STOLEN , "stolen system" },
	{ NV_MEM_TYPE_SGRAM  , "SGRAM" },
	{ NV_MEM_TYPE_SDRAM  , "SDRAM" },
	{ NV_MEM_TYPE_DDR1   , "DDR" },
	{ NV_MEM_TYPE_DDR2   , "DDR2" },
	{ NV_MEM_TYPE_DDR3   , "DDR3" },
	{ NV_MEM_TYPE_GDDR2  , "GDDR2" },
	{ NV_MEM_TYPE_GDDR3  , "GDDR3" },
	{ NV_MEM_TYPE_GDDR4  , "GDDR4" },
	{ NV_MEM_TYPE_GDDR5  , "GDDR5" }
};

struct NVGpioFunc {
	UInt8 func;
	UInt8 line;
	UInt8 log[2];
};

enum NVDcbGpioTag {
	DCB_GPIO_PANEL_POWER = 0x01,
	DCB_GPIO_TVDAC0 = 0x0c,
	DCB_GPIO_TVDAC1 = 0x2d,
	DCB_GPIO_PWM_FAN = 0x09,
	DCB_GPIO_FAN_SENSE = 0x3d,
	DCB_GPIO_UNUSED = 0xff
};

struct NVVoltageLevel {
	UInt32 voltage; /* microvolts */
	UInt8  vid;
};

struct NVVoltage {
	bool supported;
	UInt8 version;
	UInt8 vid_mask;
    
	struct NVVoltageLevel *level;
    int nr_level;
};

struct NouveauCard {
    UInt32 chipset;
    UInt32 card_type;
    UInt16 device_id;
    UInt16 vendor_id;
    
    IOMemoryMap *mmio;
    
    NVBios bios;
    
    UInt32 crystal;
    NVSensorConstants sensor_constants;
    NVVoltage voltage;
    
    UInt64 vram_size;
    NVVRAMType vram_type;
    
    SInt8 card_index;
    
    bool fallback_temperature;
    bool i2c_temperature;
    
    I2CDevPtr i2c_sensor;
    int (*i2c_get_gpu_temperature)(I2CDevPtr dev);
    int (*i2c_get_board_temperature)(I2CDevPtr dev);
};

UInt32 nv_rd32(NouveauCard *card, UInt32 reg);
void nv_wr32(NouveauCard *card, UInt32 reg, UInt32 val);
UInt32 nv_mask(NouveauCard *card, UInt32 reg, UInt32 mask, UInt32 val);
UInt8 nv_rd08(NouveauCard *card, UInt32 reg);
void nv_wr08(NouveauCard *card, UInt32 reg, UInt8 val);

void nouveau_volt_init(NouveauCard *card);
float nouveau_voltage_get(NouveauCard *card);

void nouveau_vram_init(NouveauCard *card);
NVVRAMType nouveau_mem_vbios_type(NouveauCard *card);

UInt8 *dcb_table(NouveauCard *card);
UInt8 *dcb_gpio_table(NouveauCard *card);
UInt8 *dcb_gpio_entry(NouveauCard *card, int idx, int ent, UInt8 *version);
bool nouveau_gpio_func_valid(NouveauCard *card, UInt8 tag);
bool nouveau_gpio_find(NouveauCard *card, int idx, UInt8 func, UInt8 line, struct NVGpioFunc *gpio);
UInt32 NVReadCRTC(NouveauCard *card, int head, UInt32 reg);
int nv10_gpio_sense(NouveauCard *card, int line);
int nv50_gpio_sense(NouveauCard *card, int line);
int nvd0_gpio_sense(NouveauCard *card, int line);
int nouveau_gpio_sense(NouveauCard *card, int idx, int line);
int nouveau_gpio_get(NouveauCard *card, int idx, UInt8 tag, UInt8 line);
int nouveau_pwmfan_get(NouveauCard *card);
int nouveau_rpmfan_get(NouveauCard *card, UInt32 milliseconds);
bool nv40_pm_pwm_get(NouveauCard *card, int line, UInt32 *divs, UInt32 *duty);
bool nv50_pm_pwm_get(NouveauCard *card, int line, UInt32 *divs, UInt32 *duty);

void nv20_get_vram(NouveauCard *card);
void nvc0_get_vram(NouveauCard *card);

UInt32 nv40_read_pll_1(NouveauCard *card, UInt32 reg);
UInt32 nv40_read_pll_2(NouveauCard *card, UInt32 reg);
UInt32 nv40_read_clk(NouveauCard *card, UInt32 src);
UInt32 nv40_get_clock(NouveauCard *card, NVClockSource name);
UInt32 nva3_read_clk(NouveauCard *card, int clk, bool ignore_en);


UInt32 nva3_read_vco(NouveauCard *card, int clk);
UInt32 nva3_read_clk(NouveauCard *card, int clk, bool ignore_en);
UInt32 nva3_read_pll(NouveauCard *card, int clk, UInt32 pll);
UInt32 nva3_get_clock(NouveauCard *card, NVClockSource name);

UInt32 nv50_read_div(NouveauCard *card);
UInt32 nv50_read_pll_src(NouveauCard *card, UInt32 base);
UInt32 nv50_read_pll_ref(NouveauCard *card, UInt32 base);
UInt32 nv50_read_pll(NouveauCard *card, UInt32 base);
UInt32 nv50_read_clk(NouveauCard *card, NV50ClockSource source);
UInt32 nv50_get_clock(NouveauCard *card, NVClockSource name);

UInt32 nvc0_read_vco(NouveauCard *card, UInt32 dsrc);
UInt32 nvc0_read_pll(NouveauCard *card, UInt32 pll);
UInt32 nvc0_read_div(NouveauCard *card, UInt32 doff, UInt32 dsrc, UInt32 dctl);
UInt32 nvc0_read_mem(NouveauCard *card);
UInt32 nvc0_read_clk(NouveauCard *card, UInt32 clk);
UInt32 nvc0_get_clock(NouveauCard *card, NVClockSource name);

int nv40_sensor_setup(NouveauCard *card);
int nv40_get_temperature(NouveauCard *card);
int nv84_get_temperature(NouveauCard *card);
int g92_get_temperature(NouveauCard *card);
void nouveau_temp_init(NouveauCard *card);

int score_vbios(NouveauCard *card, const bool writeable);
void bios_shadow_pramin(NouveauCard *card);
void bios_shadow_prom(NouveauCard *card);

#endif /* NOUVEAU_H */
