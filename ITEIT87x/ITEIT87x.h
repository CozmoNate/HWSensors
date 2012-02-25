/*
 *  IT87x.h
 *  HWSensors
 *
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *
 *  Created by mozo on 08/10/10.
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

const UInt8 ITE_ENVIRONMENT_CONTROLLER_LDN				= 0x04;

// ITE
const UInt8 ITE_VENDOR_ID								= 0x90;
const UInt8 ITE_VERSION_REGISTER                        = 0x22;

// ITE Environment Controller
const UInt8 ITE_ADDRESS_REGISTER_OFFSET					= 0x05;
const UInt8 ITE_DATA_REGISTER_OFFSET					= 0x06;

// ITE Environment Controller Registers    
const UInt8 ITE_CONFIGURATION_REGISTER					= 0x00;
const UInt8 ITE_TEMPERATURE_BASE_REG					= 0x29;
const UInt8 ITE_VENDOR_ID_REGISTER						= 0x58;
//const UInt8 ITE_FAN_TACHOMETER_16_BIT_ENABLE_REGISTER	= 0x0c;
const UInt8 ITE_FAN_TACHOMETER_DIVISOR_REGISTER         = 0x0B;
const UInt8 ITE_FAN_TACHOMETER_REG[5]					= { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
const UInt8 ITE_FAN_TACHOMETER_EXT_REG[5]				= { 0x18, 0x19, 0x1a, 0x81, 0x83 };
const UInt8 ITE_VOLTAGE_BASE_REG						= 0x20;

const float ITE_VOLTAGE_GAIN[]							= {1, 1, 1, (6.8f / 10 + 1), 1, 1, 1, 1, 1 };

const UInt8 ITE_SMARTGUARDIAN_MAIN_CONTROL				= 0x13;
const UInt8 ITE_SMARTGUARDIAN_PWM_CONTROL[5]			= { 0x15, 0x16, 0x17, 0x88, 0x89 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_STOP[5]		= { 0x60, 0x68, 0x70, 0x90, 0x98 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_START[5]		= { 0x61, 0x69, 0x71, 0x91, 0x99 };
//const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[5]	= { 0x62, 0x6a, 0x72, 0x92, 0x9a };
const UInt8 ITE_SMARTGUARDIAN_START_PWM[5]				= { 0x63, 0x6b, 0x73, 0x93, 0x9b };
const UInt8 ITE_SMARTGUARDIAN_CONTROL[5]				= { 0x64, 0x6c, 0x74, 0x94, 0x9c };
//const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_FULL_OFF[5]	= { 0x65, 0x6d, 0x75, 0x95, 0x9d };

enum IT87xModel
{
	IT8512F = 0x8512,
    IT8712F = 0x8712,
    IT8716F = 0x8716,
    IT8718F = 0x8718,
    IT8720F = 0x8720,
    IT8721F = 0x8721,
    IT8726F = 0x8726,
	IT8728F = 0x8728,
	IT8752F = 0x8752,
    IT8772E = 0x8772
};


class IT87x : public SuperIOMonitor
{
    OSDeclareDefaultStructors(IT87x)
	
private:
    UInt8                   voltageGain;
    UInt8                   voltageSpecificGain[9];
    
    bool                    has16bitFanCounter;
    
	UInt8					readByte(UInt8 reg);
	void					writeByte(UInt8 reg, UInt8 value);
	
	virtual void			enter();
	virtual void			exit();
    virtual bool			probePort();
    virtual bool			startPlugin();
	
	virtual long			readTemperature(unsigned long index);
	virtual long			readVoltage(unsigned long index);
	virtual long			readTachometer(unsigned long index);
	
    virtual int				getPortsCount();
	virtual const char *	getModelName();
	
public:
	
};