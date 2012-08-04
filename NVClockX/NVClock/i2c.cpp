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

/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_dac.c,v 1.43 2004/11/30 23:50:26 mvojkovi Exp $ */

//#include <unistd.h>
#include <stdarg.h>
#include "xf86i2c.h"
#include "backend.h"

/*
 * DDC1 support only requires DDC_SDA_MASK,
 * DDC2 support requires DDC_SDA_MASK and DDC_SCL_MASK
 */
#define DDC_SDA_READ_MASK  (1 << 3)
#define DDC_SCL_READ_MASK  (1 << 2)
#define DDC_SDA_WRITE_MASK (1 << 4)
#define DDC_SCL_WRITE_MASK (1 << 5)

static void NVLockUnlock(int lock)
{
	unsigned char cr11;

	nv_card->PCIO[0x3d4] = 0x1f;
	nv_card->PCIO[0x3d5] = lock ? 0x99 : 0x57;

	nv_card->PCIO[0x3d4] = 0x11;
	cr11 = nv_card->PCIO[0x3d5];
	if(lock) cr11 |= 0x80;
	else cr11 &= ~0x80;
	nv_card->PCIO[0x3d5] = cr11;
}


static void NV_I2CGetBits(I2CBusPtr b, int *clock, int *data)
{
	unsigned char val;
	int DDCBase = (int)b->DriverPrivate.val;

	/* Get the result. */
	nv_card->PCIO[0x3d4] = DDCBase;
	val = nv_card->PCIO[0x3d5];

	*clock = (val & DDC_SCL_READ_MASK) != 0;
	*data  = (val & DDC_SDA_READ_MASK) != 0;
}


static void NV_I2CPutBits(I2CBusPtr b, int clock, int data)
{
	unsigned char val;
	int DDCBase = (int)b->DriverPrivate.val;

	nv_card->PCIO[0x3d4] = DDCBase + 1;
	val = nv_card->PCIO[0x3d5] & 0xf0;
	if (clock)
		val |= DDC_SCL_WRITE_MASK;
	else
		val &= ~DDC_SCL_WRITE_MASK;

	if (data)
		val |= DDC_SDA_WRITE_MASK;
	else
		val &= ~DDC_SDA_WRITE_MASK;

	nv_card->PCIO[0x3d4] = DDCBase + 1;
	nv_card->PCIO[0x3d5] = val | 0x1;
}


I2CBusPtr NV_I2CCreateBusPtr(char *name, int bus)
{
	I2CBusPtr I2CPtr;

	I2CPtr = xf86CreateI2CBusRec();
	if(!I2CPtr) return NULL;

	I2CPtr->BusName    = name;
	I2CPtr->scrnIndex  = nv_card->number; /* We need to use unique indices or else it can lead to a segfault in multicard situations */
	I2CPtr->I2CAddress = I2CAddress;
	I2CPtr->I2CPutBits = NV_I2CPutBits;
	I2CPtr->I2CGetBits = NV_I2CGetBits;
	I2CPtr->AcknTimeout = 5;
	I2CPtr->DriverPrivate.val = bus;

	if (!xf86I2CBusInit(I2CPtr))
	{
		return 0;
	}
	return I2CPtr;
}


static void ProbeDevice (I2CBusPtr bus, I2CSlaveAddr addr, const char *format, ...)
{
	I2CDevPtr dev;
	char *s;
	va_list ap;

	if(xf86I2CProbeAddress(bus, addr))
	{
		dev = xf86CreateI2CDevRec();
		s = new char[8];
		va_start (ap, format);
		vsnprintf (s, 7, format, ap);
		va_end (ap);
		dev->DevName = s;
		dev->SlaveAddr = addr;
		dev->pI2CBus = bus;

		if (!xf86I2CDevInit(dev))
		{
			delete[] dev->DevName;
			xf86DestroyI2CDevRec(dev, TRUE);
		}
	}
}


static void I2CProbeAllDevices (I2CBusPtr busses[], int nbus)
{
	I2CSlaveAddr addr;
	int bus;
	for (bus = 0; bus < nbus; bus++)
	{
		for (addr = 0x00; addr < 0x100; addr += 2)
		{
			ProbeDevice (busses[bus], addr, "%1i:%02X", bus, addr);
		}
	}
}


