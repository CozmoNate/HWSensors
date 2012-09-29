//
//  i2c.c
//  HWSensors
//
//  Created by Kozlek on 31.07.12.
//
//

/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2001-2006 Roderick Colenbrander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * I2C support needed for hardware monitoring, this code is partly based on code from nvtv
 * and the opensource xfree86 nv driver.
 */

/* NVTV TV common routines -- Dirk Thierbach <dthierbach@gmx.de>
 *
 * This file is part of nvtv, a tool for tv-output on NVidia cards.
 *
 * nvtv is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nvtv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * $Id: i2c.c,v 1.14 2007/04/24 08:10:47 thunderbird Exp $
 *
 * Contents:
 *
 * Header: Common tv-related routines.
 *
 */

/***************************************************************************\
 |*                                                                           *|
 |*       Copyright 2003 NVIDIA, Corporation.  All rights reserved.           *|
 |*                                                                           *|
 |*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
 |*     international laws.  Users and possessors of this source code are     *|
 |*     hereby granted a nonexclusive,  royalty-free copyright license to     *|
 |*     use this code in individual and commercial software.                  *|
 |*                                                                           *|
 |*     Any use of this source code must include,  in the user documenta-     *|
 |*     tion and  internal comments to the code,  notices to the end user     *|
 |*     as follows:                                                           *|
 |*                                                                           *|
 |*       Copyright 2003 NVIDIA, Corporation.  All rights reserved.           *|
 |*                                                                           *|
 |*     NVIDIA, CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY     *|
 |*     OF  THIS SOURCE  CODE  FOR ANY PURPOSE.  IT IS  PROVIDED  "AS IS"     *|
 |*     WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORPOR-     *|
 |*     ATION DISCLAIMS ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,     *|
 |*     INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGE-     *|
 |*     MENT,  AND FITNESS  FOR A PARTICULAR PURPOSE.   IN NO EVENT SHALL     *|
 |*     NVIDIA, CORPORATION  BE LIABLE FOR ANY SPECIAL,  INDIRECT,  INCI-     *|
 |*     DENTAL, OR CONSEQUENTIAL DAMAGES,  OR ANY DAMAGES  WHATSOEVER RE-     *|
 |*     SULTING FROM LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION     *|
 |*     OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  ARISING OUT OF     *|
 |*     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE CODE.     *|
 |*                                                                           *|
 |*     U.S. Government  End  Users.   This source code  is a "commercial     *|
 |*     item,"  as that  term is  defined at  48 C.F.R. 2.101 (OCT 1995),     *|
 |*     consisting  of "commercial  computer  software"  and  "commercial     *|
 |*     computer  software  documentation,"  as such  terms  are  used in     *|
 |*     48 C.F.R. 12.212 (SEPT 1995)  and is provided to the U.S. Govern-     *|
 |*     ment only as  a commercial end item.   Consistent with  48 C.F.R.     *|
 |*     12.212 and  48 C.F.R. 227.7202-1 through  227.7202-4 (JUNE 1995),     *|
 |*     all U.S. Government End Users  acquire the source code  with only     *|
 |*     those rights set forth herein.                                        *|
 |*                                                                           *|
 \***************************************************************************/

#include "nvclock_i2c.h"
#include "nouveau.h"

void nvclock_i2c_lock_unlock(nouveau_device *device, int lock)
{
	unsigned char cr11;
    volatile unsigned char *PCIO = (volatile unsigned char*)(device->mmio->getVirtualAddress() + 0x00601000);
    
    PCIO[0x3d4] = 0x1f;
	PCIO[0x3d5] = lock ? 0x99 : 0x57;
    
	PCIO[0x3d4] = 0x11;
	cr11 = PCIO[0x3d5];
	if(lock) cr11 |= 0x80;
	else cr11 &= ~0x80;
	PCIO[0x3d5] = cr11;
}


void nvclock_i2c_get_bits(I2CBusPtr b, int *clock, int *data)
{
	unsigned char val;
	int DDCBase = (int)b->DriverPrivate.val;
    volatile unsigned char *PCIO = (volatile unsigned char*)(b->card->mmio->getVirtualAddress() + 0x00601000);
    
	/* Get the result. */
	PCIO[0x3d4] = DDCBase;
	val = PCIO[0x3d5];
    
	*clock = (val & DDC_SCL_READ_MASK) != 0;
	*data  = (val & DDC_SDA_READ_MASK) != 0;
}

