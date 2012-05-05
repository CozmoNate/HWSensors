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

#include <architecture/i386/pio.h>

#define kGeForceXPWMSensor  1000

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(GeForceX, FakeSMCPlugin)

#define ROM16(x) OSSwapLittleToHostInt16(*(UInt16 *)&(x))
#define ROM32(x) OSSwapLittleToHostInt32(*(UInt32 *)&(x))
#define ROMPTR(d,x) ({            \
ROM16(x) ? &d[ROM16(x)] : NULL; \
})


/* register access */
UInt32 GeForceX::nv_rd32(UInt32 reg)
{
	return _OSReadInt32((volatile void *)mmio->getVirtualAddress(), reg);
}

void GeForceX::nv_wr32(UInt32 reg, UInt32 val)
{
    _OSWriteInt32((volatile void *)mmio->getVirtualAddress(), reg, val);
}

UInt32 GeForceX::nv_mask(UInt32 reg, UInt32 mask, UInt32 val)
{
	UInt32 tmp = nv_rd32(reg);
	nv_wr32(reg, (tmp & ~mask) | val);
	return tmp;
}

UInt8 GeForceX::nv_rd08(UInt32 reg)
{
	return *(volatile UInt8 *)(mmio->getVirtualAddress() + reg);
}

void GeForceX::nv_wr08(UInt32 reg, UInt8 val)
{
    *(volatile UInt8 *)(mmio->getVirtualAddress() + reg) = val;
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

// Voltage ===

static const UInt8 vidtag[] = { 0x04, 0x05, 0x06, 0x1a, 0x73 };
static int nr_vidtag = sizeof(vidtag) / sizeof(vidtag[0]);

bool GeForceX::nouveau_gpio_func_valid(UInt8 tag)
{
	struct NVGpioFunc func;
	return nouveau_gpio_find(0, tag, 0xff, &func);
}

void GeForceX::nouveau_volt_init()
{
	struct NVBitEntry P;
	UInt8 *volt = NULL, *entry;
	int i, headerlen, recordlen, entries, vidmask, vidshift;
    
	if (bios.type == NVBIOS_BIT) {
		if (!bit_table(bios, 'P', &P)) {
            HWSensorsWarningLog("volt BIT P not found");
			return;
        }
        
		if (P.version == 1)
			volt = ROMPTR(bios.data, P.data[16]);
		else
            if (P.version == 2)
                volt = ROMPTR(bios.data, P.data[12]);
            else {
                HWSensorsWarningLog("unknown volt for BIT P %d", P.version);
            }
	} else {
		if (bios.data[bios.offset + 6] < 0x27) {
			HWSensorsWarningLog("BMP version too old for voltage");
			return;
		}
        
		volt = ROMPTR(bios.data, bios.data[bios.offset + 0x98]);
	}
    
	if (!volt) {
		HWSensorsWarningLog("voltage table pointer invalid");
		return;
	}
    
	switch (volt[0]) {
        case 0x10:
        case 0x11:
        case 0x12:
            headerlen = 5;
            recordlen = volt[1];
            entries   = volt[2];
            vidshift  = 0;
            vidmask   = volt[4];
            break;
        case 0x20:
            headerlen = volt[1];
            recordlen = volt[3];
            entries   = volt[2];
            vidshift  = 0; /* could be vidshift like 0x30? */
            vidmask   = volt[5];
            break;
        case 0x30:
            headerlen = volt[1];
            recordlen = volt[2];
            entries   = volt[3];
            vidmask   = volt[4];
            /* no longer certain what volt[5] is, if it's related to
             * the vid shift then it's definitely not a function of
             * how many bits are set.
             *
             * after looking at a number of nva3+ vbios images, they
             * all seem likely to have a static shift of 2.. lets
             * go with that for now until proven otherwise.
             */
            vidshift  = 2;
            break;
        case 0x40:
            headerlen = volt[1];
            recordlen = volt[2];
            entries   = volt[3]; /* not a clue what the entries are for.. */
            vidmask   = volt[11]; /* guess.. */
            vidshift  = 0;
            break;
        default:
            HWSensorsWarningLog("voltage table 0x%02x unknown", volt[0]);
            return;
	}
        
	/* validate vid mask */
	voltage.vid_mask = vidmask;
	if (!voltage.vid_mask)
		return;
    
	i = 0;
	while (vidmask) {
		if (i > nr_vidtag) {
			HWSensorsWarningLog("vid bit %d unknown", i);
			return;
		}
        
		if (!nouveau_gpio_func_valid(vidtag[i])) {
			HWSensorsWarningLog("vid bit %d has no gpio tag", i);
			return;
		}
        
		vidmask >>= 1;
		i++;
	}
    
	/* parse vbios entries into common format */
	voltage.version = volt[0];
	if (voltage.version < 0x40) {
		voltage.nr_level = entries;
		voltage.level = (NVVoltageLevel*)IOMalloc(sizeof(voltage.level) * entries);//kcalloc(entries, sizeof(*voltage->level), GFP_KERNEL);
		if (!voltage.level) {
            HWSensorsWarningLog("failed to allocate voltages array version <0x40");
			return;
        }
        
		entry = volt + headerlen;
		for (i = 0; i < entries; i++, entry += recordlen) {
			voltage.level[i].voltage = entry[0] * 10000;
			voltage.level[i].vid     = entry[1] >> vidshift;
		}
	} else {
		UInt32 volt_uv = ROM32(volt[4]);
		SInt16 step_uv = ROM16(volt[8]);
		UInt8 vid;
        
		voltage.nr_level = voltage.vid_mask + 1;
		voltage.level = (NVVoltageLevel*)IOMalloc(sizeof(voltage.level) * voltage.nr_level); //kcalloc(voltage->nr_level, sizeof(*voltage->level), GFP_KERNEL);
		if (!voltage.level) {
            HWSensorsWarningLog("failed to allocate voltages array");
			return;
        }
        
		for (vid = 0; vid <= voltage.vid_mask; vid++) {
			voltage.level[vid].voltage = volt_uv;
			voltage.level[vid].vid = vid;
			volt_uv += step_uv;
		}
	}
    
	voltage.supported = true;
}

float GeForceX::nouveau_voltage_get()
{
	UInt8 vid = 0;
	int i;
    
	for (i = 0; i < nr_vidtag; i++) {
		if (!(voltage.vid_mask & (1 << i)))
			continue;
        
		vid |= nouveau_gpio_get(0, vidtag[i], 0xff) << i;
	}
    
	for (i = 0; i < voltage.nr_level; i++) {
		if (voltage.level[i].vid == vid)
			return voltage.level[i].voltage / 1000000.0f;
	}
    
    return 0;
}

// VRAM ===

NVVRAMType GeForceX::nouveau_mem_vbios_type()
{
	struct NVBitEntry M;
    
	UInt8 ramcfg = (nv_rd32(0x101000) & 0x0000003c) >> 2;
    
	if (bit_table(bios, 'M', &M) || M.version != 2 || M.length < 5) {
		UInt8 *table = ROMPTR(bios.data, M.data[3]);
		if (table && table[0] == 0x10 && ramcfg < table[3]) {
			UInt8 *entry = table + table[1] + (ramcfg * table[2]);
			switch (entry[0] & 0x0f) {
                case 0: return NV_MEM_TYPE_DDR2;
                case 1: return NV_MEM_TYPE_DDR3;
                case 2: return NV_MEM_TYPE_GDDR3;
                case 3: return NV_MEM_TYPE_GDDR5;
                default:
                    HWSensorsInfoLog("mem type from VBIOS 0x%x", entry[0] & 0x0f);
                    break;
			}
            
		}
	}
	return NV_MEM_TYPE_UNKNOWN;
}

void GeForceX::nv20_get_vram()
{
	UInt32 ram_size = nv_rd32(0x10020c);
	UInt32 pbus1218 = nv_rd32(0x001218);
    
	vram_size = ram_size & 0xff000000;
    
	switch (pbus1218 & 0x00000300) {
        case 0x00000000: vram_type = NV_MEM_TYPE_SDRAM; break;
        case 0x00000100: vram_type = NV_MEM_TYPE_DDR1; break;
        case 0x00000200: vram_type = NV_MEM_TYPE_GDDR3; break;
        case 0x00000300: vram_type = NV_MEM_TYPE_GDDR2; break;
	}
}

void GeForceX::nvc0_get_vram()
{
	UInt32 parts = nv_rd32(0x022438);
	UInt32 pmask = nv_rd32(0x022554);
	UInt32 bsize = nv_rd32(0x10f20c);
	bool uniform = true;
	int part;
    
	HWSensorsDebugLog("0x100800: 0x%08x", nv_rd32(0x100800));
	HWSensorsDebugLog("parts 0x%08x mask 0x%08x", parts, pmask);
    
	vram_type = nouveau_mem_vbios_type();
	//vram_rank_B = !!(nv_rd32(dev, 0x10f200) & 0x00000004);
    
	/* read amount of vram attached to each memory controller */
	for (part = 0; part < parts; part++) {
		if (!(pmask & (1 << part))) {
			UInt32 psize = nv_rd32(0x11020c + (part * 0x1000));
			if (psize != bsize) {
				if (psize < bsize)
					bsize = psize;
				uniform = false;
			}
            
			HWSensorsDebugLog("%d: mem_amount 0x%08x", part, psize);
			vram_size += (UInt64)psize << 20;
		}
	}
}

void GeForceX::nouveau_vram_init()
{
    vram_type = NV_MEM_TYPE_UNKNOWN;
    
    switch (chipset & 0xf0) {
        case 0x00: {
            UInt32 boot0 = nv_rd32(NV04_PFB_BOOT_0);
            
            if (boot0 & 0x00000100) {
                vram_size  = ((boot0 >> 12) & 0xf) * 2 + 2;
                vram_size *= 1024 * 1024;
            } else {
                switch (boot0 & NV04_PFB_BOOT_0_RAM_AMOUNT) {
                    case NV04_PFB_BOOT_0_RAM_AMOUNT_32MB:
                        vram_size = 32 * 1024 * 1024;
                        break;
                    case NV04_PFB_BOOT_0_RAM_AMOUNT_16MB:
                        vram_size = 16 * 1024 * 1024;
                        break;
                    case NV04_PFB_BOOT_0_RAM_AMOUNT_8MB:
                        vram_size = 8 * 1024 * 1024;
                        break;
                    case NV04_PFB_BOOT_0_RAM_AMOUNT_4MB:
                        vram_size = 4 * 1024 * 1024;
                        break;
                }
            }
            
            if ((boot0 & 0x00000038) <= 0x10)
                vram_type = NV_MEM_TYPE_SGRAM;
            else
                vram_type = NV_MEM_TYPE_SDRAM;
            break;
        }
        case 0x10: {
            if (chipset == 0x1a || chipset == 0x1f) {
                
            } else {
                UInt32 fifo_data = nv_rd32(NV04_PFB_FIFO_DATA);
                UInt32 cfg0 = nv_rd32(0x100200);
                
                vram_size = fifo_data & NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_MASK;
                
                if (cfg0 & 0x00000001)
                    vram_type = NV_MEM_TYPE_DDR1;
                else
                    vram_type = NV_MEM_TYPE_SDRAM;
            }
            break;
        }
        case 0x20: 
        case 0x30:
            nv20_get_vram();
            break;
        case 0x40:
        case 0x60: {
            /* 0x001218 is actually present on a few other NV4X I looked at,
             * and even contains sane values matching 0x100474.  From looking
             * at various vbios images however, this isn't the case everywhere.
             * So, I chose to use the same regs I've seen NVIDIA reading around
             * the memory detection, hopefully that'll get us the right numbers
             */
            if (chipset == 0x40) {
                UInt32 pbus1218 = nv_rd32(0x001218);
                switch (pbus1218 & 0x00000300) {
                    case 0x00000000: vram_type = NV_MEM_TYPE_SDRAM; break;
                    case 0x00000100: vram_type = NV_MEM_TYPE_DDR1; break;
                    case 0x00000200: vram_type = NV_MEM_TYPE_GDDR3; break;
                    case 0x00000300: vram_type = NV_MEM_TYPE_DDR2; break;
                }
            } else
                if (chipset == 0x49 || chipset == 0x4b) {
                    UInt32 pfb914 = nv_rd32(0x100914);
                    switch (pfb914 & 0x00000003) {
                        case 0x00000000: vram_type = NV_MEM_TYPE_DDR1; break;
                        case 0x00000001: vram_type = NV_MEM_TYPE_DDR2; break;
                        case 0x00000002: vram_type = NV_MEM_TYPE_GDDR3; break;
                        case 0x00000003: break;
                    }
                } else
                    if (chipset != 0x4e) {
                        UInt32 pfb474 = nv_rd32(0x100474);
                        if (pfb474 & 0x00000004)
                            vram_type = NV_MEM_TYPE_GDDR3;
                        if (pfb474 & 0x00000002)
                            vram_type = NV_MEM_TYPE_DDR2;
                        if (pfb474 & 0x00000001)
                            vram_type = NV_MEM_TYPE_DDR1;
                    } else {
                        vram_type = NV_MEM_TYPE_STOLEN;
                    }
            
            vram_size = nv_rd32(0x10020c) & 0xff000000;
            break;
        }
        case 0x50:
        case 0x80: /* gotta love NVIDIA's consistency.. */
        case 0x90:
        case 0xa0: {
            UInt32 pfb714 = nv_rd32(0x100714);
            
            switch (pfb714 & 0x00000007) {
                case 0: vram_type = NV_MEM_TYPE_DDR1; break;
                case 1:
                    if (nouveau_mem_vbios_type() == NV_MEM_TYPE_DDR3)
                        vram_type = NV_MEM_TYPE_DDR3;
                    else
                        vram_type = NV_MEM_TYPE_DDR2;
                    break;
                case 2: vram_type = NV_MEM_TYPE_GDDR3; break;
                case 3: vram_type = NV_MEM_TYPE_GDDR4; break;
                case 4: vram_type = NV_MEM_TYPE_GDDR5; break;
                default:
                    break;
            }
            
            //dev_priv->vram_rank_B = !!(nv_rd32(dev, 0x100200) & 0x4);
            vram_size  = nv_rd32(0x10020c);
            vram_size |= (vram_size & 0xff) << 32;
            vram_size &= 0xffffffff00ULL;
            break;
        }
        case 0xc0:
        case 0xd0:
        case 0xe0:
            nvc0_get_vram();
            break;
    }
}

// GPIO ===

UInt8 * GeForceX::dcb_table()
{
	UInt8 *dcb = NULL;
    
	if (card_type > NV_04)
		dcb = (UInt8 *)ROMPTR(bios.data, bios.data[0x36]);
	if (!dcb) {
		HWSensorsWarningLog("no DCB data found in VBIOS");
		return NULL;
	}
    
	if (dcb[0] >= 0x41) {
		HWSensorsWarningLog("DCB version 0x%02x unknown", dcb[0]);
		return NULL;
	} else
        if (dcb[0] >= 0x30) {
            if (ROM32(dcb[6]) == 0x4edcbdcb)
                return dcb;
        } else
            if (dcb[0] >= 0x20) {
                if (ROM32(dcb[4]) == 0x4edcbdcb)
                    return dcb;
            } else
                if (dcb[0] >= 0x15) {
                    if (!memcmp(&dcb[-7], "DEV_REC", 7))
                        return dcb;
                } else {
                    /*
                     * v1.4 (some NV15/16, NV11+) seems the same as v1.5, but
                     * always has the same single (crt) entry, even when tv-out
                     * present, so the conclusion is this version cannot really
                     * be used.
                     *
                     * v1.2 tables (some NV6/10, and NV15+) normally have the
                     * same 5 entries, which are not specific to the card and so
                     * no use.
                     *
                     * v1.2 does have an I2C table that read_dcb_i2c_table can
                     * handle, but cards exist (nv11 in #14821) with a bad i2c
                     * table pointer, so use the indices parsed in
                     * parse_bmp_structure.
                     *
                     * v1.1 (NV5+, maybe some NV4) is entirely unhelpful
                     */
                    HWSensorsWarningLog("no useful DCB data in VBIOS");
                    return NULL;
                }
    
	HWSensorsWarningLog("DCB header validation failed");
	return NULL;
}

UInt8 * GeForceX::dcb_gpio_table()
{
	UInt8 *dcb = dcb_table();
	if (dcb) {
		if (dcb[0] >= 0x30 && dcb[1] >= 0x0c)
			return ROMPTR(bios.data, dcb[0x0a]);
		if (dcb[0] >= 0x22 && dcb[-1] >= 0x13)
			return ROMPTR(bios.data, dcb[-15]);
	}
	return NULL;
}

UInt8 * GeForceX::dcb_gpio_entry(int idx, int ent, UInt8 *version)
{
	UInt8 *table = dcb_gpio_table();
	if (table) {
		*version = table[0];
		if (*version < 0x30 && ent < table[2])
			return table + 3 + (ent * table[1]);
		else if (ent < table[2])
			return table + table[1] + (ent * table[3]);
	}
	return NULL;
}

bool GeForceX::nouveau_gpio_find(int idx, UInt8 func, UInt8 line, struct NVGpioFunc *gpio)
{
	UInt8 *table, *entry, version;
	int i = -1;
    
	if (line == 0xff && func == 0xff)
		return false;
    
	while ((entry = dcb_gpio_entry(idx, ++i, &version))) {
		if (version < 0x40) {
			UInt16 data = ROM16(entry[0]);           
			(*gpio).line = (data & 0x001f) >> 0;
			(*gpio).func = (data & 0x07e0) >> 5;
			(*gpio).log[0] = (data & 0x1800) >> 11;
			(*gpio).log[1] = (data & 0x6000) >> 13;
		} else {
            if (version < 0x41) {
                (*gpio).line = entry[0] & 0x1f;
                (*gpio).func = entry[1];
                (*gpio).log[0] = (entry[3] & 0x18) >> 3;
                (*gpio).log[1] = (entry[3] & 0x60) >> 5;
                
            } else {
                (*gpio).line = entry[0] & 0x3f;
                (*gpio).func = entry[1];
                (*gpio).log[0] = (entry[4] & 0x30) >> 4;
                (*gpio).log[1] = (entry[4] & 0xc0) >> 6;
                
            }
            
            if ((line == 0xff || line == gpio->line) &&
                (func == 0xff || func == gpio->func))
                return true;
        }
        
        /* DCB 2.2, fixed TVDAC GPIO data */
        if ((table = dcb_table()) && table[0] >= 0x22) {
            if (func == DCB_GPIO_TVDAC0) {
                (*gpio).func = DCB_GPIO_TVDAC0;
                (*gpio).line = table[-4] >> 4;
                (*gpio).log[0] = !!(table[-5] & 2);
                (*gpio).log[1] =  !(table[-5] & 2);
                
                return true;
            }
        }
    
	/* Apple iMac G4 NV18 */
	/*if (nv_match_device(dev, 0x0189, 0x10de, 0x0010)) {
		if (func == DCB_GPIO_TVDAC0) {
			*gpio = (struct gpio_func) {
				.func = DCB_GPIO_TVDAC0,
				.line = 4,
				.log[0] = 0,
				.log[1] = 1,
			};
			return 0;
		}*/
    }
    
	return false;
}

UInt32 GeForceX::NVReadCRTC(int head, UInt32 reg)
{
	if (head)
		reg += NV_PCRTC0_SIZE;
    
	return nv_rd32(reg);
}

int GeForceX::nv10_gpio_sense(int line)
{
	if (line < 2) {
		line = line * 16;
		line = NVReadCRTC(0, NV_PCRTC_GPIO) >> line;
		return !!(line & 0x0100);
	} else
        if (line < 10) {
            line = (line - 2) * 4;
            line = NVReadCRTC(0, NV_PCRTC_GPIO_EXT) >> line;
            return !!(line & 0x04);
        } else
            if (line < 14) {
                line = (line - 10) * 4;
                line = NVReadCRTC(0, NV_PCRTC_850) >> line;
                return !!(line & 0x04);
            }
    
	return 0;
}

static inline bool nv50_gpio_location(int line, UInt32 *reg, UInt32 *shift)
{
	const UInt32 nv50_gpio_reg[4] = { 0xe104, 0xe108, 0xe280, 0xe284 };
    
	if (line >= 32)
		return false;
    
	*reg = nv50_gpio_reg[line >> 3];
	*shift = (line & 7) << 2;
    
	return true;
}

int GeForceX::nv50_gpio_sense(int line)
{
	UInt32 reg, shift;
    
	if (!nv50_gpio_location(line, &reg, &shift))
		return 0;
    
	return !!(nv_rd32(reg) & (4 << shift));
}

int GeForceX::nvd0_gpio_sense(int line)
{
	return !!(nv_rd32(0x00d610 + (line * 4)) & 0x00004000);
}

// Fan ===

int GeForceX::nouveau_gpio_sense(int idx, int line)
{
    switch (chipset & 0xf0) {
        case 0x40:
        case 0x60:
            return nv10_gpio_sense(line);
            break;
        case 0x50:
        case 0x80:
        case 0x90:
        case 0xa0:
        case 0xc0:
            return nv50_gpio_sense(line);
        case 0xd0:
        case 0xe0:
            return nvd0_gpio_sense(line);
            break;
    }
    
	return 0;
}

int GeForceX::nouveau_gpio_get(int idx, UInt8 tag, UInt8 line)
{
	struct NVGpioFunc gpio;
	int ret = 0;
    
	if (nouveau_gpio_find(idx, tag, line, &gpio)) {
		ret = nouveau_gpio_sense(idx, gpio.line);
		if (ret >= 0)
			ret = (ret == (gpio.log[1] & 1));
	}
    
	return ret;
}

bool GeForceX::nv40_pm_pwm_get(int line, UInt32 *divs, UInt32 *duty)
{
	if (line == 2) {
		UInt32 reg = nv_rd32(0x0010f0);
		if (reg & 0x80000000) {
			*duty = (reg & 0x7fff0000) >> 16;
			*divs = (reg & 0x00007fff);
			return true;
		}
	} else
        if (line == 9) {
            UInt32 reg = nv_rd32(0x0015f4);
            if (reg & 0x80000000) {
                *divs = nv_rd32(0x0015f8);
                *duty = (reg & 0x7fffffff);
                return true;
            }
        } else {
            HWSensorsWarningLog("unknown pwm ctrl for gpio %d", line);
            return false;
        }
    
	return false;
}

static inline bool pwm_info(int *line, int *ctrl, int *indx)
{
	if (*line == 0x04) {
		*ctrl = 0x00e100;
		*line = 4;
		*indx = 0;
	} else
        if (*line == 0x09) {
            *ctrl = 0x00e100;
            *line = 9;
            *indx = 1;
        } else
            if (*line == 0x10) {
                *ctrl = 0x00e28c;
                *line = 0;
                *indx = 0;
            } else {
                return false;
            }
    
	return true;
}

bool GeForceX::nv50_pm_pwm_get(int line, UInt32 *divs, UInt32 *duty)
{
	int ctrl, id;
    
	if (!pwm_info(&line, &ctrl, &id))
		return false;
    
	if (nv_rd32(ctrl) & (1 << line)) {
		*divs = nv_rd32(0x00e114 + (id * 8));
		*duty = nv_rd32(0x00e118 + (id * 8));
		return true;
	}
    
	return false;
}

int GeForceX::nouveau_pwmfan_get()
{
	struct NVGpioFunc gpio;
	UInt32 divs, duty;
    
    switch (chipset & 0xf0) {
        case 0x40:
        case 0x60:
        case 0x50:
        case 0x80:
        case 0x90:
        case 0xa0:
        case 0xc0:
        case 0xd0:
            break;
        default:
            return 0;
    }
    
	if (nouveau_gpio_find(0, DCB_GPIO_PWM_FAN, 0xff, &gpio)) {
        
        bool ret = false;
        
        switch (chipset & 0xf0) {
            case 0x40:
            case 0x60:
                ret = nv40_pm_pwm_get(gpio.line, &divs, &duty);
                break;
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
            case 0xc0:
            case 0xd0:
                ret = nv50_pm_pwm_get(gpio.line, &divs, &duty);
                break;
        }
        
		if (ret && divs) {
			divs = max(divs, duty);
			if (card_type <= NV_40 || (gpio.log[0] & 1))
				duty = divs - duty;
			return (duty * 100) / divs;
		}
        
		return nouveau_gpio_get(0, gpio.func, 0xff) * 100;
	}
    
	return 0;
}

float GeForceX::nouveau_rpmfan_get(UInt32 milliseconds)
{
	struct NVGpioFunc gpio;
	UInt32 cycles, cur, prev;
	    
	if (nouveau_gpio_find(0, DCB_GPIO_FAN_SENSE, 0xff, &gpio)) {    
        /* Monitor the GPIO input 0x3b for 250ms.
         * When the fan spins, it changes the value of GPIO FAN_SENSE.
         * We get 4 changes (0 -> 1 -> 0 -> 1 -> [...]) per complete rotation.
         */
        
        clock_sec_t secs, end_secs; 
        clock_nsec_t nanosecs, end_nanosecs;
        
        clock_get_system_nanotime(&secs, &nanosecs);
        
        end_secs = secs;
		end_nanosecs = nanosecs + milliseconds * 1e6;
		if (end_nanosecs >= 1e9) {		
			end_secs++;			
			end_nanosecs -= 1e9;
		}
        
        prev = nouveau_gpio_sense(0, gpio.line);
        cycles = 0;
        
        do {
            cur = nouveau_gpio_sense(0, gpio.line);
            if (prev != cur) {
                cycles++;
                prev = cur;
            }
            
            //IOSleep(1); /* supports 0 < rpm < 7500 */
            IODelay(500); /* supports 0 < rpm < 7500 */

            clock_get_system_nanotime(&secs, &nanosecs);  
            
        } while (secs == end_secs ? nanosecs < end_nanosecs : secs < end_secs);
        
        /* interpolate to get rpm */
        return cycles / 4.0f * (1000.0f / milliseconds) * 60.0f;
    }
    
    return 0;
}


// NV04 ===

//

// NV40 ===

UInt32 GeForceX::nv40_read_pll_1(UInt32 reg)
{
	UInt32 ctrl = nv_rd32(reg + 0x00);
	int P = (ctrl & 0x00070000) >> 16;
	int N = (ctrl & 0x0000ff00) >> 8;
	int M = (ctrl & 0x000000ff) >> 0;
	UInt32 ref = 27000, clk = 0;
    
	if (ctrl & 0x80000000)
		clk = ref * N / M;
    
	return clk >> P;
}

UInt32 GeForceX::nv40_read_pll_2(UInt32 reg)
{
	UInt32 ctrl = nv_rd32(reg + 0x00);
	UInt32 coef = nv_rd32(reg + 0x04);
	int N2 = (coef & 0xff000000) >> 24;
	int M2 = (coef & 0x00ff0000) >> 16;
	int N1 = (coef & 0x0000ff00) >> 8;
	int M1 = (coef & 0x000000ff) >> 0;
	int P = (ctrl & 0x00070000) >> 16;
	UInt32 ref = 27000, clk = 0;
    
	if ((ctrl & 0x80000000) && M1) {
		clk = ref * N1 / M1;
		if ((ctrl & 0x40000100) == 0x40000000) {
			if (M2)
				clk = clk * N2 / M2;
			else
				clk = 0;
		}
	}
    
	return clk >> P;
}

UInt32 GeForceX::nv40_read_clk(UInt32 src)
{
	switch (src) {
        case 3:
            return nv40_read_pll_2(0x004000);
        case 2:
            return nv40_read_pll_1(0x004008);
        default:
            break;
	}
    
	return 0;
}

UInt32 GeForceX::nv40_get_clock(NVClockSource name)
{
	UInt32 clocks = 0;
    UInt32 ctrl = nv_rd32(0x00c040);
    
    switch (name) {
        case NVClockCore:
            clocks = nv40_read_clk((ctrl & 0x00000003) >> 0);
            break;
            
        case NVClockShader:
            clocks = nv40_read_clk((ctrl & 0x00000030) >> 4);
            break;
                        
        case NVCLockMemory:
            clocks = nv40_read_pll_2(0x4020);
            break;
    }
    
	return clocks / 1e3;
}

// NVA3 ===

UInt32 GeForceX::nva3_read_vco(int clk)
{
	UInt32 sctl = nv_rd32(0x4120 + (clk * 4));
	if ((sctl & 0x00000030) != 0x00000030)
		return nva3_read_pll(0x41, 0x00e820);
	return nva3_read_pll(0x42, 0x00e8a0);
}

UInt32 GeForceX::nva3_read_clk(int clk, bool ignore_en)
{
	UInt32 sctl, sdiv, sclk;
    
	/* refclk for the 0xe8xx plls is a fixed frequency */
	if (clk >= 0x40) {
		if (chipset == 0xaf) {
			/* no joke.. seriously.. sigh.. */
			return nv_rd32(0x00471c) * 1000;
		}
        
		return crystal;
	}
    
	sctl = nv_rd32(0x4120 + (clk * 4));
	if (!ignore_en && !(sctl & 0x00000100))
		return 0;
    
	switch (sctl & 0x00003000) {
        case 0x00000000:
            return crystal;
        case 0x00002000:
            if (sctl & 0x00000040)
                return 108000;
            return 100000;
        case 0x00003000:
            sclk = nva3_read_vco(clk);
            sdiv = ((sctl & 0x003f0000) >> 16) + 2;
            return (sclk * 2) / sdiv;
        default:
            return 0;
	}
}

UInt32 GeForceX::nva3_read_pll(int clk, UInt32 pll)
{
	UInt32 ctrl = nv_rd32(pll + 0);
	UInt32 sclk = 0, P = 1, N = 1, M = 1;
    
	if (!(ctrl & 0x00000008)) {
		if (ctrl & 0x00000001) {
			UInt32 coef = nv_rd32(pll + 4);
			M = (coef & 0x000000ff) >> 0;
			N = (coef & 0x0000ff00) >> 8;
			P = (coef & 0x003f0000) >> 16;
            
			/* no post-divider on these.. */
			if ((pll & 0x00ff00) == 0x00e800)
				P = 1;
            
			sclk = nva3_read_clk(0x00 + clk, false);
		}
	} else {
		sclk = nva3_read_clk(0x10 + clk, false);
	}
    
	if (M * P)
		return sclk * N / (M * P);
    
	return 0;
}

UInt32 GeForceX::nva3_get_clock(NVClockSource name)
{
	UInt32 clocks = 0;
    
    switch (name) {
        case NVClockCore:
            clocks = nva3_read_pll(0x00, 0x4200);
            break;
            
        case NVClockShader:
            clocks = nva3_read_pll(0x01, 0x4220);
            break;
                       
        case NVClockCopy:
            clocks = nva3_read_pll(0x00, 0x4200);
            break;
            
        case NVClockDaemon:
            clocks = nva3_read_clk(0x25, false);
            break;
            
        case NVClockVdec:
            clocks = nva3_read_clk(0x21, false);
            break;
            
        case NVCLockMemory:
            clocks = nva3_read_pll(0x02, 0x4000);
            break;
    }
    
	return clocks / 1e3;
}

// NV50 ===

UInt32 GeForceX::nv50_read_div()
{
	switch (chipset) {
        case 0x50: /* it exists, but only has bit 31, not the dividers.. */
        case 0x84:
        case 0x86:
        case 0x98:
        case 0xa0:
            return nv_rd32(0x004700);
        case 0x92:
        case 0x94:
        case 0x96:
            return nv_rd32(0x004800);
        default:
            return 0x00000000;
	}
}

UInt32 GeForceX::nv50_read_pll_src(UInt32 base)
{
	UInt32 coef, ref = nv50_read_clk(nv50_clk_src_crystal);
	UInt32 rsel = nv_rd32(0x00e18c);
	int P, N, M = 0, id = 0;
    
	switch (chipset) {
        case 0x50:
        case 0xa0:
            switch (base) {
                case 0x4020:
                case 0x4028: id = !!(rsel & 0x00000004); break;
                case 0x4008: id = !!(rsel & 0x00000008); break;
                case 0x4030: id = 0; break;
                default:
                    HWSensorsWarningLog("ref: bad pll 0x%06x", base);
                    return 0;
            }
            
            coef = nv_rd32(0x00e81c + (id * 0x0c));
            ref *=  (coef & 0x01000000) ? 2 : 4;
            P    =  (coef & 0x00070000) >> 16;
            N    = ((coef & 0x0000ff00) >> 8) + 1;
            M    = ((coef & 0x000000ff) >> 0) + 1;
            break;
        case 0x84:
        case 0x86:
        case 0x92:
            coef = nv_rd32(0x00e81c);
            P    = (coef & 0x00070000) >> 16;
            N    = (coef & 0x0000ff00) >> 8;
            M    = (coef & 0x000000ff) >> 0;
            break;
        case 0x94:
        case 0x96:
        case 0x98:
            rsel = nv_rd32(0x00c050);
            switch (base) {
                case 0x4020: rsel = (rsel & 0x00000003) >> 0; break;
                case 0x4008: rsel = (rsel & 0x0000000c) >> 2; break;
                case 0x4028: rsel = (rsel & 0x00001800) >> 11; break;
                case 0x4030: rsel = 3; break;
                default:
                    HWSensorsWarningLog("ref: bad pll 0x%06x", base);
                    return 0;
            }
            
            switch (rsel) {
                case 0: id = 1; break;
                case 1: return nv50_read_clk(nv50_clk_src_crystal);
                case 2: return nv50_read_clk(nv50_clk_src_href);
                case 3: id = 0; break;
            }
            
            coef =  nv_rd32(0x00e81c + (id * 0x28));
            P    = (nv_rd32(0x00e824 + (id * 0x28)) >> 16) & 7;
            P   += (coef & 0x00070000) >> 16;
            N    = (coef & 0x0000ff00) >> 8;
            M    = (coef & 0x000000ff) >> 0;
            break;
//        default:
//            BUG_ON(1);
	}
    
	if (M)
        return (ref * N / M) >> P;
    
	return 0;
}

UInt32 GeForceX::nv50_read_pll_ref(UInt32 base)
{
	UInt32 src, mast = nv_rd32(0x00c040);
    
	switch (base) {
        case 0x004028:
            src = !!(mast & 0x00200000);
            break;
        case 0x004020:
            src = !!(mast & 0x00400000);
            break;
        case 0x004008:
            src = !!(mast & 0x00010000);
            break;
        case 0x004030:
            src = !!(mast & 0x02000000);
            break;
        case 0x00e810:
            return nv50_read_clk(nv50_clk_src_crystal);
        default:
            HWSensorsWarningLog("bad pll 0x%06x", base);
            return 0;
	}
    
	if (src)
		return nv50_read_clk(nv50_clk_src_href);
    
	return nv50_read_pll_src(base);
}

UInt32 GeForceX::nv50_read_pll(UInt32 base)
{
	UInt32 mast = nv_rd32(0x00c040);
	UInt32 ctrl = nv_rd32(base + 0);
	UInt32 coef = nv_rd32(base + 4);
	UInt32 ref = nv50_read_pll_ref(base);
	UInt32 clk = 0;
	int N1, N2, M1, M2;
    
	if (base == 0x004028 && (mast & 0x00100000)) {
		/* wtf, appears to only disable post-divider on nva0 */
		if (chipset != 0xa0)
			return nv50_read_clk(nv50_clk_src_dom6);
	}
    
	N2 = (coef & 0xff000000) >> 24;
	M2 = (coef & 0x00ff0000) >> 16;
	N1 = (coef & 0x0000ff00) >> 8;
	M1 = (coef & 0x000000ff);
	if ((ctrl & 0x80000000) && M1) {
		clk = ref * N1 / M1;
		if ((ctrl & 0x40000100) == 0x40000000) {
			if (M2)
				clk = clk * N2 / M2;
			else
				clk = 0;
		}
	}
    
	return clk;
}

UInt32 GeForceX::nv50_read_clk(NV50ClockSource source)
{
	UInt32 mast = nv_rd32(0x00c040);
	UInt32 P = 0;
    
	switch (source) {
        case nv50_clk_src_crystal:
            return crystal;
        case nv50_clk_src_href:
            return 100000; /* PCIE reference clock */
        case nv50_clk_src_hclk:
            return nv50_read_clk(nv50_clk_src_href) * 27778 / 10000;
        case nv50_clk_src_hclkm3:
            return nv50_read_clk(nv50_clk_src_hclk) * 3;
        case nv50_clk_src_hclkm3d2:
            return nv50_read_clk(nv50_clk_src_hclk) * 3 / 2;
        case nv50_clk_src_host:
            switch (mast & 0x30000000) {
                case 0x00000000: return nv50_read_clk(nv50_clk_src_href);
                case 0x10000000: break;
                case 0x20000000: /* !0x50 */
                case 0x30000000: return nv50_read_clk(nv50_clk_src_hclk);
            }
            break;
        case nv50_clk_src_nvclk:
            if (!(mast & 0x00100000))
                P = (nv_rd32(0x004028) & 0x00070000) >> 16;
            switch (mast & 0x00000003) {
                case 0x00000000: return nv50_read_clk(nv50_clk_src_crystal) >> P;
                case 0x00000001: return nv50_read_clk(nv50_clk_src_dom6);
                case 0x00000002: return nv50_read_pll(0x004020) >> P;
                case 0x00000003: return nv50_read_pll(0x004028) >> P;
            }
            break;
        case nv50_clk_src_sclk:
            P = (nv_rd32(0x004020) & 0x00070000) >> 16;
            switch (mast & 0x00000030) {
                case 0x00000000:
                    if (mast & 0x00000080)
                        return nv50_read_clk(nv50_clk_src_host) >> P;
                    return nv50_read_clk(nv50_clk_src_crystal) >> P;
                case 0x00000010: break;
                case 0x00000020: return nv50_read_pll(0x004028) >> P;
                case 0x00000030: return nv50_read_pll(0x004020) >> P;
            }
            break;
        case nv50_clk_src_mclk:
            P = (nv_rd32(0x004008) & 0x00070000) >> 16;
            if (nv_rd32(0x004008) & 0x00000200) {
                switch (mast & 0x0000c000) {
                    case 0x00000000:
                        return nv50_read_clk(nv50_clk_src_crystal) >> P;
                    case 0x00008000:
                    case 0x0000c000:
                        return nv50_read_clk(nv50_clk_src_href) >> P;
                }
            } else {
                return nv50_read_pll(0x004008) >> P;
            }
            break;
        case nv50_clk_src_vdec:
            P = (nv50_read_div() & 0x00000700) >> 8;
            switch (chipset) {
                case 0x84:
                case 0x86:
                case 0x92:
                case 0x94:
                case 0x96:
                case 0xa0:
                    switch (mast & 0x00000c00) {
                        case 0x00000000:
                            if (chipset == 0xa0) /* wtf?? */
                                return nv50_read_clk(nv50_clk_src_nvclk) >> P;
                            return nv50_read_clk(nv50_clk_src_crystal) >> P;
                        case 0x00000400:
                            return 0;
                        case 0x00000800:
                            if (mast & 0x01000000)
                                return nv50_read_pll(0x004028) >> P;
                            return nv50_read_pll(0x004030) >> P;
                        case 0x00000c00:
                            return nv50_read_clk(nv50_clk_src_nvclk) >> P;
                    }
                    break;
                case 0x98:
                    switch (mast & 0x00000c00) {
                        case 0x00000000:
                            return nv50_read_clk(nv50_clk_src_nvclk) >> P;
                        case 0x00000400:
                            return 0;
                        case 0x00000800:
                            return nv50_read_clk(nv50_clk_src_hclkm3d2) >> P;
                        case 0x00000c00:
                            return nv50_read_clk(nv50_clk_src_mclk) >> P;
                    }
                    break;
            }
            break;
        case nv50_clk_src_dom6:
            switch (chipset) {
                case 0x50:
                case 0xa0:
                    return nv50_read_pll(0x00e810) >> 2;
                case 0x84:
                case 0x86:
                case 0x92:
                case 0x94:
                case 0x96:
                case 0x98:
                    P = (nv50_read_div() & 0x00000007) >> 0;
                    switch (mast & 0x0c000000) {
                        case 0x00000000: return nv50_read_clk(nv50_clk_src_href);
                        case 0x04000000: break;
                        case 0x08000000: return nv50_read_clk(nv50_clk_src_hclk);
                        case 0x0c000000:
                            return nv50_read_clk(nv50_clk_src_hclkm3) >> P;
                    }
                    break;
                default:
                    break;
            }
        default:
            break;
	}
    
	HWSensorsWarningLog("unknown clock source %d 0x%08x", source, mast);
    
	return 0;
}

UInt32 GeForceX::nv50_get_clock(NVClockSource name)
{    
    if (chipset == 0xaa || chipset == 0xac)
		return 0;
    
    UInt32 clocks = 0;
    
    switch (name) {
        case NVClockCore:
            clocks = nv50_read_clk(nv50_clk_src_nvclk);
            break;
            
        case NVClockShader:
            clocks = nv50_read_clk(nv50_clk_src_sclk);
            break;
            
        case NVClockVdec:
            if (chipset != 0x50)
                clocks = nv50_read_clk(nv50_clk_src_vdec);
            break;
            
        case NVCLockMemory:
            clocks = nv50_read_clk(nv50_clk_src_mclk);
            break;
    }
    
	return clocks / 1e3;
}

// NVC0 ===

UInt32 GeForceX::nvc0_read_vco(UInt32 dsrc)
{
	UInt32 ssrc = nv_rd32(dsrc);
	if (!(ssrc & 0x00000100))
		return nvc0_read_pll(0x00e800);
	return nvc0_read_pll(0x00e820);
}

UInt32 GeForceX::nvc0_read_pll(UInt32 pll)
{
	UInt32 ctrl = nv_rd32(pll + 0);
	UInt32 coef = nv_rd32(pll + 4);
	UInt32 P = (coef & 0x003f0000) >> 16;
	UInt32 N = (coef & 0x0000ff00) >> 8;
	UInt32 M = (coef & 0x000000ff) >> 0;
	UInt32 sclk, doff;
    
	if (!(ctrl & 0x00000001))
		return 0;
    
	switch (pll & 0xfff000) {
        case 0x00e000:
            sclk = 27000;
            P = 1;
            break;
        case 0x137000:
            doff = (pll - 0x137000) / 0x20;
            sclk = nvc0_read_div(doff, 0x137120, 0x137140);
            break;
        case 0x132000:
            switch (pll) {
                case 0x132000:
                    sclk = nvc0_read_pll(0x132020);
                    break;
                case 0x132020:
                    sclk = nvc0_read_div(0, 0x137320, 0x137330);
                    break;
                default:
                    return 0;
            }
            break;
        default:
            return 0;
	}
    
	return sclk * N / M / P;
}

UInt32 GeForceX::nvc0_read_div(UInt32 doff, UInt32 dsrc, UInt32 dctl)
{
	UInt32 ssrc = nv_rd32(dsrc + (doff * 4));
	UInt32 sctl = nv_rd32(dctl + (doff * 4));
    
	switch (ssrc & 0x00000003) {
        case 0:
            if ((ssrc & 0x00030000) != 0x00030000)
                return 27000;
            return 108000;
        case 2:
            return 100000;
        case 3:
            if (sctl & 0x80000000) {
                UInt32 sclk = nvc0_read_vco(dsrc + (doff * 4));
                UInt32 sdiv = (sctl & 0x0000003f) + 2;
                return (sclk * 2) / sdiv;
            }
            
            return nvc0_read_vco(dsrc + (doff * 4));            
	}
    return 0;
}

UInt32 GeForceX::nvc0_read_mem()
{
	UInt32 ssel = nv_rd32(0x1373f0);
    
	if (ssel & 0x00000001)
        return nvc0_read_div(0, 0x137300, 0x137310); // sould i devide this value too?
    
	return nvc0_read_pll(0x132000) / 2; // is it correct divisor for all c0 cards? 
}

UInt32 GeForceX::nvc0_read_clk(UInt32 clk)
{
	UInt32 sctl = nv_rd32(0x137250 + (clk * 4));
	UInt32 ssel = nv_rd32(0x137100);
	UInt32 sclk, sdiv;
    
	if (ssel & (1 << clk)) {
		if (clk < 7)
			sclk = nvc0_read_pll(0x137000 + (clk * 0x20));
		else
			sclk = nvc0_read_pll(0x1370e0);
		sdiv = ((sctl & 0x00003f00) >> 8) + 2;
	} else {
		sclk = nvc0_read_div(clk, 0x137160, 0x1371d0);
		sdiv = ((sctl & 0x0000003f) >> 0) + 2;
	}
    
	if (sctl & 0x80000000)
		return (sclk * 2) / sdiv;
	return sclk;
}

UInt32 GeForceX::nvc0_get_clock(NVClockSource name)
{
    UInt32 clocks = 0;
    
    switch (name) {
        case NVClockCore:
            clocks = nvc0_read_clk(0x00) / 2;
            break;
            
        case NVClockShader:
            clocks = nvc0_read_clk(0x00);
            break;
        
        case NVClockRop:
            clocks = nvc0_read_clk(0x01);
            break;
        
        case NVClockCopy:
            clocks = nvc0_read_clk(0x09);
            break;
            
        case NVClockDaemon:
            clocks = nvc0_read_clk(0x0c);
            break;
        
        case NVClockVdec:
            clocks = nvc0_read_clk(0x0e);
            break;
        
        case NVCLockMemory:
            clocks = nvc0_read_mem();
            break;
    }

	return clocks / 1e3;
}

// Temperatures ===

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
    
	nv_wr32(0x0015b0, sensor_calibration);
    
	/* Wait for the sensor to update */
	IOSleep(5);
    
	/* read */
	return nv_rd32(0x0015b4) & 0x1fff;
}

