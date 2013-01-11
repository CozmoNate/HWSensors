/*
 * Copyright (C) 2006 Claudio Ciccani <klan@users.sf.net>
 *
 * Graphics driver for ATI Radeon cards written by
 *             Claudio Ciccani <klan@users.sf.net>.  
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#ifndef __RADEON_CHIPSETS_H__
#define __RADEON_CHIPSETS_H__

#include <IOKit/IOLib.h>
#include <IOKit/pci/IOPCIDevice.h>

#include "linux_definitions.h"
#include "radeon_family.h"
#include "atom.h"

enum radeon_int_thermal_type {
	THERMAL_TYPE_NONE,
	THERMAL_TYPE_RV6XX,
	THERMAL_TYPE_RV770,
	THERMAL_TYPE_EVERGREEN,
	THERMAL_TYPE_SUMO,
	THERMAL_TYPE_NI,
	THERMAL_TYPE_SI,
};

typedef struct {
    UInt16 device_id;
//    RADEONChipFamily chip_family;
	UInt16 ChipFamily;
    int is_mobility;
    int igp;
    int nocrtc2;
    int nointtvout;
    int singledac;
} RADEONCardInfo;

struct radeon_device {
    IOPCIDevice         *pdev;
    IOMemoryMap         *mmio;
    UInt32              chip_id;
	UInt16              family;
	RADEONCardInfo      info;
	SInt8               card_index;
    
    radeon_int_thermal_type int_thermal_type;
    
    /* BIOS */
	UInt8				*bios;
    char                bios_name[64];
	bool				is_atom_bios;
	UInt16              bios_header_start;
    UInt32              bios_size;
    
    struct atom_context atom_context;
    
    int (*get_core_temp)(struct radeon_device *);
};

#define RREG08(offset)		(*(volatile UInt8 *)(rdev->mmio->getVirtualAddress() + offset))
#define RREG16(offset)		OSReadLittleInt16(((volatile UInt8 *)rdev->mmio->getVirtualAddress()), offset)
#define RREG32(offset)		OSReadLittleInt32(((volatile UInt8 *)rdev->mmio->getVirtualAddress()), offset)
#define WREG32(offset,val)	OSWriteLittleInt32(((volatile UInt8 *)rdev->mmio->getVirtualAddress()), offset, val)

/*
 * BIOS helpers.
 */
#define RBIOS8(i) (rdev->bios[i])
#define RBIOS16(i) (RBIOS8(i) | (RBIOS8((i)+1) << 8))
#define RBIOS32(i) ((RBIOS16(i)) | (RBIOS16((i)+2) << 16))

#endif /* __RADEON_CHIPSETS_H__ */
