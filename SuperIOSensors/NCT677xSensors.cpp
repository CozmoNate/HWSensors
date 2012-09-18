/*
 *  NCT677x.c
 *  HWSensors
 *
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *  Copyright (C) 2011-2012 THe KiNG. All Rights Reserved (R)
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

#include "NuvotonNCT677x.h"
#include "FakeSMCDefinitions.h"
#include "SuperIO.h"

/*#define Debug FALSE

#define LogPrefix "NCT677xMonitor: "
#define HWSensorsDebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define HWSensorsWarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define HWSensorsInfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)*/

#define super SuperIOMonitor
OSDefineMetaClassAndStructors(NCT677xMonitor, SuperIOMonitor)

UInt8 NCT677xMonitor::readByte(UInt16 reg) 
{
    UInt8 bank = reg >> 8;
    UInt8 regi = reg & 0xFF;
    
    outb((UInt16)(address + NUVOTON_ADDRESS_REGISTER_OFFSET), NUVOTON_BANK_SELECT_REGISTER);
    outb((UInt16)(address + NUVOTON_DATA_REGISTER_OFFSET), bank);
    outb((UInt16)(address + NUVOTON_ADDRESS_REGISTER_OFFSET), regi);
    
    return inb((UInt16)(address + NUVOTON_DATA_REGISTER_OFFSET));
}

void NCT677xMonitor::writeByte(UInt16 reg, UInt8 value)
{
	UInt8 bank = reg >> 8;
    UInt8 regi = reg & 0xFF;
    
    outb((UInt16)(address + NUVOTON_ADDRESS_REGISTER_OFFSET), NUVOTON_BANK_SELECT_REGISTER);
    outb((UInt16)(address + NUVOTON_DATA_REGISTER_OFFSET), bank);
    outb((UInt16)(address + NUVOTON_ADDRESS_REGISTER_OFFSET), regi);
    outb((UInt16)(address + NUVOTON_DATA_REGISTER_OFFSET), value);
}

UInt8 NCT677xMonitor::temperatureSensorsLimit()
{
    return 9;
}

UInt8 NCT677xMonitor::voltageSensorsLimit()
{
    return 9;
}

UInt8 NCT677xMonitor::tachometerSensorsLimit()
{
    return 5;
}

float NCT677xMonitor::readTemperature(UInt32 index)
{
    if (index < temperatureSensorsLimit()) {
        
        int value = readByte(NUVOTON_TEMPERATURE_REG[index]) << 1;
        
        //if (NUVOTON_TEMPERATURE_HALF_BIT[index] > 0)
            //value |= ((readByte(NUVOTON_TEMPERATURE_HALF_REG[index]) >> NUVOTON_TEMPERATURE_HALF_BIT[index]) & 0x1);
        
        float t = 0.5f * (float)value;
        
        return t <= 125 && t >= -55 ? t : 0;
    }
    
    
	return 0;
}

float NCT677xMonitor::readVoltage(UInt32 index)
{
    if (index < voltageSensorsLimit()) {
        
        float value = readByte(NUVOTON_VOLTAGE_REG[index]) * (NUVOTON_VOLTAGE_SCALE[index]) * 0.001f;
        
        bool valid = value > 0;
        
        // check if battery voltage monitor is enabled
        if (valid && NUVOTON_VOLTAGE_REG[index] == NUVOTON_VOLTAGE_VBAT_REG) {
            valid = (readByte(0x5D) & 0x01) > 0;
        }
        
        return valid ? value : 0;
    }
    
    return 0;
}

float NCT677xMonitor::readTachometer(UInt32 index)
{
    if (index < tachometerSensorsLimit()) {
        UInt8 high = readByte(NUVOTON_FAN_RPM_REG[index]);
        UInt8 low = readByte(NUVOTON_FAN_RPM_REG[index] + 1);
        
        int value = (high << 8) | low;
        
        return value > minFanRPM ? value : 0;
    }
    
    return 0;
}

bool NCT677xMonitor::initialize()
{
    UInt16 vendor = (UInt16)(readByte(NUVOTON_VENDOR_ID_HIGH_REGISTER) << 8) | readByte(NUVOTON_VENDOR_ID_LOW_REGISTER);
    
    if (vendor != NUVOTON_VENDOR_ID)
    {
        HWSensorsWarningLog("wrong vendor ID=0x%x", vendor);
        return false;
    }
    
    switch (model) {
        case NCT6771F:
            minFanRPM = (int)(1.35e6 / 0xFFFF);
            break;
            
        case NCT6776F:
        case NCT6779D:
            minFanRPM = (int)(1.35e6 / 0x1FFF);
            break;
    }
    
	return true;
}
