//
//  atom.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

/*
 * Copyright 2008 Advanced Micro Devices, Inc.
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
 * Author: Stanislaw Skowronek
 */

#include "linux_definitions.h"
#include "radeon_definitions.h"
#include "radeon.h"
#include "atom.h"
#include "atom-names.h"

static inline uint8_t get_u8(void *bios, int ptr)
{
    return ((unsigned char *)bios)[ptr];
}
#define U8(ptr) get_u8(ctx->ctx->bios, (ptr))
#define CU8(ptr) get_u8(ctx->bios, (ptr))
static inline uint16_t get_u16(void *bios, int ptr)
{
    return get_u8(bios ,ptr)|(((uint16_t)get_u8(bios, ptr+1))<<8);
}
#define U16(ptr) get_u16(ctx->ctx->bios, (ptr))
#define CU16(ptr) get_u16(ctx->bios, (ptr))
static inline uint32_t get_u32(void *bios, int ptr)
{
    return get_u16(bios, ptr)|(((uint32_t)get_u16(bios, ptr+2))<<16);
}
#define U32(ptr) get_u32(ctx->ctx->bios, (ptr))
#define CU32(ptr) get_u32(ctx->bios, (ptr))
#define CSTR(ptr) (((char *)(ctx->bios))+(ptr))

static int atom_iio_len[] = { 1, 2, 3, 3, 3, 3, 4, 4, 4, 3 };

static void atom_index_iio(struct atom_context *ctx, int base)
{
	ctx->iio = (uint16_t*)IOMalloc(2 * 256);
	while (CU8(base) == ATOM_IIO_START) {
		ctx->iio[CU8(base + 1)] = base + 2;
		base += 2;
		while (CU8(base) != ATOM_IIO_END)
			base += atom_iio_len[CU8(base)];
		base += 3;
	}
}

bool atom_parse(struct radeon_device *rdev)
{
	int base;
	struct atom_context *ctx = &rdev->atom_context;
	char *str;
	char name[512];
	int i;
    
	if (!ctx)
		return NULL;
    
	ctx->bios = (uint8_t*)rdev->bios;
    
	if (CU16(0) != ATOM_BIOS_MAGIC) {
		radeon_error(rdev, "Invalid BIOS magic.\n");
		return false;
	}
	if (strncmp
	    (CSTR(ATOM_ATI_MAGIC_PTR), ATOM_ATI_MAGIC,
	     strlen(ATOM_ATI_MAGIC))) {
            radeon_error(rdev, "Invalid ATI magic.\n");
            return false;
        }
    
	base = CU16(ATOM_ROM_TABLE_PTR);
	if (strncmp
	    (CSTR(base + ATOM_ROM_MAGIC_PTR), ATOM_ROM_MAGIC,
	     strlen(ATOM_ROM_MAGIC))) {
            radeon_error(rdev, "Invalid ATOM magic.\n");
            return false;
        }
    
	ctx->cmd_table = CU16(base + ATOM_ROM_CMD_PTR);
	ctx->data_table = CU16(base + ATOM_ROM_DATA_PTR);
	atom_index_iio(ctx, CU16(ctx->data_table + ATOM_DATA_IIO_PTR) + 4);
    
	str = CSTR(CU16(base + ATOM_ROM_MSG_PTR));
	while (*str && ((*str == '\n') || (*str == '\r')))
		str++;
	/* name string isn't always 0 terminated */
	for (i = 0; i < 511; i++) {
		name[i] = str[i];
		if (name[i] < '.' || name[i] > 'z') {
			name[i] = 0;
			break;
		}
	}
	radeon_info(rdev, "ATOM BIOS: %s\n", name);
    
	return true;
}

bool atom_parse_data_header(struct atom_context *ctx, int index, uint16_t * size, uint8_t * frev, uint8_t * crev, uint16_t * data_start)
{
	int offset = index * 2 + 4;
	int idx = CU16(ctx->data_table + offset);
	u16 *mdt = (u16 *)(ctx->bios + ctx->data_table + 4);
    
	if (!mdt[index])
		return false;
    
	if (size)
		*size = CU16(idx);
	if (frev)
		*frev = CU8(idx + 2);
	if (crev)
		*crev = CU8(idx + 3);
	*data_start = idx;
	return true;
}