void nvclock_i2c_put_bits(I2CBusPtr b, int clock, int data)
{
	unsigned char val;
	int DDCBase = (int)b->DriverPrivate.val;
    volatile unsigned char *PCIO = (volatile unsigned char*)(b->card->mmio->getVirtualAddress() + 0x00601000);
    
	PCIO[0x3d4] = DDCBase + 1;
	val = PCIO[0x3d5] & 0xf0;
	if (clock)
		val |= DDC_SCL_WRITE_MASK;
	else
		val &= ~DDC_SCL_WRITE_MASK;
    
	if (data)
		val |= DDC_SDA_WRITE_MASK;
	else
		val &= ~DDC_SDA_WRITE_MASK;
    
	PCIO[0x3d4] = DDCBase + 1;
	PCIO[0x3d5] = val | 0x1;
}

void nvclock_nv50_i2c_get_bits(I2CBusPtr bus, int *clock, int *data)
{
	const long offset = bus->DriverPrivate.val;
    volatile unsigned int *PMC = (volatile unsigned int*)bus->card->mmio->getVirtualAddress();
    
    unsigned char val = PMC[offset/4];
    
	*clock = !!(val & 1);
	*data = !!(val & 2);
}

void nvclock_nv50_i2c_put_bits(I2CBusPtr bus, int clock, int data)
{
	const long offset = bus->DriverPrivate.val;
    volatile unsigned int *PMC = (volatile unsigned int*)bus->card->mmio->getVirtualAddress();
    
	PMC[offset/4] = 4 | clock | data << 1;
}

I2CBusPtr nvclock_i2c_create_bus_ptr(nouveau_device *device, char *name, int bus)
{
	I2CBusPtr I2CPtr;
    
	I2CPtr = xf86CreateI2CBusRec();
	if(!I2CPtr) return NULL;
    
	I2CPtr->BusName    = name;
	I2CPtr->scrnIndex  = device->card_index; /* We need to use unique indices or else it can lead to a segfault in multicard situations */
	I2CPtr->I2CAddress = I2CAddress;
    
    if (device->card_type < NV_50) {
        I2CPtr->I2CPutBits = nvclock_i2c_put_bits;
        I2CPtr->I2CGetBits = nvclock_i2c_get_bits;
        I2CPtr->AcknTimeout = 5;
    }
    else {
        I2CPtr->I2CPutBits = nvclock_nv50_i2c_put_bits;
        I2CPtr->I2CGetBits = nvclock_nv50_i2c_get_bits;
        I2CPtr->AcknTimeout = 40;
    }
	I2CPtr->DriverPrivate.val = bus;
    I2CPtr->card = device;
    
	if (!xf86I2CBusInit(I2CPtr))
		return 0;

	return I2CPtr;
}

I2CDevPtr nvclock_i2c_probe_device(I2CBusPtr bus, I2CSlaveAddr addr, const char *format, ...)
{
	I2CDevPtr dev;
	char *s;
	va_list ap;
    
// Commented out cose xf86I2CProbeAddress was already called in nouveau_i2c_identify function 
//	if(xf86I2CProbeAddress(bus, addr))
//	{
		dev = xf86CreateI2CDevRec();
		s = (char*)IOMalloc(8);
		va_start (ap, format);
		vsnprintf (s, 7, format, ap);
		va_end (ap);
		dev->DevName = s;
		dev->SlaveAddr = addr;
		dev->pI2CBus = bus;
        
		if (!xf86I2CDevInit(dev))
		{
			IOFree(dev->DevName, 8);
			xf86DestroyI2CDevRec(dev, TRUE);
            
            return NULL;
		}
        
        return dev;
//	}
//    
//    return NULL;
}

void nvclock_i2c_probe_all_devices(I2CBusPtr busses[], int nbus)
{
	I2CSlaveAddr addr;
	int bus;
	for (bus = 0; bus < nbus; bus++)
	{
		for (addr = 0x00; addr < 0x100; addr += 2)
		{
			nvclock_i2c_probe_device(busses[bus], addr, "%1i:%02X", bus, addr);
		}
	}
}

