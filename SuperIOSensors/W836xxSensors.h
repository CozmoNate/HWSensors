/*
 *  W836x.h
 *  HWSensors
 *
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *
 *  Created by kozlek on 14/10/10.
 *  Copyright 2010 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
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
 Portions created by the Initial Developer are Copyright (C) 2011
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

const UInt16 WINBOND_VENDOR_ID						= 0x5CA3;
const UInt8 WINBOND_HIGH_BYTE						= 0x80;

// Winbond Hardware Monitor
const UInt8 WINBOND_ADDRESS_REGISTER_OFFSET			= 0x05;
const UInt8 WINBOND_DATA_REGISTER_OFFSET			= 0x06;
const UInt8 WINBOND_BANK_SELECT_REGISTER			= 0x4E;

const UInt8 WINBOND_VENDOR_ID_REGISTER              = 0x4F;
const UInt8 WINBOND_TEMPERATURE_SOURCE_SELECT_REG	= 0x49;

const UInt16 WINBOND_TEMPERATURE[]					= { 0x0150, 0x0250, 0x0027 };

const UInt16 WINBOND_VOLTAGE_VBAT                   = 0x0551;

const UInt16 WINBOND_VOLTAGE[]                      = { 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0550, 0x0551, 0x0552 };
const UInt16 WINBOND_VOLTAGE1[]                     = { 0x0020, 0x0021, 0x0022, 0x0023, 0x0024,                 0x0550, 0x0551 };

const UInt16 WINBOND_TACHOMETER[]					= { 0x0028, 0x0029, 0x002A, 0x003F, 0x0553 };

const UInt8 WINBOND_TACHOMETER_DIV0[]				= { 0x47, 0x47, 0x4B, 0x59, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV0_BIT[]			= { 4,    6,    6,    0,    2 };
const UInt8 WINBOND_TACHOMETER_DIV1[]				= { 0x47, 0x47, 0x4B, 0x59, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV1_BIT[]			= { 5,    7,    7,    1,    3 };
const UInt8 WINBOND_TACHOMETER_DIV2[]				= { 0x5D, 0x5D, 0x5D, 0x4C, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV2_BIT[]			= { 5,    6,    7,    7,    7 };

const UInt16 WINBOND_TACHOMETER_DIVISOR[]			= { 0x0047, 0x004B, 0x004C, 0x0059, 0x005D };
const UInt8 WINBOND_TACHOMETER_DIVISOR0[]			= {     36,     38,     30,      8,     10 };
const UInt8 WINBOND_TACHOMETER_DIVISOR1[]			= {     37,     39,     31,      9,     11 };
const UInt8 WINBOND_TACHOMETER_DIVISOR2[]			= {      5,      6,      7,     23,     15 };

// Fan Control
const UInt8 WINBOND_FAN_CONFIG[]					= { 0x04, 0x04, 0x12, 0x62 };
const UInt8 WINBOND_FAN_CONTROL_BIT[]				= { 0x02, 0x04, 0x01, 0x04 };
const UInt8 WINBOND_FAN_MODE_BIT[]					= { 0x00, 0x01, 0x00, 0x06 };
const UInt8 WINBOND_FAN_OUTPUT[]					= { 0x01, 0x03, 0x11, 0x61 };

class W836xMonitor : public SuperIOMonitor
{
    OSDeclareDefaultStructors(W836xMonitor)
	
private:
	UInt8					fanLimit;
    UInt8					voltageLimit;
    float                   voltageGain;
	UInt16					fanValue[5];
	bool					fanValueObsolete[5];
	
	void					writeByte(UInt16 reg, UInt8 value);
	UInt8					readByte(UInt16 reg);
    
    UInt64                  setBit(UInt64 target, UInt16 bit, UInt32 value);
    
    virtual UInt8           temperatureSensorsLimit();
    virtual UInt8           voltageSensorsLimit();
    virtual UInt8           tachometerSensorsLimit();
    
	virtual float			readTemperature(UInt32 index);
	virtual float			readVoltage(UInt32 index);
    void					updateTachometers();
	virtual float			readTachometer(UInt32 index);
    
    virtual bool            addTemperatureSensors(OSDictionary *configuration);
    virtual bool            addTachometerSensors(OSDictionary *configuration);
    
    virtual bool            initialize();
    
public:
	
};