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
    // I don't know what I am doing...
    IOMemoryMap * mmio5 = NULL;
    IOMemoryDescriptor * theDescriptor;
    IOPhysicalAddress bar = (IOPhysicalAddress)((rdev->pdev->configRead32(kIOPCIConfigBaseAddress5)) & ~0x3f);
    
    theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x80000, kIODirectionOutIn);
    
    if(theDescriptor != NULL)
    {
        mmio5 = theDescriptor->map();
    }
    
    IOMemoryMap * mmio = NULL;
    volatile UInt8* mmio_base;
    
    if (mmio5 && mmio5->getPhysicalAddress() != 0)
    {
        mmio = mmio5;
        mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
    }
    
    // Finally: temp retrieval
    UInt32 temp, actual_temp = 0;
    temp = OSReadLittleInt32(((volatile UInt8 *)mmio_base), THM_TCON_CUR_TMP) >> THM_TCON_CUR_TMP__CUR_TEMP__SHIFT;
    actual_temp = temp & 0x1ff;
    
    return actual_temp;
}

