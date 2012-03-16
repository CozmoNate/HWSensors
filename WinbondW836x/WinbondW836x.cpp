/*
 *  W836x.cpp
 *  HWSensors
 *
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *
 *  Created by mozo on 14/10/10.
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

#include "WinbondW836x.h"
#include "FakeSMCDefinitions.h"
#include "SuperIO.h"

#define Debug FALSE

#define LogPrefix "W836xMonitor: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super SuperIOMonitor
OSDefineMetaClassAndStructors(W836xMonitor, SuperIOMonitor)

UInt8 W836xMonitor::readByte(UInt8 bank, UInt8 reg) 
{
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	return inb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET));
}

void W836xMonitor::writeByte(UInt8 bank, UInt8 reg, UInt8 value)
{
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), value); 
}

UInt64 W836xMonitor::setBit(UInt64 target, UInt16 bit, UInt32 value)
{
	if (((value & 1) == value) && bit <= 63)
	{
		UInt64 mask = (((UInt64)1) << bit);
		return value > 0 ? target | mask : target & ~mask;
	}
	
	return value;
}

UInt8 W836xMonitor::temperatureSensorsLimit()
{
    return 3;
}

UInt8 W836xMonitor::voltageSensorsLimit()
{
    return voltageLimit;
}

UInt8 W836xMonitor::tachometerSensorsLimit()
{
    return fanLimit;
}

float W836xMonitor::readTemperature(UInt32 index)
{
	UInt32 value = readByte(WINBOND_TEMPERATURE_BANK[index], WINBOND_TEMPERATURE[index]) << 1;
	
	if (WINBOND_TEMPERATURE_BANK[index] > 0) 
		value |= readByte(WINBOND_TEMPERATURE_BANK[index], (UInt8)(WINBOND_TEMPERATURE[index] + 1)) >> 7;
	
	float temperature = (float)value / 2.0f;
	
	return temperature <= 125 && temperature >= -55 ? temperature : 0;
}

float W836xMonitor::readVoltage(UInt32 index)
{
    float voltage = 0;
    
    if (WINBOND_VOLTAGE[index] != WINBOND_VOLTAGE_VBAT) {
        
        UInt16 V = 0;
        
        switch (model) 
        {
            case W83627EHF:
            case W83627DHG:
            case W83627DHGP:        
            case W83667HG:
            case W83667HGB:
                V = readByte(WINBOND_VOLTAGE_BANK[index], WINBOND_VOLTAGE[index]);
                break;
            case W83627HF:
            case W83627THF:
            case W83687THF:
                V = readByte(WINBOND_VOLTAGE1_BANK[index], WINBOND_VOLTAGE1[index]);
                break;
        }
        
        if (index == 0 && (model == W83627HF || model == W83627THF || model == W83687THF)) 
        {
            UInt8 vrmConfiguration = readByte(0, 0x18);
            
            if ((vrmConfiguration & 0x01) == 0)
                voltage = 0.016f * (float)V; // VRM8 formula
            else
                voltage = 0.00488f * (float)V + 0.69f; // VRM9 formula
        }
        else voltage = (float)V * voltageGain;
    }
	else {
        // Battery voltage
        if ((readByte(0, 0x5D) & 0x01) > 0)
            voltage = readByte(5, WINBOND_VOLTAGE_VBAT) * voltageGain;
    }
	
	return voltage;
}

void W836xMonitor::updateTachometers()
{
	UInt64 bits = 0;
	
	for (int i = 0; i < 5; i++)
	{
		bits = (bits << 8) | readByte(0, WINBOND_TACHOMETER_DIVISOR[i]);
	}
	
	UInt64 newBits = bits;
	
	for (int i = 0; i < fanLimit; i++)
	{
		// assemble fan divisor
		UInt8 offset =	(((bits >> WINBOND_TACHOMETER_DIVISOR2[i]) & 1) << 2) |
		(((bits >> WINBOND_TACHOMETER_DIVISOR1[i]) & 1) << 1) |
		((bits >> WINBOND_TACHOMETER_DIVISOR0[i]) & 1);
		
		UInt8 divisor = 1 << offset;
		UInt8 count = readByte(WINBOND_TACHOMETER_BANK[i], WINBOND_TACHOMETER[i]);
		
		// update fan divisor
		if (count > 192 && offset < 7)
		{
			offset++;
		}
		else if (count < 96 && offset > 0)
		{
			offset--;
		}
		
		fanValue[i] = (count < 0xff) ? 1.35e6f / (float(count * divisor)) : 0;
		fanValueObsolete[i] = false;
		
		newBits = setBit(newBits, WINBOND_TACHOMETER_DIVISOR2[i], (offset >> 2) & 1);
		newBits = setBit(newBits, WINBOND_TACHOMETER_DIVISOR1[i], (offset >> 1) & 1);
		newBits = setBit(newBits, WINBOND_TACHOMETER_DIVISOR0[i],  offset       & 1);
	}		
	
	// write new fan divisors 
	for (int i = 4; i >= 0; i--) 
	{
		UInt8 oldByte = bits & 0xff;
		UInt8 newByte = newBits & 0xff;
		
		if (oldByte != newByte)
		{
			writeByte(0, WINBOND_TACHOMETER_DIVISOR[i], newByte);
		}
		
		bits = bits >> 8;
		newBits = newBits >> 8;
	}
}

float W836xMonitor::readTachometer(UInt32 index)
{
	if (fanValueObsolete[index])
		updateTachometers();
	
	fanValueObsolete[index] = true;
	
	return fanValue[index];
}

bool W836xMonitor::addTemperatureSensors(OSDictionary *configuration)
{
    DebugLog("adding temperature sensors...");
    
    UInt8 flag = 0;
    
    switch (model) 
    {
        case W83667HG:
        case W83667HGB:
        case W83627DHG:        
        case W83627DHGP:
            // do not add temperature sensor registers that read PECI
            flag = readByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
            break;                
    }
    
    int index = 0;
    
    for (int i = 0; i < temperatureSensorsLimit(); i++) 
    {				
        switch (model) 
        {
            case W83667HG:
            case W83667HGB:
                if ((i == 0 && (flag & 0x04) == 0) || (i == 1 && (flag & 0x40) == 0))
                    continue;
                break;
                
            case W83627DHG:        
            case W83627DHGP:
                if ((i == 0 && (flag & 0x07) == 0) || (i == 1 && (flag & 0x70) == 0)) 
                    continue;
                break;
        }
        
        char key[8];
        
        snprintf(key, 8, "TEMPIN%X", index++);
        
        if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key))) {
            if (name->isEqualTo("CPU")) {
                if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor, i))
                    WarningLog("can't add CPU temperature sensor");
            }
            else if (name->isEqualTo("System")) {				
                if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor,i))
                    WarningLog("can't add System temperature sensor");
            }
            else if (name->isEqualTo("Ambient")) {				
                if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE, kSuperIOTemperatureSensor,i))
                    WarningLog("can't add Ambient temperature sensor");
            }
        }
    }
    
    return true;
}

bool W836xMonitor::addTachometerSensors(OSDictionary *configuration)
{
    DebugLog("setting fanLimit value...");
    
    OSNumber* fanlimit = OSDynamicCast(OSNumber, configuration->getObject("FANINLIMIT")); 
    
	if (fanlimit && fanlimit->unsigned8BitValue() > 0)
		fanLimit = fanlimit->unsigned8BitValue();
    
    // Be sure readTachometer will report correct values 
    updateTachometers();
    
    for (int i = 0; i < fanLimit; i++)
        readTachometer(i);
    
    return super::addTachometerSensors(configuration);
}

bool W836xMonitor::initialize()
{
    UInt16 vendor = (UInt16)(readByte(0x80, WINBOND_VENDOR_ID_REGISTER) << 8) | readByte(0, WINBOND_VENDOR_ID_REGISTER);
    
    if (vendor != WINBOND_VENDOR_ID) {
        WarningLog("wrong vendor ID=0x%x", vendor);
        return false;
    }
    
    switch (model) 
    {
        case W83627EHF:
            voltageLimit = 10;
            fanLimit = 5;
            voltageGain = 0.008f;
            break;
        case W83627DHG:
        case W83627DHGP:        
        case W83667HG:
        case W83667HGB:
            voltageLimit = 9;
            fanLimit = 5;
            voltageGain = 0.008f;
            break;
        case W83627HF:
        case W83627THF:
        case W83687THF:
            voltageLimit = 7;
            fanLimit = 3;
            voltageGain = 0.016f;
            break;
    }
    
	return true;
}
