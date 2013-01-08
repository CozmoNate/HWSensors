//
//  radeon_atombios.cpp
//  HWSensors
//
//  Created by Natan Zalkin on 08.01.13.
//
//

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
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_RV770) {
			rdev->int_thermal_type = THERMAL_TYPE_RV770;
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_EVERGREEN) {
			rdev->int_thermal_type = THERMAL_TYPE_EVERGREEN;
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_SUMO) {
			rdev->int_thermal_type = THERMAL_TYPE_SUMO;
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_NISLANDS) {
			rdev->int_thermal_type = THERMAL_TYPE_NI;
		} else if (controller->ucType == ATOM_PP_THERMALCONTROLLER_SISLANDS) {
			rdev->int_thermal_type = THERMAL_TYPE_SI;
		}
    }
}

static int radeon_atombios_parse_power_table_4_5(struct radeon_device *rdev)
{
	int state_index = 0;

	union power_info *power_info;
	int index = GetIndexIntoMasterTable(DATA, PowerPlayInfo);
    u16 data_offset;
	u8 frev, crev;
    
    radeon_debug(rdev, "entering radeon_atombios_parse_power_table_4_5...\n");
    
	if (!atom_parse_data_header(&rdev->atom_context, index, NULL, &frev, &crev, &data_offset))
		return state_index;
	
    power_info = (union power_info *)(rdev->atom_context.bios + data_offset);
   
    radeon_atombios_add_pplib_thermal_controller(rdev, &power_info->pplib.sThermalController);
    
	return state_index;
}

static int radeon_atombios_parse_power_table_6(struct radeon_device *rdev)
{
	int state_index = 0;
	union power_info *power_info;
	int index = GetIndexIntoMasterTable(DATA, PowerPlayInfo);
    u16 data_offset;
	u8 frev, crev;
    
    radeon_debug(rdev, "entering radeon_atombios_parse_power_table_6...\n");
    
	if (!atom_parse_data_header(&rdev->atom_context, index, NULL, &frev, &crev, &data_offset))
		return state_index;
    
	power_info = (union power_info *)(rdev->atom_context.bios + data_offset);
    
	radeon_atombios_add_pplib_thermal_controller(rdev, &power_info->pplib.sThermalController);
    
    return state_index;
}

void radeon_atombios_get_power_modes(struct radeon_device *rdev)
{
	int index = GetIndexIntoMasterTable(DATA, PowerPlayInfo);
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