//
//  nouveau_bios.c
//  HWSensors
//
//  Created by Kozlek on 07.08.12.
//
//

/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */

#include "nouveau_bios.h"

#include "nouveau_definitions.h"
#include "nouveau.h"

u8 nv_ro08(struct nouveau_device *device, u32 addr)
{
	return device->bios.data[addr];
}

u16 nv_ro16(struct nouveau_device *device, u32 addr)
{
	return *(u16 *)&device->bios.data[addr];
}

u32 nv_ro32(struct nouveau_device *device, u32 addr)
{
	return *(u32 *)&device->bios.data[addr];
}

void nv_wo08(struct nouveau_device *device, u32 addr, u8 data)
{
	device->bios.data[addr] = data;
}

void nv_wo16(struct nouveau_device *device, u32 addr, u16 data)
{
	*(u16 *)&device->bios.data[addr] = data;
}

void nv_wo32(struct nouveau_device *device, u32 addr, u32 data)
{
	*(u16 *)&device->bios.data[addr] = data;
}

inline bool nv_strncmp(struct nouveau_device *device, u32 addr, u32 len, const char *str)
{
	while (len--) {
		if (nv_ro08(device, addr++) != *(str++))
			return false;
	}
	return true;
}

u16 nouveau_dcb_table(struct nouveau_device *device, u8 *ver, u8 *hdr, u8 *cnt, u8 *len)
{
	u16 dcb = 0x0000;
    
	if (device->card_type > NV_04)
		dcb = nv_ro16(device, 0x36);
	if (!dcb) {
		nv_warn(device, "DCB table not found\n");
		return dcb;
	}
    
	*ver = nv_ro08(device, dcb);
    
	if (*ver >= 0x41) {
		nv_warn(device, "DCB *ver 0x%02x unknown\n", *ver);
		return 0x0000;
	} else
        if (*ver >= 0x30) {
            if (nv_ro32(device, dcb + 6) == 0x4edcbdcb) {
                *hdr = nv_ro08(device, dcb + 1);
                *cnt = nv_ro08(device, dcb + 2);
                *len = nv_ro08(device, dcb + 3);
                return dcb;
            }
        } else
            if (*ver >= 0x20) {
                if (nv_ro32(device, dcb + 4) == 0x4edcbdcb) {
                    u16 i2c = nv_ro16(device, dcb + 2);
                    *hdr = 8;
                    *cnt = (i2c - dcb) / 8;
                    *len = 8;
                    return dcb;
                }
            } else
                if (*ver >= 0x15) {
                    if (!nv_strncmp(device, dcb - 7, 7, "DEV_REC")) {
                        u16 i2c = nv_ro16(device, dcb + 2);
                        *hdr = 4;
                        *cnt = (i2c - dcb) / 10;
                        *len = 10;
                        return dcb;
                    }
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
                    nv_warn(device, "DCB contains no useful data\n");
                    return 0x0000;
                }
    
	nv_warn(device, "DCB header validation failed\n");
	return 0x0000;
}

static u8 nvbios_checksum(const u8 *data, int size)
{
	u8 sum = 0;
	while (size--)
		sum += *data++;
	return sum;
}

static u16 nvbios_findstr(const u8 *data, int size, const char *str, int len)
{
	int i, j;
    
	for (i = 0; i <= (size - len); i++) {
		for (j = 0; j < len; j++)
			if ((char)data[i + j] != str[j])
				break;
		if (j == len)
			return i;
	}
    
	return 0;
}

int nouveau_bios_score(struct nouveau_device *device, const bool writeable)
{
	if (!device->bios.data || device->bios.data[0] != 0x55 || device->bios.data[1] != 0xAA) {
		nv_debug(device, "VBIOS signature not found\n");
		return 0;
	}
    
	if (nvbios_checksum((u8*)device->bios.data, device->bios.data[2] * 512)) {
		nv_debug(device, "VBIOS checksum invalid\n");
		/* if a ro image is somewhat bad, it's probably all rubbish */
		return writeable ? 2 : 1;
	}
    
	nv_debug(device, "VBIOS appears to be valid\n");
	return 3;
}

static void nouveau_bios_shadow_pramin(struct nouveau_device *device)
{
	u32 bar0 = 0;
	int i;
    
    nv_debug(device, "shadowing bios from PRAMIN\n");
    
	if (device->card_type >= NV_50) {
		u64 addr = (u64)(nv_rd32(device, 0x619f04) & 0xffffff00) << 8;
		if (!addr) {
			addr  = (u64)nv_rd32(device, 0x001700) << 16;
			addr += 0xf0000;
		}
        
		bar0 = nv_mask(device, 0x001700, 0xffffffff, addr >> 16);
	}
    
	/* bail if no rom signature */
	if (nv_rd08(device, 0x700000) != 0x55 ||
	    nv_rd08(device, 0x700001) != 0xaa)
		goto out;
    
	device->bios.size = nv_rd08(device, 0x700002) * 512;
	device->bios.data = (u8*)IOMalloc(device->bios.size);
    
	if (device->bios.data) {
		for (i = 0; i < device->bios.size; i++)
            //device->bios.data[i] = nv_rd08(device, 0x700000 + i);
			nv_wo08(device, i, nv_rd08(device, 0x700000 + i));
	}
    
out:
	if (device->card_type >= NV_50)
		nv_wr32(device, 0x001700, bar0);
}

