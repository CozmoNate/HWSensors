/*
 *  F718x.cpp
 *  HWSensors
 *
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *
 *  Created by kozlek on 16/10/10.
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

#include "FintekF718x.h"
#include "FakeSMCDefinitions.h"
#include "SuperIO.h"

/*#define Debug FALSE

#define LogPrefix "F718xMonitor: "
#define HWSensorsDebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define HWSensorsWarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define HWSensorsInfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)*/

#define super SuperIOMonitor
OSDefineMetaClassAndStructors(F718xMonitor, SuperIOMonitor)

UInt8 F718xMonitor::readByte(UInt8 reg) 
{
	outb(address + FINTEK_ADDRESS_REGISTER_OFFSET, reg);
	return inb(address + FINTEK_DATA_REGISTER_OFFSET);
} 

UInt8 F718xMonitor::temperatureSensorsLimit()
{
    return 3;
}

UInt8 F718xMonitor::voltageSensorsLimit()
{
    switch (model) 
	{
        case F71858:
			return 0;
        default:
			return 9;
	};
}

UInt8 F718xMonitor::tachometerSensorsLimit()
{
    return (model == F71882 || model == F71858 ? 4 : 3);
}

float F718xMonitor::readTemperature(UInt32 index)
{
	if (model == F71858) 
	{
        int tableMode = 0x3 & readByte(FINTEK_TEMPERATURE_CONFIG_REG);
        int high = readByte(FINTEK_TEMPERATURE_BASE_REG + 2 * index);
        int low = readByte(FINTEK_TEMPERATURE_BASE_REG + 2 * index + 1);      
        
        if (high != 0xbb && high != 0xcc) 
        {
            int bits = 0;
            
            switch (tableMode) 
            {
                case 0: bits = 0; break;
                case 1: bits = 0; break;
                case 2: bits = (high & 0x80) << 8; break;
                case 3: bits = (low & 0x01) << 15; break;
            }
            bits |= high << 7;
            bits |= (low & 0xe0) >> 1;
            
            short val = (short)(bits & 0xfff0);
            
            return (float)val / 128.0f;
        } 
        
        return 0;
	}
    
    SInt8 value = readByte(FINTEK_TEMPERATURE_BASE_REG + 2 * (index + 1));
	
	return value < 0 ? -value : value;
}

float F718xMonitor::readVoltage(UInt32 index)
{
	//return (index == 1 ? 0.5f : 1.0f) * (readByte(FINTEK_VOLTAGE_BASE_REG + index) << 4) * 0.001f;
    return (float)(readByte(FINTEK_VOLTAGE_BASE_REG + index)) * 0.008f;
}

float F718xMonitor::readTachometer(UInt32 index)
{
	SInt32 value = readByte(FINTEK_FAN_TACHOMETER_REG[index]) << 8;
	value |= readByte(FINTEK_FAN_TACHOMETER_REG[index] + 1);
	
	if (value > 0)
		value = (value < 0x0fff) ? 1.5e6f / (float)value : 0;
	
	return value;
}

bool F718xMonitor::initialize()
{    
    winbond_family_enter(port);
    
    UInt16 vendor = superio_listen_port_word(port, FINTEK_VENDOR_ID_REGISTER);
    
    winbond_family_exit(port);
    
    if (vendor != FINTEK_VENDOR_ID)
    {
        HWSensorsWarningLog("wrong vendor id=0x%x", vendor);
        return false;
    }
	
	return true;
}