int GeForceX::nv40_get_temperature()
{
	int offset = sensor_constants.offset_mult / sensor_constants.offset_div;
	int core_temp;
    
	if (card_type >= NV_50) {
		core_temp = nv_rd32(0x20008);
	} else {
		core_temp = nv_rd32(0x0015b4) & 0x1fff;
		/* Setup the sensor if the temperature is 0 */
		if (core_temp == 0)
			core_temp = nv40_sensor_setup();
	}
    
	core_temp = core_temp * sensor_constants.slope_mult / sensor_constants.slope_div;
	core_temp = core_temp + offset + sensor_constants.offset_constant;
    
	return core_temp;
}

int GeForceX::nv84_get_temperature()
{
	return nv_rd32(0x20400);
}

void GeForceX::nouveau_temp_init()
{
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
                        
                    case 0x92:
                        //Warning! These parameters are not used by NVIDIA anywhere and came from ASUS SmartDoctor
                        //algorithm (which most likely uses roughly approximated calibration) so the result can be
                        //really (!!!) inaccurate!
                        
                        //ASUS SmartDoctor uses (-13115 + x) / 18.7 + 1 calibration equation
                        sensor_constants.offset_mult = -131150 /*+ 187*/;
                        sensor_constants.offset_div = 187;
                        sensor_constants.slope_mult = 10;
                        sensor_constants.slope_div = 187;
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

// VBIOS ===

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
		UInt64 addr = (UInt64)(nv_rd32(0x619f04) & 0xffffff00) << 8;
		if (!addr) {
			addr  = (UInt64)nv_rd32(0x001700) << 16;
			addr += 0xf0000;
		}
        
		bar0 = nv_mask(0x001700, 0xffffffff, addr >> 16);
	}
    
	/* bail if no rom signature */
	if (nv_rd08(NV_PRAMIN_OFFSET + 0) != 0x55 ||
	    nv_rd08(NV_PRAMIN_OFFSET + 1) != 0xaa)
		goto out;
    
	bios.length = nv_rd08(NV_PRAMIN_OFFSET + 2) * 512;
	bios.data = (uint8_t*)IOMalloc(bios.length);
	if (bios.data) {
		for (i = 0; i < bios.length; i++)
			bios.data[i] = nv_rd08(NV_PRAMIN_OFFSET + i);
	}
    