static void nouveau_bios_shadow_prom(struct nouveau_device *device)
{
	u32 pcireg, access;
	u16 pcir;
	int i;
    
    nv_debug(device, "shadowing bios from PROM\n");
    
	/* enable access to rom */
	if (device->card_type >= NV_50)
		pcireg = 0x088050;
	else
		pcireg = 0x001850;
	access = nv_mask(device, pcireg, 0x00000001, 0x00000000);
    
	/* bail if no rom signature, with a workaround for a PROM reading
	 * issue on some chipsets.  the first read after a period of
	 * inactivity returns the wrong result, so retry the first header
	 * byte a few times before giving up as a workaround
	 */
	i = 16;
	do {
		if (nv_rd08(device, 0x300000) == 0x55)
			break;
	} while (i--);
    
	if (!i || nv_rd08(device, 0x300001) != 0xaa)
		goto out;
    
	/* additional check (see note below) - read PCI record header */
	pcir = nv_rd08(device, 0x300018) |
    nv_rd08(device, 0x300019) << 8;
	if (nv_rd08(device, 0x300000 + pcir) != 'P' ||
	    nv_rd08(device, 0x300001 + pcir) != 'C' ||
	    nv_rd08(device, 0x300002 + pcir) != 'I' ||
	    nv_rd08(device, 0x300003 + pcir) != 'R')
		goto out;
    
	/* read entire bios image to system memory */
	device->bios.size = nv_rd08(device, 0x300002) * 512;
	device->bios.data = (u8*)IOMalloc(device->bios.size);
	if (device->bios.data) {
		for (i = 0; i < device->bios.size; i++)
            //device->bios.data[i] = nv_rd08(device, 0x300000 + i);
			nv_wo08(device, i, nv_rd08(device, 0x300000 + i));
	}
    
out:
	/* disable access to rom */
	nv_wr32(device, pcireg, access);
}

void nouveau_vbios_init(struct nouveau_device *device)
{
    device->vbios.data = device->bios.data;
    device->vbios.length = device->bios.size;
}

bool nouveau_bios_shadow(struct nouveau_device *device)
{
    nv_debug(device, "trying to shadow bios\n");
    
    nouveau_bios_shadow_pramin(device);
    
    if (device->bios.data && nouveau_bios_score(device, true) > 1) {
        nv_debug(device, "VBIOS successfully read from PRAMIN\n");
        nouveau_vbios_init(device);
        return true;
    }
        
    nouveau_bios_shadow_prom(device);
    
    if (device->bios.data && nouveau_bios_score(device, true) > 1) {
        nv_debug(device, "VBIOS successfully read from PROM\n");
        nouveau_vbios_init(device);
        return true;
    }
    
    return false;
}

static inline u16
bmp_version(struct nouveau_device *device)
{
	if (device->bios.bmp_offset) {
		return nv_ro08(device, device->bios.bmp_offset + 5) << 8 |
        nv_ro08(device, device->bios.bmp_offset + 6);
	}
    
	return 0x0000;
}

int nouveau_bit_entry(struct nouveau_device *device, u8 id, struct bit_entry *bit)
{
	if (device->bios.bit_offset) {
		u8  entries = nv_ro08(device, device->bios.bit_offset + 10);
		u32 entry   = device->bios.bit_offset + 12;
		while (entries--) {
			if (nv_ro08(device, entry + 0) == id) {
				bit->id      = nv_ro08(device, entry + 0);
				bit->version = nv_ro08(device, entry + 1);
				bit->length  = nv_ro16(device, entry + 2);
				bit->offset  = nv_ro16(device, entry + 4);
				return 0;
			}
            
			entry += nv_ro08(device, device->bios.bit_offset + 9);
		}
        
		return -ENOENT;
	}
    
	return -EINVAL;
}

struct bit_table {
	const char id;
	int (* const parse_fn)(struct drm_device *, struct nvbios *, struct bit_entry *);
};

#define BIT_TABLE(id, funcid) ((struct bit_table){ id, parse_bit_##funcid##_tbl_entry })

