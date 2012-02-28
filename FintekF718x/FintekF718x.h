/*
 *  F718x.h
 *  HWSensors
 *
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *
 *  Created by mozo on 16/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
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

// Registers
const UInt8 FINTEK_CONFIGURATION_CONTROL_REGISTER = 0x02;
const UInt8 FINTEK_DEVCIE_SELECT_REGISTER = 0x07;
const UInt8 FINTEK_BASE_ADDRESS_REGISTER = 0x60;

const UInt8 FINTEK_VENDOR_ID_REGISTER = 0x23;
const UInt16 FINTEK_VENDOR_ID = 0x1934;
const UInt8 F71858_HARDWARE_MONITOR_LDN = 0x02;
const UInt8 FINTEK_HARDWARE_MONITOR_LDN = 0x04;

// Hardware Monitor
const UInt8 FINTEK_ADDRESS_REGISTER_OFFSET = 0x05;
const UInt8 FINTEK_DATA_REGISTER_OFFSET = 0x06;

// Hardware Monitor Registers
const UInt8 FINTEK_VOLTAGE_BASE_REG = 0x20;
const UInt8 FINTEK_TEMPERATURE_CONFIG_REG = 0x69;
const UInt8 FINTEK_TEMPERATURE_BASE_REG = 0x70;
const UInt8 FINTEK_FAN_TACHOMETER_REG[] = { 0xA0, 0xB0, 0xC0, 0xD0 };

enum F718xMode 
{
	F71858		= 0x0507,
    F71862		= 0x0601, 
    F71869		= 0x0814,
    F71869A     = 0x1007,
    F71882		= 0x0541,
    F71889ED	= 0x0909,
    F71889F		= 0x0723,
	F71808		= 0x0901,
};

class F718x : public SuperIOMonitor
{
    OSDeclareDefaultStructors(F718x)
	
private:
	UInt8					readByte(UInt8 reg);	
	
	virtual bool			probePort();
    virtual bool			startPlugin();
	virtual void			enter();
	virtual void			exit();
	
	virtual UInt16			readTemperature(UInt32 index);
	virtual UInt16			readVoltage(UInt32 index);
	virtual UInt16			readTachometer(UInt32 index);
	
	virtual const char *	getModelName();
	
public:
	
};