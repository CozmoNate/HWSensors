//
//  vega.c
//  HWSensors
//
//  Created by Tobias Punke on 02.02.2018.
//
//

#include "vega.h"
#include "radeon_definitions.h"

#include <IOKit/IOLib.h>
#include <IOKit/pci/IOPCIDevice.h>

//VEGA10
#define THM_TCON_CUR_TMP                      0x59800
#define THM_TCON_CUR_TMP__CUR_TEMP__SHIFT     24

int vega_get_temp(struct radeon_device *rdev)
{
    UInt32 temp, actual_temp = 0;
    temp = OSReadLittleInt32(rdev->mmio_base, THM_TCON_CUR_TMP) >> THM_TCON_CUR_TMP__CUR_TEMP__SHIFT;
    actual_temp = temp & 0x1ff;
    
    return actual_temp;
}

