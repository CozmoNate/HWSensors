/*
 *  NCT677x.h
 *  HWSensors
 *
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *  Copyright 2012 The King and mozodojo. All rights reserved.
 *  
 *
 */

/*
 
 Version: MPL 1.1/GPL 2.0/LGPL 2.1
 
 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 
 http://www.mozilla.org/MPL/
 
 Software distributed under the License is distributed on an "AS IS" basis,
 WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 for the specific language governing rights and limitations under the License.
 
 The Original Code is the Open Hardware Monitor code.
 
 The Initial Developer of the Original Code is 
 Michael Möller <m.moeller@gmx.ch>.
 Portions created by the Initial Developer are Copyright (C) 2010-2011
 the Initial Developer. All Rights Reserved.
 
 Contributor(s):
 
 Alternatively, the contents of this file may be used under the terms of
 either the GNU General Public License Version 2 or later (the "GPL"), or
 the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 in which case the provisions of the GPL or the LGPL are applicable instead
 of those above. If you wish to allow use of your version of this file only
 under the terms of either the GPL or the LGPL, and not to allow others to
 use your version of this file under the terms of the MPL, indicate your
 decision by deleting the provisions above and replace them with the notice
 and other provisions required by the GPL or the LGPL. If you do not delete
 the provisions above, a recipient may use your version of this file under
 the terms of any one of the MPL, the GPL or the LGPL.
 
 */

#include <IOKit/IOService.h>
#include "SuperIOMonitor.h"

const UInt8 NUVOTON_HARDWARE_MONITOR_LDN		= 0x0B;

const UInt8 NUVOTON_ADDRESS_REGISTER_OFFSET     = 0x05;
const UInt8 NUVOTON_DATA_REGISTER_OFFSET        = 0x06;
const UInt8 NUVOTON_BANK_SELECT_REGISTER        = 0x4E;
const UInt16 NUVOTON_VENDOR_ID                  = 0x5CA3;

// Hardware Monitor Registers    
const UInt16 NUVOTON_VENDOR_ID_HIGH_REGISTER    = 0x804F;
const UInt16 NUVOTON_VENDOR_ID_LOW_REGISTER     = 0x004F;
const UInt16 NUVOTON_VOLTAGE_VBAT_REG           = 0x0551;

const UInt16 NUVOTON_TEMPERATURE_REG[]          = { 0x027, 0x73, 0x75, 0x77, 0x150, 0x250, 0x62B, 0x62C, 0x62D };
const UInt16 NUVOTON_TEMPERATURE_HALF_REG[]     = { 0, 0x74, 0x76, 0x78, 0x151, 0x251, 0x62E, 0x62E, 0x62E };    
const UInt16 NUVOTON_TEMPERATURE_SRC_REG[]      = { 0x621, 0x100, 0x200, 0x300, 0x622, 0x623, 0x624, 0x625, 0x626 };
const UInt16 NUVOTON_TEMPERATURE_HALF_BIT[]     = { -1, 7, 7, 7, 7, 7, 0, 1, 2 };
const UInt16 NUVOTON_VOLTAGE_REG[]              = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x550, 0x551 };
const float  NUVOTON_VOLTAGE_SCALE[]            = { 8,    8,    16,   16,   8,    8,    8,    16,    16 };
const UInt16 NUVOTON_FAN_RPM_REG[]              = { 0x656, 0x658, 0x65A, 0x65C, 0x65E };
const UInt16 NUVOTON_FAN_PWM_OUT_REG[]          = { 0x001, 0x003, 0x011 };
const UInt16 NUVOTON_FAN_PWM_COMMAND_REG[]      = { 0x109, 0x209, 0x309 };
const UInt16 NUVOTON_FAN_CONTROL_MODE_REG[]     = { 0x102, 0x202, 0x302 };