I2CDevPtr nvclock_i2c_probe_devices(nouveau_device *device, I2CBusPtr busses[], int num_busses)
{
	int bus;
	I2CDevPtr dev;
    
    nv_debug(device, "probing I2C busses...\n");
    
	/* Unlock the extended CRTC registers to get i2c working */
	nvclock_i2c_lock_unlock(device, 0);
    
	/* On NV40 cards the i2c busses can be disabled */
	if(device->card_type == NV_40)
	{
        volatile unsigned char *PCIO = (volatile unsigned char*)(device->mmio->getVirtualAddress() + 0x00601000);
        PCIO[0x3d4] = 0x49;
        PCIO[0x3d5] |= 0x4; /* Unlock the i2c busses */
	}
    
	nvclock_i2c_probe_all_devices(busses, num_busses);
    
	for(bus = 0; bus < num_busses; bus++)
	{
		for(dev = busses[bus]->FirstDev; dev; dev = dev->NextDev)
		{
			nv_debug(dev->pI2CBus->card, "got response on bus:%s port:0x%x\n", busses[bus]->BusName, dev->SlaveAddr / 2);
            
			dev->arch = dev->pI2CBus->card->chipset;
                           
			switch(dev->SlaveAddr / 2)
			{
                case 0x2d:
					if(w83l785r_detect(dev))
						return dev;
					if(w83781d_detect(dev))
						return dev;
                    break;
				case 0x2e:
					if(f75375_detect(dev))
						return dev;
					if(adt7473_detect(dev))
						return dev;
                    break;
                case 0x4c:
					if(lm99_detect(dev))
						return dev;
                    break;
				default:
                    /* Unknown device */
					break;
			}
		}
	}
    
	nvclock_i2c_lock_unlock(device, 1);
    
	return NULL;
}

bool nvclock_i2c_sensor_init(nouveau_device *device)
{
    int num_busses = 0;
    I2CBusPtr busses[4];
    
    struct nouveau_i2c_port *port = NULL;
    
    for (int i = 0; i <= 4; i++) {
        if ((port = nouveau_i2c_find(&device->i2c, NV_I2C_DEFAULT(i)))) {
            char name[16];
            
            snprintf(name, 16, "NV_I2C_DEFAULT%X", i);
            
            busses[num_busses++] = nvclock_i2c_create_bus_ptr(device, STRDUP(name, sizeof(name)), port->drive);
        }
    }
    
    if (num_busses > 0) {
        device->nvclock_i2c_sensor = nvclock_i2c_probe_devices(device, busses, num_busses);
        
        /* When a sensor is available, enable the correct function pointers */
        if(device->nvclock_i2c_sensor) {
            nv_info(device, "found %s monitoring chip\n", device->nvclock_i2c_sensor->chip_name);
            
            switch(device->nvclock_i2c_sensor->chip_id)
            {
                case LM99:
                case MAX6559:
                    device->board_temp_get = lm99_get_board_temp;
                    device->core_temp_get = lm99_get_gpu_temp;
                    break;
                case F75375:
                    device->board_temp_get = f75375_get_board_temp;
                    device->core_temp_get = f75375_get_gpu_temp;
                    device->fan_rpm_get = f75375_get_fanspeed_rpm;
                    device->fan_pwm_get = f75375_get_fanspeed_pwm;
                    break;
                case W83781D:
                    device->board_temp_get = w83781d_get_board_temp;
                    device->core_temp_get = w83781d_get_gpu_temp;
                    //nv_card->get_i2c_fanspeed_rpm = w83781d_get_fanspeed_rpm;
                    //nv_card->get_i2c_fanspeed_pwm = w83781d_get_fanspeed_pwm;
                    break;
                case W83L785R:
                    device->board_temp_get = w83l785r_get_board_temp;
                    device->core_temp_get = w83l785r_get_gpu_temp;
                    device->fan_rpm_get = w83l785r_get_fanspeed_rpm;
                    device->fan_pwm_get = w83l785r_get_fanspeed_pwm;
                    break;
                case ADT7473:
                    device->board_temp_get = adt7473_get_board_temp;
                    device->core_temp_get = adt7473_get_gpu_temp;
                    device->fan_rpm_get = adt7473_get_fanspeed_rpm;
                    device->fan_pwm_get = adt7473_get_fanspeed_pwm;
                    break;
                default:
                    break;
            }
        }
            
        return true;
    }
    
    return false;
}

