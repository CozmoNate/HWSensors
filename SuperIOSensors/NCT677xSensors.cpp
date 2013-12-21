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

#include "NCT677xSensors.h"
#include "FakeSMCDefinitions.h"
#include "SuperIODevice.h"

#define super LPCSensors
OSDefineMetaClassAndStructors(NCT677xSensors, LPCSensors)

UInt8 NCT677xSensors::readByte(UInt16 reg) 
{
    UInt8 bank = reg >> 8;
    UInt8 regi = reg & 0xFF;
    
    outb((UInt16)(address + NUVOTON_ADDRESS_REGISTER_OFFSET), NUVOTON_BANK_SELECT_REGISTER);
    outb((UInt16)(address + NUVOTON_DATA_REGISTER_OFFSET), bank);
    outb((UInt16)(address + NUVOTON_ADDRESS_REGISTER_OFFSET), regi);
    
    return inb((UInt16)(address + NUVOTON_DATA_REGISTER_OFFSET));
}

void NCT677xSensors::writeByte(UInt16 reg, UInt8 value)
{
	UInt8 bank = reg >> 8;
    UInt8 regi = reg & 0xFF;
    
    outb((UInt16)(address + NUVOTON_ADDRESS_REGISTER_OFFSET), NUVOTON_BANK_SELECT_REGISTER);
    outb((UInt16)(address + NUVOTON_DATA_REGISTER_OFFSET), bank);
    outb((UInt16)(address + NUVOTON_ADDRESS_REGISTER_OFFSET), regi);
    outb((UInt16)(address + NUVOTON_DATA_REGISTER_OFFSET), value);
}

UInt8 NCT677xSensors::temperatureSensorsLimit()
{
    return tempLimit;
}

UInt8 NCT677xSensors::voltageSensorsLimit()
{
    return voltLimit;
}

UInt8 NCT677xSensors::tachometerSensorsLimit()
{
    return fanLimit;
}

float NCT677xSensors::readTemperature(UInt32 index)
{
    if (index < tempLimit) {
        
        int value; 
        
        switch (model) {
            case NCT6771F:
            case NCT6776F:    
                value = readByte(NUVOTON_TEMPERATURE_REG[index]) << 1;
                break;
                
            case NCT6779D:
            case NCT6791D:
                value = readByte(NUVOTON_TEMPERATURE_REG_NEW[index]) << 1;
                break;
        }

        float t = 0.5f * (float)value;
        
        return t <= 125 && t >= -55 ? t : 0;
    }
    
    
	return 0;
}

float NCT677xSensors::readVoltage(UInt32 index)
{
    if (index < voltLimit) {
        
        float value;
        
        switch (model) {
            case NCT6771F:
            case NCT6776F:    
                value = readByte(NUVOTON_VOLTAGE_REG[index]) * (NUVOTON_VOLTAGE_SCALE[index]) * 0.001f;
                break;
                
            case NCT6779D:
            case NCT6791D:
                value = readByte(NUVOTON_VOLTAGE_REG_NEW[index]) * 0.008f;
                break;
        }

        bool valid = value > 0;
        
        // check if battery voltage monitor is enabled
        if (valid && NUVOTON_VOLTAGE_REG[index] == voltageVBatRegister) {
            valid = (readByte(0x5D) & 0x01) > 0;
        }
        
        return valid ? value : 0;
    }
    
    return 0;
}

float NCT677xSensors::readTachometer(UInt32 index)
{
    if (index < fanLimit) {
        UInt8 high = readByte(fanRpmBaseRegister + (index << 1));
        UInt8 low = readByte(fanRpmBaseRegister + (index << 1) + 1);
        
        int value = (high << 8) | low;
        
        return value > minFanRPM ? value : 0;
    }
    
    return 0;
}

bool NCT677xSensors::supportsTachometerControl()
{
    return true;
}

UInt8 NCT677xSensors::readTachometerControl(UInt32 index)
{
    return (float)(readByte(NUVOTON_FAN_PWM_OUT_REG[index])) / 2.55f;
}

