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
#include "AtomBIOS.h"

#define	CG_MULT_THERMAL_STATUS				0x740
#define		ASIC_TM(x)			        ((x) << 16)
#define		ASIC_TM_MASK			        0x3FF0000
#define		ASIC_TM_SHIFT			        16
#define	CG_THERMAL_STATUS				0x7F4
#define		ASIC_T(x)			        ((x) << 0)
#define		ASIC_T_MASK			        0x1FF
#define		ASIC_T_SHIFT			        0

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

/*
* Chip families. Must fit in the low 16 bits of a long word
*/
enum radeon_family {
    CHIP_FAMILY_UNKNOW,
    CHIP_FAMILY_LEGACY,
    CHIP_FAMILY_RADEON,
    CHIP_FAMILY_RV100,
    CHIP_FAMILY_RS100,    /* U1 (IGP320M) or A3 (IGP320)*/
    CHIP_FAMILY_RV200,
    CHIP_FAMILY_RS200,    /* U2 (IGP330M/340M/350M) or A4 (IGP330/340/345/350), RS250 (IGP 7000) */
    CHIP_FAMILY_R200,
    CHIP_FAMILY_RV250,
    CHIP_FAMILY_RS300,    /* RS300/RS350 */
    CHIP_FAMILY_RV280,
    CHIP_FAMILY_R300,
    CHIP_FAMILY_R350,
    CHIP_FAMILY_RV350,
    CHIP_FAMILY_RV380,    /* RV370/RV380/M22/M24 */
    CHIP_FAMILY_R420,     /* R420/R423/M18 */
    CHIP_FAMILY_RV410,    /* RV410, M26 */
    CHIP_FAMILY_RS400,    /* xpress 200, 200m (RS400/410/480) */
    CHIP_FAMILY_RV515,    /* rv515 */
    CHIP_FAMILY_R520,    /* r520 */
    CHIP_FAMILY_RV530,    /* rv530 */
    CHIP_FAMILY_R580,    /* r580 */
    CHIP_FAMILY_RV560,   /* rv560 */
    CHIP_FAMILY_RV570,   /* rv570 */
    CHIP_FAMILY_RS690,
    CHIP_FAMILY_R600,    /* r60 */
    CHIP_FAMILY_R630,
    CHIP_FAMILY_RV610,
    CHIP_FAMILY_RV630,
    CHIP_FAMILY_RV670,
    CHIP_FAMILY_RS740,
	CHIP_FAMILY_RV620,
	CHIP_FAMILY_RV635,	//HD3600
	CHIP_FAMILY_R700,	//HD4850
	CHIP_FAMILY_RV700,  //ATI Radeon HD 4870 X2
	CHIP_FAMILY_R710,   //HD4350
	CHIP_FAMILY_R730,	//HD4650
	CHIP_FAMILY_RV740,	//HD4770
	CHIP_FAMILY_RV770,	//HD4830
	CHIP_FAMILY_RS780,  //AMD 760G
	CHIP_FAMILY_RV790,
	CHIP_FAMILY_RS880,
	CHIP_FAMILY_Evergreen,
	CHIP_FAMILY_CEDAR,	//
	CHIP_FAMILY_REDWOOD,
	CHIP_FAMILY_JUNIPER,
	CHIP_FAMILY_CYPRESS,
	CHIP_FAMILY_HEMLOCK,
    CHIP_FAMILY_LAST,
};

#define IS_RV100_VARIAN (((rinfo)->ChipFamily == CHIP_FAMILY_RV100)	|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_RV200)		|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_RS100)		|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_RS200)		|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_RV250)		|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_RV280)		|| \
								((rinfo)->ChipFamily == CHIP_RS300))


#define IS_R300_VARIANT	(((rinfo)->ChipFamily == CHIP_FAMILY_R300)		|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_RV350)		|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_R350)		|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_RV380)		|| \
								((rinfo)->ChipFamily == CHIP_FAMILY_R420))

#define IS_AVIVO_VARIANT ((rinfo->ChipFamily >= CHIP_FAMILY_RV515))

#define IS_DCE3_VARIANT ((rinfo->ChipFamily >= CHIP_FAMILY_RV620))

#define IS_R500_3D ((rinfo->ChipFamily == CHIP_FAMILY_RV515)  ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_R520)   ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RV530)  ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_R580)   ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RV560)  ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RV570))

#define IS_R300_3D ((rinfo->ChipFamily == CHIP_FAMILY_R300)  ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RV350) ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_R350)  ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RV380) ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_R420)  ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RV410) ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RS690) ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RS600) ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RS740) ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RS400) ||  \
					(rinfo->ChipFamily == CHIP_FAMILY_RS480))



#endif /* __RADEON_CHIPSETS_H__ */