out:
	if (card_type >= NV_50)
		nv_wr32(0x001700, bar0);
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
    
	access = nv_mask(pcireg, 0x00000001, 0x00000000);
    
	/* bail if no rom signature, with a workaround for a PROM reading
	 * issue on some chipsets.  the first read after a period of
	 * inactivity returns the wrong result, so retry the first header
	 * byte a few times before giving up as a workaround
	 */
	i = 16;
	do {
		if (nv_rd08(NV_PROM_OFFSET + 0) == 0x55)
			break;
	} while (i--);
    
	if (!i || nv_rd08(NV_PROM_OFFSET + 1) != 0xaa)
		goto out;
    
	/* additional check (see note below) - read PCI record header */
	pcir = nv_rd08(NV_PROM_OFFSET + 0x18) |
    nv_rd08(NV_PROM_OFFSET + 0x19) << 8;
	if (nv_rd08(NV_PROM_OFFSET + pcir + 0) != 'P' ||
	    nv_rd08(NV_PROM_OFFSET + pcir + 1) != 'C' ||
	    nv_rd08(NV_PROM_OFFSET + pcir + 2) != 'I' ||
	    nv_rd08(NV_PROM_OFFSET + pcir + 3) != 'R')
		goto out;
    
	/* read entire bios image to system memory */
	bios.length = nv_rd08(NV_PROM_OFFSET + 2) * 512;
	bios.data = (uint8_t*)IOMalloc(bios.length);
	if (bios.data) {
		for (i = 0; i < bios.length; i++)
			bios.data[i] = nv_rd08(NV_PROM_OFFSET + i);
	}
    