void NCT677xSensors::writeTachometerControl(UInt32 index, UInt8 percent)
{
    if (index < fanLimit && !fanControlEnabled[index]) {

        fanDefaultControl[index] = readByte(NUVOTON_FAN_CONTROL_MODE_REG[index]);
        fanDefaultCommand[index] = readByte(NUVOTON_FAN_PWM_COMMAND_REG[index]);

        // set manual mode
        writeByte(NUVOTON_FAN_CONTROL_MODE_REG[index], 0);

//        switch (model) {
//            case NCT6771F: {
//                UInt8 reg = readByte(NUVOTON_FAN_PWM_MODE_OLD_REG[index]);
//                reg &= ~NUVOTON_PWM_MODE_MASK_OLD[index];
//                
//                UInt8 val = 0; // 0 - DC mode; 1 - PWM mode
//                
//                if (val)
//                    reg |= ~NUVOTON_PWM_MODE_MASK_OLD[index];
//                
//                writeByte(NUVOTON_FAN_PWM_MODE_OLD_REG[index], reg);
//                break;
//            }
//                
//            default: {
//                UInt8 reg = readByte(NUVOTON_FAN_PWM_MODE_REG[index]);
//                reg &= ~NUVOTON_PWM_MODE_MASK[index];
//                
//                UInt8 val = 0; // 0 - DC mode; 1 - PWM mode
//                
//                if (val)
//                    reg |= ~NUVOTON_PWM_MODE_MASK[index];
//                
//                writeByte(NUVOTON_FAN_PWM_MODE_REG[index], reg);
//                break;
//            }
//        }

        fanControlEnabled[index] = true;
    }

    UInt8 value = (float)(percent) * 2.55;

    // set output value
    writeByte(NUVOTON_FAN_PWM_COMMAND_REG[index], value);

//    writeByte(NUVOTON_FAN_PWM_OUT_REG[index], value);
//    
//	if (index == 2)	{ /* floor: disable if val == 0 */
//		UInt8 reg = readByte(NUVOTON_TEMPERATURE_SEL_REG[index]);
//		reg &= 0x7f;
//		if (value) reg |= 0x80;
//		writeByte(NUVOTON_TEMPERATURE_SEL_REG[index], reg);
//	}
}

void NCT677xSensors::disableTachometerControl(UInt32 index)
{
    // Restore fan control mode
    writeByte(NUVOTON_FAN_CONTROL_MODE_REG[index], fanDefaultControl[index]);

    // Restore PWM command value
    writeByte(NUVOTON_FAN_PWM_COMMAND_REG[index], fanDefaultCommand[index]);

    fanControlEnabled[index] = false;
}

bool NCT677xSensors::initialize()
{
    UInt16 vendor = (UInt16)(readByte(NUVOTON_VENDOR_ID_HIGH_REGISTER) << 8) | readByte(NUVOTON_VENDOR_ID_LOW_REGISTER);
    
    if (vendor != NUVOTON_VENDOR_ID)
    {
        HWSensorsWarningLog("wrong vendor ID=0x%x", vendor);
        //return false;
    }
    
    switch (model) {
        case NCT6771F:
            fanLimit = 3;
            tempLimit = 9;
            voltLimit = 9;
            fanRpmBaseRegister = 0x656;
            voltageVBatRegister = 0x551;
            minFanRPM = (int)(1.35e6 / 0xFFFF);
            break;
            
        case NCT6776F:
            fanLimit = 3;
            tempLimit = 9;
            voltLimit = 9;
            fanRpmBaseRegister = 0x656;
            voltageVBatRegister = 0x551;
            minFanRPM = (int)(1.35e6 / 0x1FFF);
            break;
            
        case NCT6779D:
            fanLimit = 5;
            tempLimit = 7;
            voltLimit = 15;
            fanRpmBaseRegister = 0x4C0;
            voltageVBatRegister = 0x488;
            minFanRPM = (int)(1.35e6 / 0x1FFF);
            break;

        case NCT6791D:
            fanLimit = 6;
            tempLimit = 7;
            voltLimit = 15;
            fanRpmBaseRegister = 0x4C0;
            voltageVBatRegister = 0x488;
            minFanRPM = (int)(1.35e6 / 0x1FFF);


            
            break;
    }

	return true;
}

void NCT677xSensors::didPoweredOn()
{
    switch (model) {
        case NCT6771F: {
            // disable the hardware monitor i/o space lock on NCT6791D chips
            winbond_family_enter(port);

            superio_select_logical_device(port, kWinbondHardwareMonitorLDN);

            /* Activate logical device if needed */
            UInt8 options = superio_listen_port_byte(port, NUVOTON_REG_ENABLE);

            if (!(options & 0x01)) {
                superio_write_port_byte(port, NUVOTON_REG_ENABLE, options | 0x01);
            }

            options = superio_listen_port_byte(port, NUVOTON_HWMON_IO_SPACE_LOCK);

            // if the i/o space lock is enabled
            if (options & 0x10) {
                // disable the i/o space lock
                superio_write_port_byte(port, NUVOTON_HWMON_IO_SPACE_LOCK, (UInt8)(options & ~0x10));
            }

            winbond_family_exit(port);
            
            break;
        }
    }
}
