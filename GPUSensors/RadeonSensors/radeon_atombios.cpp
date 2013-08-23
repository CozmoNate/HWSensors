//
//  radeon_atombios.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

/*
 * Copyright 2007-8 Advanced Micro Devices, Inc.
 * Copyright 2008 Red Hat Inc.
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
 * Authors: Dave Airlie
 *          Alex Deucher
 */

#include "radeon_definitions.h"
#include "atombios.h"
#include "radeon_atombios.h"
#include "radeon.h"

union power_info {
	struct _ATOM_POWERPLAY_INFO info;
	struct _ATOM_POWERPLAY_INFO_V2 info_2;
	struct _ATOM_POWERPLAY_INFO_V3 info_3;
	struct _ATOM_PPLIB_POWERPLAYTABLE pplib;
	struct _ATOM_PPLIB_POWERPLAYTABLE2 pplib2;
	struct _ATOM_PPLIB_POWERPLAYTABLE3 pplib3;
};

static void radeon_atombios_add_pplib_thermal_controller(struct radeon_device *rdev, ATOM_PPLIB_THERMALCONTROLLER *controller)
{
    radeon_debug(rdev, "entering radeon_atombios_add_pplib_thermal_controller...\n");
    
	if (controller->ucType > 0) {
		if (controller->ucType == ATOM_PP_THERMALCONTROLLER_RV6xx) {
			rdev->int_thermal_type = THERMAL_TYPE_RV6XX;
            radeon_info(rdev, "found RV6XX type thermal sensor\n");
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_RV770) {
			rdev->int_thermal_type = THERMAL_TYPE_RV770;
            radeon_info(rdev, "found RV770 type thermal sensor\n");
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_EVERGREEN) {
			rdev->int_thermal_type = THERMAL_TYPE_EVERGREEN;
            radeon_info(rdev, "found EVERGREEN type thermal sensor\n");
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_SUMO) {
			rdev->int_thermal_type = THERMAL_TYPE_SUMO;
            radeon_info(rdev, "found SUMO type thermal sensor\n");
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_NISLANDS) {
			rdev->int_thermal_type = THERMAL_TYPE_NI;
            radeon_info(rdev, "found NI type thermal sensor\n");
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_SISLANDS) {
			rdev->int_thermal_type = THERMAL_TYPE_SI;
            radeon_info(rdev, "found SI type thermal sensor\n");
		}
        else radeon_warn(rdev, "unknown thermal sensor type 0x%04x\n", controller->ucType);
    }
    //else radeon_warn(rdev, "thermal sensor type not specified using default configuration\n");
}

static int radeon_atombios_parse_power_table_4_5(struct radeon_device *rdev)
{
	int state_index = 0;

	union power_info *power_info;
	int index = (int)GetIndexIntoMasterTable(DATA, PowerPlayInfo);
    u16 data_offset;
	u8 frev, crev;
    
    radeon_debug(rdev, "entering radeon_atombios_parse_power_table_4_5...\n");
    
	if (!atom_parse_data_header(&rdev->atom_context, index, NULL, &frev, &crev, &data_offset))
		return state_index;
	
    power_info = (union power_info *)(rdev->bios + data_offset);
   
    radeon_atombios_add_pplib_thermal_controller(rdev, &power_info->pplib.sThermalController);
    
	return state_index;
}

static int radeon_atombios_parse_power_table_6(struct radeon_device *rdev)
{
	int state_index = 0;
	union power_info *power_info;
	int index = (int)GetIndexIntoMasterTable(DATA, PowerPlayInfo);
    u16 data_offset;
	u8 frev, crev;
    
    radeon_debug(rdev, "entering radeon_atombios_parse_power_table_6...\n");
    
	if (!atom_parse_data_header(&rdev->atom_context, index, NULL, &frev, &crev, &data_offset))
		return state_index;
    
	power_info = (union power_info *)(rdev->bios + data_offset);
    
	radeon_atombios_add_pplib_thermal_controller(rdev, &power_info->pplib.sThermalController);
    
    return state_index;
}

void radeon_atombios_get_power_modes(struct radeon_device *rdev)
{
	int index = (int)GetIndexIntoMasterTable(DATA, PowerPlayInfo);
	u16 data_offset;
	u8 frev, crev;
	int state_index = 0;
    
    radeon_debug(rdev, "entering radeon_atombios_get_power_modes...\n");
    
	if (atom_parse_data_header(&rdev->atom_context, index, NULL, &frev, &crev, &data_offset)) {
		switch (frev) {
            case 1:
            case 2:
            case 3:
                //state_index = radeon_atombios_parse_power_table_1_3(rdev);
                break;
            case 4:
            case 5:
                state_index = radeon_atombios_parse_power_table_4_5(rdev);
                break;
            case 6:
                state_index = radeon_atombios_parse_power_table_6(rdev);
                break;
            default:
                break;
		}
	} 
}