static I2CDevPtr I2cProbeDevices(I2CBusPtr busses[], int num_busses)
{
	int bus;
	I2CDevPtr dev;

	/* Unlock the extended CRTC registers to get i2c working */
	NVLockUnlock(0);

	/* On NV40 cards the i2c busses can be disabled */
	if(nv_card->arch & NV4X)
	{
		nv_card->PCIO[0x3d4] = 0x49;
		nv_card->PCIO[0x3d5] |= 0x4; /* Unlock the i2c busses */
	}
	I2CProbeAllDevices(busses, num_busses);

	if(nv_card->debug)
		IOLog("Probing I2C busses\n");

	for(bus = 0; bus < num_busses; bus++)
	{
		for(dev = busses[bus]->FirstDev; dev; dev = dev->NextDev)
		{
			if(nv_card->debug)
				IOLog("bus: %x device: %x\n", bus, dev->SlaveAddr);

			dev->arch = nv_card->arch;
			switch(dev->SlaveAddr)
			{
				/* LM99 */
				case 0x98:
					if(lm99_detect(dev))
						return dev;
					break;
				case 0x5a:
					if(w83l785r_detect(dev))
						return dev;
					if(w83781d_detect(dev))
						return dev;
				case 0x5c:
					if(f75375_detect(dev))
						return dev;
					if(adt7473_detect(dev))
						return dev;						
				case 0x6e: /* DDC/CI ? */
				/* The addresses below oftenly appear on most cards but what are these? */
				case 0x70:
				case 0xa0:
				case 0xa2:
				case 0xa4:
				case 0xa6:
				case 0xa8:
				case 0xaa:
				case 0xac:
				case 0xae:
					break;
				default:
				/* Unknown device */
					break;
			}
		}
	}

	NVLockUnlock(1);
	return NULL;
}

void i2c_sensor_init(void)
{
	nv_card->sensor = I2cProbeDevices(nv_card->busses, nv_card->num_busses);
	
	/* When a sensor is available, enable the correct function pointers */
	if(nv_card->sensor)
	{
		nv_card->sensor_name = nv_card->sensor->chip_name;
	
		switch(nv_card->sensor->chip_id)
		{
			case LM99:
			case MAX6559:
				nv_card->caps |= BOARD_TEMP_MONITORING | GPU_TEMP_MONITORING;
				nv_card->get_board_temp = lm99_get_board_temp;
				nv_card->get_gpu_temp = lm99_get_gpu_temp;
				break;
			case F75375:
				nv_card->caps |= BOARD_TEMP_MONITORING | GPU_TEMP_MONITORING | I2C_FANSPEED_MONITORING;
				nv_card->get_board_temp = f75375_get_board_temp;
				nv_card->get_gpu_temp = f75375_get_gpu_temp;
				nv_card->get_i2c_fanspeed_rpm = f75375_get_fanspeed_rpm;
				nv_card->get_i2c_fanspeed_pwm = f75375_get_fanspeed_pwm;
				nv_card->set_i2c_fanspeed_pwm = f75375_set_fanspeed_pwm;
				break;
			case W83781D:
				nv_card->caps |= BOARD_TEMP_MONITORING | GPU_TEMP_MONITORING | I2C_FANSPEED_MONITORING;
				nv_card->get_board_temp = w83781d_get_board_temp;
				nv_card->get_gpu_temp = w83781d_get_gpu_temp;
				nv_card->get_i2c_fanspeed_rpm = w83781d_get_fanspeed_rpm;
				nv_card->get_i2c_fanspeed_pwm = w83781d_get_fanspeed_pwm;
				nv_card->set_i2c_fanspeed_pwm = w83781d_set_fanspeed_pwm;
				break;
			case W83L785R:
				nv_card->caps |= BOARD_TEMP_MONITORING | GPU_TEMP_MONITORING | I2C_FANSPEED_MONITORING;
				nv_card->get_board_temp = w83l785r_get_board_temp;
				nv_card->get_gpu_temp = w83l785r_get_gpu_temp;
				nv_card->get_i2c_fanspeed_rpm = w83l785r_get_fanspeed_rpm;
				nv_card->get_i2c_fanspeed_pwm = w83l785r_get_fanspeed_pwm;
				nv_card->set_i2c_fanspeed_pwm = w83l785r_set_fanspeed_pwm;
				break;
			case ADT7473:
				nv_card->caps |= BOARD_TEMP_MONITORING | GPU_TEMP_MONITORING | I2C_FANSPEED_MONITORING | I2C_AUTOMATIC_FANSPEED_CONTROL;
				nv_card->get_board_temp = adt7473_get_board_temp;
				nv_card->get_gpu_temp = adt7473_get_gpu_temp;
				nv_card->get_i2c_fanspeed_mode = adt7473_get_fanspeed_mode;
				nv_card->set_i2c_fanspeed_mode = adt7473_set_fanspeed_mode;
				nv_card->get_i2c_fanspeed_rpm = adt7473_get_fanspeed_rpm;
				nv_card->get_i2c_fanspeed_pwm = adt7473_get_fanspeed_pwm;
				nv_card->set_i2c_fanspeed_pwm = adt7473_set_fanspeed_pwm;
		}
	}
	else
	{
		nv_card->sensor = NULL;
	}
}

