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

const UInt8 NUVOTON_ADDRESS_REGISTER_OFFSET     = 0x05;
const UInt8 NUVOTON_DATA_REGISTER_OFFSET        = 0x06;
const UInt8 NUVOTON_BANK_SELECT_REGISTER        = 0x4E;
const UInt16 NUVOTON_VENDOR_ID                  = 0x5CA3;

// Hardware Monitor Registers    
const UInt16 NUVOTON_VENDOR_ID_HIGH_REGISTER    = 0x804F;
const UInt16 NUVOTON_VENDOR_ID_LOW_REGISTER     = 0x004F;
const UInt16 NUVOTON_VOLTAGE_VBAT_REG           = 0x0551;

const UInt16 NUVOTON_TEMPERATURE_REG[]          = { 0x027,  0x73,  0x75,  0x77, 0x150, 0x250, 0x62B, 0x62C, 0x62D };
const UInt16 NUVOTON_TEMPERATURE_HALF_REG[]     = {     0,  0x74,  0x76,  0x78, 0x151, 0x251, 0x62E, 0x62E, 0x62E };    
const UInt16 NUVOTON_TEMPERATURE_SRC_REG[]      = { 0x621, 0x100, 0x200, 0x300, 0x622, 0x623, 0x624, 0x625, 0x626 };
const UInt16 NUVOTON_TEMPERATURE_HALF_BIT[]     = {    -1,     7,     7,     7,     7,     7,     0,     1,     2 };

const UInt16 NUVOTON_VOLTAGE_REG[]              = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x550, 0x551 };
const float  NUVOTON_VOLTAGE_SCALE[]            = { 8,    8,    16,   16,   8,    8,    8,    16,    16 };

const UInt16 NUVOTON_FAN_RPM_REG[]              = { 0x656, 0x658, 0x65A, 0x65C, 0x65E };
const UInt16 NUVOTON_FAN_PWM_OUT_REG[]          = { 0x001, 0x003, 0x011 };
const UInt16 NUVOTON_FAN_PWM_COMMAND_REG[]      = { 0x109, 0x209, 0x309 };
const UInt16 NUVOTON_FAN_CONTROL_MODE_REG[]     = { 0x102, 0x202, 0x302 };

class NCT677xMonitor : public SuperIOMonitor
{
    OSDeclareDefaultStructors(NCT677xMonitor)
	
private:
    //int                     temperature[3];
	//bool					temperatureIsObsolete[3];
    
    int                     minFanRPM;
    
   	UInt8					readByte(UInt16 reg);
    void					writeByte(UInt16 reg, UInt8 value);
	
    virtual UInt8           temperatureSensorsLimit();
    virtual UInt8           voltageSensorsLimit();
    virtual UInt8           tachometerSensorsLimit();
    //void					updateTemperatures();
    
	virtual float           readTemperature(UInt32 index);
	virtual float			readVoltage(UInt32 index);
	virtual float			readTachometer(UInt32 index);
    
	virtual bool			initialize();
	
public:
	
};