int nouveau_bit_table(struct nouveau_device *device, u8 id, struct bit_entry *bit)
{
	struct nvbios *bios = &device->vbios;
	u8 entries, *entry;
    
	if (bios->type != NVBIOS_BIT)
		return -ENODEV;
    
	entries = bios->data[bios->offset + 10];
	entry   = &bios->data[bios->offset + 12];
	while (entries--) {
		if (entry[0] == id) {
			bit->id = entry[0];
			bit->version = entry[1];
			bit->length = ROM16(entry[2]);
			bit->offset = ROM16(entry[4]);
			bit->data = ROMPTR(device, entry[4]);
			return 0;
		}
        
		entry += bios->data[bios->offset + 9];
	}
    
	return -ENOENT;
}

static bool nouveau_parse_vbios_struct(struct nouveau_device *device)
{
	struct nvbios *bios = &device->vbios;
	const uint8_t bit_signature[] = { 0xff, 0xb8, 'B', 'I', 'T' };
	const uint8_t bmp_signature[] = { 0xff, 0x7f, 'N', 'V', 0x0 };
	int offset;
    
	offset = nvbios_findstr(bios->data, bios->length, (char*)bit_signature, sizeof(bit_signature));
	if (offset) {
		//nv_info(device, "BIT BIOS found\n");
		bios->type = NVBIOS_BIT;
		bios->offset = offset;
		return true; //parse_bit_structure(device, offset + 6);
	}
    
	offset = nvbios_findstr(bios->data, bios->length, (char*)bmp_signature, sizeof(bmp_signature));
	if (offset) {
		//nv_info(device, "BMP BIOS found\n");
		bios->type = NVBIOS_BMP;
		bios->offset = offset;
		return true; //parse_bmp_structure(device, bios, offset);
	}
    
	//nv_error(device, "No known BIOS signature found\n");
    
	return false;
}

void nouveau_bios_parse(struct nouveau_device *device)
{
    struct nouveau_bios *bios = &device->bios;
    
    if (!bios->data)
        return;
    
    nv_debug(device, "parsing VBIOS\n");
    
    /* detect type of vbios we're dealing with */
	bios->bmp_offset = nvbios_findstr((u8*)bios->data, bios->size,
                                      "\xff\x7f""NV\0", 5);
	if (bios->bmp_offset) {
		nv_info(device, "VBIOS BMP version %x.%x\n",
                bmp_version(device) >> 8,
                bmp_version(device) & 0xff);
	}
    
	bios->bit_offset = nvbios_findstr((u8*)bios->data, bios->size,
                                      "\xff\xb8""BIT", 5);
	if (bios->bit_offset)
		nv_debug(device, "VBIOS BIT signature found\n");
    
    struct bit_entry bit_i;
    
	/* determine the vbios version number */
	if (!nouveau_bit_entry(device, 'i', &bit_i) && bit_i.length >= 4) {
		bios->version.major = nv_ro08(device, bit_i.offset + 3);
		bios->version.chip  = nv_ro08(device, bit_i.offset + 2);
		bios->version.minor = nv_ro08(device, bit_i.offset + 1);
		bios->version.micro = nv_ro08(device, bit_i.offset + 0);
	} else
        if (bmp_version(device)) {
            bios->version.major = nv_ro08(device, bios->bmp_offset + 13);
            bios->version.chip  = nv_ro08(device, bios->bmp_offset + 12);
            bios->version.minor = nv_ro08(device, bios->bmp_offset + 11);
            bios->version.micro = nv_ro08(device, bios->bmp_offset + 10);
        }
    
	nv_debug(device, "VBIOS version %02x.%02x.%02x.%02x\n",
            bios->version.major, bios->version.chip,
            bios->version.minor, bios->version.micro);
    
    nouveau_parse_vbios_struct(device);
}

u16 nouveau_dcb_i2c_table(struct nouveau_device *device, u8 *ver, u8 *hdr, u8 *cnt, u8 *len)
{
	u16 i2c = 0x0000;
	u16 dcb = nouveau_dcb_table(device, ver, hdr, cnt, len);
	if (dcb) {
		if (*ver >= 0x15)
			i2c = nv_ro16(device, dcb + 2);
		if (*ver >= 0x30)
			i2c = nv_ro16(device, dcb + 4);
	}
    
	if (i2c && *ver >= 0x30) {
		*ver = nv_ro08(device, i2c + 0);
		*hdr = nv_ro08(device, i2c + 1);
		*cnt = nv_ro08(device, i2c + 2);
		*len = nv_ro08(device, i2c + 3);
	} else {
		//*ver = *ver; /* use DCB version */
		*hdr = 0;
		*cnt = 16;
		*len = 4;
	}
    
	return i2c;
}