out:
	/* disable access to rom */
	nv_wr32(pcireg, access);
}

void GeForceX::bios_shadow()
{
    //try to load bios from "vbios" property created by Chameleon boolloader
    
    if (OSData *vbios = OSDynamicCast(OSData, device->getProperty("vbios"))) {
        bios.length = vbios->getLength();
        bios.data = (UInt8 *)IOMalloc(bios.length);
        memcpy(bios.data, vbios->getBytesNoCopy(), bios.length);
    }

    if (3 != score_vbios(false)) {

        bios_shadow_pramin();        
        
        if (3 != score_vbios(true)) {
            
            bios_shadow_prom();
                        
            if (3 != score_vbios(false)) {
                HWSensorsWarningLog("failed to read VBIOS");
                
                /*if (!bios.data) {
                    bios.length = 65536;
                    bios.data = (UInt8 *)IOMalloc(bios.length);
                }*/
            } else HWSensorsInfoLog("VBIOS successfully read from PROM");
        } else HWSensorsInfoLog("VBIOS successfully read from PRAMIN");
    } else HWSensorsInfoLog("VBIOS successfully read from I/O registry");
    
    
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
    }
}

// Driver ===

/*IOReturn GeForceX::loopTimerEvent(void)
{
    if (fanCounter++ < 4) {
        fanRMP = nouveau_rpmfan_get(250);
    }
    
    timersource->setTimeoutMS(1000);
    
    return kIOReturnSuccess;
}*/