const UInt8 NCT6771F_SOURCE_SYSTIN                  = 1;
const UInt8 NCT6771F_SOURCE_CPUTIN                  = 2;
const UInt8 NCT6771F_SOURCE_AUXTIN                  = 3;
const UInt8 NCT6771F_SOURCE_SMBUSMASTER             = 4;
const UInt8 NCT6771F_SOURCE_PECI_0                  = 5; 
const UInt8 NCT6771F_SOURCE_PECI_1                  = 6; 
const UInt8 NCT6771F_SOURCE_PECI_2                  = 7;
const UInt8 NCT6771F_SOURCE_PECI_3                  = 8;
const UInt8 NCT6771F_SOURCE_PECI_4                  = 9;
const UInt8 NCT6771F_SOURCE_PECI_5                  = 10;
const UInt8 NCT6771F_SOURCE_PECI_6                  = 11;
const UInt8 NCT6771F_SOURCE_PECI_7                  = 12;
const UInt8 NCT6771F_SOURCE_PCH_CHIP_CPU_MAX_TEMP   = 13;
const UInt8 NCT6771F_SOURCE_PCH_CHIP_TEMP           = 14;
const UInt8 NCT6771F_SOURCE_PCH_CPU_TEMP            = 15;
const UInt8 NCT6771F_SOURCE_PCH_MCH_TEMP            = 16; 
const UInt8 NCT6771F_SOURCE_PCH_DIM0_TEMP           = 17;
const UInt8 NCT6771F_SOURCE_PCH_DIM1_TEMP           = 18;
const UInt8 NCT6771F_SOURCE_PCH_DIM2_TEMP           = 19;
const UInt8 NCT6771F_SOURCE_PCH_DIM3_TEMP           = 20;
   

const UInt8 NCT6776F_SOURCE_SYSTIN                  = 1;
const UInt8 NCT6776F_SOURCE_CPUTIN                  = 2;
const UInt8 NCT6776F_SOURCE_AUXTIN                  = 3;
const UInt8 NCT6776F_SOURCE_SMBUSMASTER_0           = 4;
const UInt8 NCT6776F_SOURCE_SMBUSMASTER_1           = 5;
const UInt8 NCT6776F_SOURCE_SMBUSMASTER_2           = 6;
const UInt8 NCT6776F_SOURCE_SMBUSMASTER_3           = 7;
const UInt8 NCT6776F_SOURCE_SMBUSMASTER_4           = 8;
const UInt8 NCT6776F_SOURCE_SMBUSMASTER_5           = 9;
const UInt8 NCT6776F_SOURCE_SMBUSMASTER_6           = 10;
const UInt8 NCT6776F_SOURCE_SMBUSMASTER_7           = 11;
const UInt8 NCT6776F_SOURCE_PECI_0                  = 12;
const UInt8 NCT6776F_SOURCE_PECI_1                  = 13;
const UInt8 NCT6776F_SOURCE_PCH_CHIP_CPU_MAX_TEMP   = 14;
const UInt8 NCT6776F_SOURCE_PCH_CHIP_TEMP           = 15;
const UInt8 NCT6776F_SOURCE_PCH_CPU_TEMP            = 16;
const UInt8 NCT6776F_SOURCE_PCH_MCH_TEMP            = 17;
const UInt8 NCT6776F_SOURCE_PCH_DIM0_TEMP           = 18;
const UInt8 NCT6776F_SOURCE_PCH_DIM1_TEMP           = 19;
const UInt8 NCT6776F_SOURCE_PCH_DIM2_TEMP           = 20;
const UInt8 NCT6776F_SOURCE_PCH_DIM3_TEMP           = 21;
const UInt8 NCT6776F_SOURCE_BYTE_TEMP               = 22;

enum NCT677xModel {
	NCT6771F = 0xB470,
    NCT6776F = 0xC330
};

class NCT677x : public SuperIOMonitor
{
    OSDeclareDefaultStructors(NCT677x)
	
private:
    //int                     temperature[3];
	//bool					temperatureIsObsolete[3];
    
    int                     minFanRPM;
    
   	UInt8					readByte(UInt16 reg);
    void					writeByte(UInt16 reg, UInt8 value);
	
	virtual bool			probePort();
    virtual bool			startPlugin();
	virtual void			enter();
	virtual void			exit();
    
    //void					updateTemperatures();
    
	virtual long			readTemperature(unsigned long index);
	virtual long			readVoltage(unsigned long index);
	virtual long			readTachometer(unsigned long index);
	
	virtual const char *	getModelName();
	
public:
	
};