static u16 nouveau_dcb_i2c_entry(struct nouveau_device *device, u8 idx, u8 *ver, u8 *len)
{
	u8  hdr, cnt;
	u16 i2c = nouveau_dcb_i2c_table(device, ver, &hdr, &cnt, len);
	if (i2c && idx < cnt)
		return i2c + hdr + (idx * *len);
	return 0x0000;
}

int nouveau_dcb_i2c_parse(struct nouveau_device *device, u8 idx, struct dcb_i2c_entry *info)
{
	u8  ver, len;
	u16 ent = nouveau_dcb_i2c_entry(device, idx, &ver, &len);
	if (ent) {
		info->data = nv_ro32(device, ent + 0);
		info->type = (dcb_i2c_type)nv_ro08(device, ent + 3);
		if (ver < 0x30) {
			info->type &= 0x07;
			if (info->type == 0x07)
				info->type = 0xff;
		}
        
		switch (info->type) {
            case DCB_I2C_NV04_BIT:
                info->drive = nv_ro08(device, ent + 0);
                info->sense = nv_ro08(device, ent + 1);
                return 0;
            case DCB_I2C_NV4E_BIT:
                info->drive = nv_ro08(device, ent + 1);
                return 0;
            case DCB_I2C_NVIO_BIT:
            case DCB_I2C_NVIO_AUX:
                info->drive = nv_ro08(device, ent + 0);
                return 0;
            case DCB_I2C_UNUSED:
                return 0;
            default:
                nv_warn(device, "unknown i2c type %d\n", info->type);
                info->type = DCB_I2C_UNUSED;
                return 0;
		}
	}
    
	if (device->bios.bmp_offset && idx < 2) {
		/* BMP (from v4.0 has i2c info in the structure, it's in a
		 * fixed location on earlier VBIOS
		 */
		if (nv_ro08(device, device->bios.bmp_offset + 5) < 4)
			ent = 0x0048;
		else
			ent = 0x0036 + device->bios.bmp_offset;
        
		if (idx == 0) {
			info->drive = nv_ro08(device, ent + 4);
			if (!info->drive) info->drive = 0x3f;
			info->sense = nv_ro08(device, ent + 5);
			if (!info->sense) info->sense = 0x3e;
		} else
            if (idx == 1) {
                info->drive = nv_ro08(device, ent + 6);
                if (!info->drive) info->drive = 0x37;
                info->sense = nv_ro08(device, ent + 7);
                if (!info->sense) info->sense = 0x36;
            }
        
		info->type = DCB_I2C_NV04_BIT;
		return 0;
	}
    
	return -ENOENT;
}

static u16 extdev_table(struct nouveau_device *device, u8 *ver, u8 *hdr, u8 *len, u8 *cnt)
{
	u8  dcb_ver, dcb_hdr, dcb_cnt, dcb_len;
	u16 dcb, extdev = 0;
    
	dcb = nouveau_dcb_table(device, &dcb_ver, &dcb_hdr, &dcb_cnt, &dcb_len);
	if (!dcb || (dcb_ver != 0x30 && dcb_ver != 0x40))
		return 0x0000;
    
	extdev = nv_ro16(device, dcb + 18);
	if (!extdev)
		return 0x0000;
    
	*ver = nv_ro08(device, extdev + 0);
	*hdr = nv_ro08(device, extdev + 1);
	*cnt = nv_ro08(device, extdev + 2);
	*len = nv_ro08(device, extdev + 3);
    
	return extdev + *hdr;
}

static u16 nvbios_extdev_entry(struct nouveau_device *device, int idx, u8 *ver, u8 *len)
{
	u8 hdr, cnt;
	u16 extdev = extdev_table(device, ver, &hdr, len, &cnt);
	if (extdev && idx < cnt)
		return extdev + idx * *len;
	return 0x0000;
}

static void extdev_parse_entry(struct nouveau_device *device, u16 offset, struct nvbios_extdev_func *entry)
{
	entry->type = nv_ro08(device, offset + 0);
	entry->addr = nv_ro08(device, offset + 1);
	entry->bus = (nv_ro08(device, offset + 2) >> 4) & 1;
}

int nvbios_extdev_parse(struct nouveau_device *device, int idx, struct nvbios_extdev_func *func)
{
	u8 ver, len;
	u16 entry;
    
	if (!(entry = nvbios_extdev_entry(device, idx, &ver, &len)))
		return -EINVAL;
    
	extdev_parse_entry(device, entry, func);
    
	return 0;
}

int nvbios_extdev_find(struct nouveau_device *device, enum nvbios_extdev_type type, struct nvbios_extdev_func *func)
{
	u8 ver, len, i;
	u16 entry;
    
	i = 0;
	while (!(entry = nvbios_extdev_entry(device, i++, &ver, &len))) {
		extdev_parse_entry(device, entry, func);
		if (func->type == type)
			return 0;
	}
    
	return -EINVAL;
}