float GeForceX::getSensorValue(FakeSMCSensor *sensor)
{
    switch (sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            switch (chipset & 0xf0) {
                case 0x40:
                case 0x60:
                case 0x50:
                case 0x80:
                case 0x90:
                case 0xa0:
                    if (chipset >= 0x84 && chipset != 0x92)
                        return nv84_get_temperature();
                    else
                        return nv40_get_temperature();

                case 0xc0:
                case 0xd0:
                    return nv84_get_temperature();

            }
            break;
            
        case kFakeSMCFrequencySensor:
            switch (chipset & 0xf0) {
                case 0x40:
                case 0x60:
                    return nv40_get_clock((NVClockSource)sensor->getIndex());

                case 0x50:
                case 0x80:
                case 0x90:
                case 0xa0:
                    switch (chipset) {
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
                            return nv50_get_clock((NVClockSource)sensor->getIndex());

                        default:
                            return nva3_get_clock((NVClockSource)sensor->getIndex());
                    }
                    break;
                case 0xc0:
                case 0xd0:
                    return nvc0_get_clock((NVClockSource)sensor->getIndex());
            }
            break;
            
        case kGeForceXPWMSensor:
            return nouveau_pwmfan_get();
        
        case kFakeSMCTachometerSensor:
            return nouveau_rpmfan_get(250);
            
        case kFakeSMCVoltageSensor:
            return nouveau_voltage_get();
    }
    
    return 0;
}

