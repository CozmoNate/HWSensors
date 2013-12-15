/*
 *  NCT677x.h
 *  HWSensors
 *
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *  Copyright 2012 The King. All rights reserved.
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
#include "LPCSensors.h"

const UInt8 NUVOTON_ADDRESS_REGISTER_OFFSET     = 0x05;
const UInt8 NUVOTON_DATA_REGISTER_OFFSET        = 0x06;
const UInt8 NUVOTON_BANK_SELECT_REGISTER        = 0x4E;
const UInt8 NUVOTON_REG_ENABLE                  = 0x30;
const UInt8 NUVOTON_HWMON_IO_SPACE_LOCK         = 0x28;
const UInt16 NUVOTON_VENDOR_ID                  = 0x5CA3;

// Hardware Monitor Registers    
const UInt16 NUVOTON_VENDOR_ID_HIGH_REGISTER    = 0x804F;
const UInt16 NUVOTON_VENDOR_ID_LOW_REGISTER     = 0x004F;
const UInt16 NUVOTON_VOLTAGE_VBAT_REG           = 0x0551;

const UInt16 NUVOTON_TEMPERATURE_REG[]          = { 0x027, 0x073, 0x075, 0x077, 0x150, 0x250, 0x62B, 0x62C, 0x62D };
const UInt16 NUVOTON_TEMPERATURE_REG_NEW[]      = { 0x027, 0x073, 0x075, 0x077, 0x079, 0x07B, 0x150 };
const UInt16 NUVOTON_TEMPERATURE_SEL_REG[]      = {	0x100, 0x200, 0x300, 0x800, 0x900, 0xa00 };

const UInt16 NUVOTON_VOLTAGE_REG[]              = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x550, 0x551 };
const float  NUVOTON_VOLTAGE_SCALE[]            = { 8,    8,    16,   16,   8,    8,    8,    16,    16 };

const UInt16 NUVOTON_VOLTAGE_REG_NEW[]          = { 0x480, 0x481, 0x482, 0x483, 0x484, 0x485, 0x486, 0x487, 0x488, 0x489, 0x48A, 0x48B, 0x48C, 0x48D, 0x48E };

const UInt16 NUVOTON_FAN_RPM_REG[]              = { 0x656, 0x658, 0x65A, 0x65C, 0x65E, 0x660 };
const UInt16 NUVOTON_FAN_STOP_REG[]             = {	0x105, 0x205, 0x305, 0x805, 0x905, 0xa05 };

const UInt16 NUVOTON_FAN_PWM_OUT_REG[]          = { 0x01,  0x03,  0x11,  0x13,  0x15,  0xa09 };
const UInt16 NUVOTON_FAN_PWM_MODE_REG[]         = { 0x04,  0,     0,     0,     0,     0 };
const UInt16 NUVOTON_PWM_MODE_MASK[]            = { 0x01,  0,     0,     0,     0,     0 };
const UInt16 NUVOTON_FAN_PWM_MODE_OLD_REG[]     = { 0x04,  0x04,  0x12 };
const UInt16 NUVOTON_PWM_MODE_MASK_OLD[]        = { 0x01,  0x02,  0x01 };

class NCT677xSensors : public LPCSensors
{
    OSDeclareDefaultStructors(NCT677xSensors)
	
private:   
    UInt8					fanLimit;
    UInt8					tempLimit;
    UInt8                   voltLimit;
    UInt16                  fanRpmBaseRegister;
    int                     minFanRPM;
    UInt16                  voltageVBatRegister;
    bool                    fanControlEnabled[6];
    
   	UInt8					readByte(UInt16 reg);
    void					writeByte(UInt16 reg, UInt8 value);
	
    virtual UInt8           temperatureSensorsLimit();
    virtual UInt8           voltageSensorsLimit();
    virtual UInt8           tachometerSensorsLimit();
    //void					updateTemperatures();
    
	virtual float           readTemperature(UInt32 index);
	virtual float			readVoltage(UInt32 index);
	virtual float			readTachometer(UInt32 index);
    
    virtual bool			supportsTachometerControl();
    virtual UInt8			readTachometerControl(UInt32 index);
    virtual void			writeTachometerControl(UInt32 index, UInt8 percent);
    
	virtual bool			initialize();
	
public:
	
};