/*IOService *GeForceX::probe(IOService *provider, SInt32 *score)
{
    HWSensorsDebugLog("Probing...");
    
    if (super::probe(provider, score) != this) 
        return 0;
    
    if (!(workloop = getWorkLoop())) 
		return 0;
	
	if (!(timersource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &GeForceX::loopTimerEvent)))) 
		return 0;
	
	if (kIOReturnSuccess != workloop->addEventSource(timersource))
		return 0;
        
    return this;
}*/

bool GeForceX::start(IOService * provider)
{
	HWSensorsDebugLog("Starting...");
	
	if (!super::start(provider)) 
        return false;
    
    if ((device = (IOPCIDevice*)provider)) {
        
        device->setMemoryEnable(true);
        
        if ((mmio = device->mapDeviceMemoryWithIndex(0))) {
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

    
    UInt32 reg0 = nv_rd32(NV03_PMC_BOOT_0);
    
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
        }
    } else if ((reg0 & 0xff00fff0) == 0x20004000) {
        if (reg0 & 0x00f00000)
            chipset = 0x05;
        else
            chipset = 0x04;
        
        card_type = NV_04;
    }
    
    if (card_type) {
        
        if (OSData * data = OSDynamicCast(OSData, device->getProperty("device-id")))
            device_id = *(UInt16*)data->getBytesNoCopy(0, 2);
        
        if (OSData * data = OSDynamicCast(OSData, device->getProperty("vendor-id")))
            vendor_id = *(UInt16*)data->getBytesNoCopy(0, 2);
        
        /* determine frequency of timing crystal */
        UInt32 strap = nv_rd32(0x101000);
        if (chipset < 0x17 || (chipset >= 0x20 && chipset <= 0x25))
            strap &= 0x00000040;
        else
            strap &= 0x00400040;
        
        switch (strap) {
            case 0x00000000: crystal = 13500; break;
            case 0x00000040: crystal = 14318; break;
            case 0x00400000: crystal = 27000; break;
            case 0x00400040: crystal = 25000; break;
        }
        
        HWSensorsDebugLog("crystal freq: %u KHz", crystal);
        
        bios_shadow();
        
        nouveau_vram_init();
        nouveau_volt_init();
        nouveau_temp_init();
        
        HWSensorsInfoLog("detected an NV%2X generation card (0x%08x) with %lldMib of %s (%d)", card_type, reg0, vram_size / 1024 / 1024, NVVRAMTypeMap[(int)vram_type].name, vram_type);
        
        //Setup sensors
        
        //Find card number
        SInt8 cardIndex = getVacantGPUIndex();
        
        if (cardIndex < 0) {
            HWSensorsWarningLog("failed to obtain vacant GPU index");
            return false;
        }
        
        char key[5];
        
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
                addSensor(key, TYPE_SP78, 2, kFakeSMCTemperatureSensor, 0);
                break;
        }
        
        //Clocks
        switch (chipset & 0xf0) {
            case 0x40:
            case 0x60:
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_FREQUENCY, cardIndex);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockCore);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_SHADER_FREQUENCY, cardIndex);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockShader);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_MEMORY_FREQUENCY, cardIndex);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVCLockMemory);
                break;
            case 0xc0:
            case 0xd0:
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_FREQUENCY, cardIndex);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockCore);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_SHADER_FREQUENCY, cardIndex);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockShader);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_ROP_FREQUENCY, cardIndex);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVClockRop);
                
                snprintf(key, 5, KEY_FAKESMC_FORMAT_GPU_MEMORY_FREQUENCY, cardIndex);
                addSensor(key, TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCFrequencySensor, NVCLockMemory);
                break;
        }
        
        // Fans
        switch (chipset & 0xf0) {
            case 0x40:
            case 0x60:
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
            case 0xc0:
            case 0xd0: {
                char title[16]; 
                
                if (nouveau_pwmfan_get() > 0) {
                    snprintf(key, 5, KEY_FAKESMC_FORMAT_FAN_PWM, cardIndex);
                    addSensor(key, TYPE_UI8, TYPE_UI8_SIZE, kGeForceXPWMSensor, 0);
                }
                
                if (nouveau_rpmfan_get(100) > 0) {
                    snprintf (title, 16, "GPU %X", cardIndex);
                    addTachometer(1, title);
                }
                
                break;
            }
        }
        
        // Voltages
        switch (chipset & 0xf0) {
            case 0x30:
            case 0x40:
            case 0x60:
            case 0x50:
            case 0x80:
            case 0x90:
            case 0xa0:
            case 0xc0:
            case 0xd0:
                if (voltage.supported) {
                    snprintf(key, 5, KEY_FORMAT_GPU_VOLTAGE, cardIndex);
                    addSensor(key, TYPE_FP2E, TYPE_FPXX_SIZE, kFakeSMCVoltageSensor, 0);
                }
                break;
        }
        
        //loopTimerEvent();
        
        registerService();
        
        return true;
    }
    else HWSensorsWarningLog("NV%02X unsupported (chipset:0x%08x)", chipset, reg0);
    
    return false;
}

void GeForceX::free(void)
{
    if (bios.data) {
        IOFree(bios.data, bios.length);
        bios.data = 0;
        bios.length = 0;
    }
    
    if (mmio) {
        mmio->release();
        mmio = 0;
    }
    
    super::free();
}
