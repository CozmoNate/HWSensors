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

#include <architecture/i386/pio.h>
#include "cpuid.h"
#include "FakeSMC.h"

#define Debug FALSE

#define LogPrefix "W836x: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super SuperIOMonitor
OSDefineMetaClassAndStructors(W836x, SuperIOMonitor)

UInt8 W836x::readByte(UInt8 bank, UInt8 reg) 
{
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	return inb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET));
}

void W836x::writeByte(UInt8 bank, UInt8 reg, UInt8 value)
{
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), value); 
}

UInt64 W836x::setBit(UInt64 target, UInt16 bit, UInt32 value)
{
	if (((value & 1) == value) && bit <= 63)
	{
		UInt64 mask = (((UInt64)1) << bit);
		return value > 0 ? target | mask : target & ~mask;
	}
	
	return value;
}

long W836x::readTemperature(unsigned long index)
{
	UInt32 value = readByte(WINBOND_TEMPERATURE_BANK[index], WINBOND_TEMPERATURE[index]) << 1;
	
	if (WINBOND_TEMPERATURE_BANK[index] > 0) 
		value |= readByte(WINBOND_TEMPERATURE_BANK[index], (UInt8)(WINBOND_TEMPERATURE[index] + 1)) >> 7;
	
	float temperature = (float)value / 2.0f;
	
	return temperature <= 125 && temperature >= -55 ? temperature : 0;
}

long W836x::readVoltage(unsigned long index)
{
	float voltage = 0;
	float gain = 1;
	
	UInt16 V = readByte(0, WINBOND_VOLTAGE + index);
	
	if (index == 0 && (model == W83627HF || model == W83627THF || model == W83687THF)) 
	{
		UInt8 vrmConfiguration = readByte(0, 0x18);
		
		if ((vrmConfiguration & 0x01) == 0)
			voltage = 16.0f * V; // VRM8 formula
		else
			voltage = 4.88f * V + 690.0f; // VRM9 formula
	}
	else 
	{
		if (index == 3) gain = 2;
		
		voltage = (V << 3) * gain;
	}
	
	return voltage;
}

void W836x::updateTachometers()
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


long W836x::readTachometer(unsigned long index)
{
	if (fanValueObsolete[index])
		updateTachometers();
	
	fanValueObsolete[index] = true;
	
	return fanValue[index];
}

void W836x::enter()
{
	outb(registerPort, 0x87);
	outb(registerPort, 0x87);
}

void W836x::exit()
{
	outb(registerPort, 0xAA);
	//outb(registerPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	//outb(valuePort, 0x02);
}

bool W836x::probePort()
{
	UInt8 id =listenPortByte(SUPERIO_CHIP_ID_REGISTER);
    
    IOSleep(50);
    
	UInt8 revision = listenPortByte(SUPERIO_CHIP_REVISION_REGISTER);
	
	if (id == 0 || id == 0xff || revision == 0 || revision == 0xff)
		return false;
	
	fanLimit = 3;
	
	switch (id) 
	{		
		case 0x52:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
				case 0x30:
				case 0x40:
				case 0x41:
					model = W83627HF;
					break;
					/*case 0x70:
					 model = W83977CTF;
					 break;
					 case 0xf0:
					 model = W83977EF;
					 break;*/
					
			}
		}
		case 0x59:
		{
			switch (revision & 0xf0)
			{
				case 0x50:
					model = W83627SF;
					break;						
			}
			break;
		}
			
		case 0x60:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
					model = W83697HF;
					fanLimit = 2;
					break;						
			}
			break;
		}
			
			/*case 0x61:
			 {
			 switch (revision & 0xf0)
			 {
			 case 0x00:
			 model = W83L517D;
			 break;						
			 }
			 break;
			 }*/
			
		case 0x68:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
					model = W83697SF;
					fanLimit = 2;
					break;						
			}
			break;
		}
			
		case 0x70:
		{
			switch (revision & 0xf0)
			{
				case 0x80:
					model = W83637HF;
					fanLimit = 5;
					break;						
			}
			break;
		}
			
			
		case 0x82:
		{
			switch (revision & 0xF0)
			{
				case 0x80:
					model = W83627THF;
					break;
			}
			break;
		}
			
		case 0x85:
		{
			switch (revision)
			{
				case 0x41:
					model = W83687THF;
					// No datasheet
					break;
			}
			break;
		}
			
		case 0x88:
		{
			switch (revision & 0xF0)
			{
				case 0x50:
				case 0x60:
					model = W83627EHF;
					fanLimit = 5;
					break;
			}
			break;
		}
			
			/*case 0x97:
			 {
			 switch (revision)
			 {
			 case 0x71:
			 model = W83977FA;
			 break;
			 case 0x73:
			 model = W83977TF;
			 break;
			 case 0x74:
			 model = W83977ATF;
			 break;
			 case 0x77:
			 model = W83977AF;
			 break;
			 }
			 break;
			 }*/	
			
		case 0xA0:
		{
			switch (revision & 0xF0)
			{
				case 0x20: 
					model = W83627DHG;
					fanLimit = 5;
					break;   
			}
			break;
		}
			
		case 0xA2:
		{
			switch (revision & 0xF0)
			{
				case 0x30: 
					model = W83627UHG; 
					fanLimit = 2;
					break;   
			}
			break;
		}
			
		case 0xA5:
		{
			switch (revision & 0xF0)
			{
				case 0x10:
					model = W83667HG;
					fanLimit = 2;
					break;
			}
			break;
		}
			
		case 0xB0:
		{
			switch (revision & 0xF0)
			{
				case 0x70:
					model = W83627DHGP;
					fanLimit = 5;
					break;
			}
			break;
		}
			
		case 0xB3:
		{
			switch (revision & 0xF0)
			{
				case 0x50:
					model = W83667HGB;
                    fanLimit = 4;
					break;
			}
			break; 
		}
			
			/*default: 
			 {
			 switch (id & 0x0f) {
			 case 0x0a:
			 model = W83877F;
			 break;
			 case 0x0b:
			 model = W83877AF;
			 break;
			 case 0x0c:
			 model = W83877TF;
			 break;
			 case 0x0d:
			 model = W83877ATF;
			 break;
			 }
			 }*/
	}
	
	if (!model)
	{
		WarningLog("found unsupported chip ID=0x%x REVISION=0x%x", id, revision);
		return false;
	}
    
	selectLogicalDevice(WINBOND_HARDWARE_MONITOR_LDN);
	
    IOSleep(50);
    
	if (!getLogicalDeviceAddress()) {
        WarningLog("can't get monitoring logical device address");
		return false;
    }
    
    IOSleep(50);
    
    UInt16 vendor = (UInt16)(readByte(0x80, WINBOND_VENDOR_ID_REGISTER) << 8) | readByte(0, WINBOND_VENDOR_ID_REGISTER);
    
    if (vendor != WINBOND_VENDOR_ID)
    {
        WarningLog("wrong vendor ID=0x%x", vendor);
        return false;
    }
    
	return true;
}

bool W836x::startPlugin()
{
    InfoLog("found Winbond %s", getModelName());
	
    OSDictionary* list = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration"));
    OSDictionary* configuration = list ? OSDynamicCast(OSDictionary, list->getObject(getModelName())) : 0;
	
    if (list && !configuration) 
        configuration = OSDynamicCast(OSDictionary, list->getObject("Default"));
	
	OSBoolean* tempin0forced = configuration ? OSDynamicCast(OSBoolean, configuration->getObject("TEMPIN0FORCED")) : 0;
	OSBoolean* tempin1forced = configuration ? OSDynamicCast(OSBoolean, configuration->getObject("TEMPIN1FORCED")) : 0;
	
    OSNumber* fanlimit = configuration ? OSDynamicCast(OSNumber, configuration->getObject("FANINLIMIT")) : NULL; 
    
	if (fanlimit && fanlimit->unsigned8BitValue() > 0)
		fanLimit = fanlimit->unsigned8BitValue();
	
	cpuid_update_generic_info();
	
	bool isCpuCore_i = false;
	
	if (strcmp(cpuid_info()->cpuid_vendor, CPUID_VID_INTEL) != 0) 
	{
		switch (cpuid_info()->cpuid_family)
		{
			case 0x6:
			{
				switch (cpuid_info()->cpuid_model)
				{
					case 0x1A: // Intel Core i7 LGA1366 (45nm)
					case 0x1E: // Intel Core i5, i7 LGA1156 (45nm)
					case 0x25: // Intel Core i3, i5, i7 LGA1156 (32nm)
					case 0x2C: // Intel Core i7 LGA1366 (32nm) 6 Core
						isCpuCore_i = true;
						break;
				}
			}	break;
		}
	}
	
	if (isCpuCore_i)
	{
		// Heatsink
		if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
			return false;
	}
	else 
	{	
		switch (model) 
		{
			case W83667HG:
			case W83667HGB:
			{
				// do not add temperature sensor registers that read PECI
				UInt8 flag = readByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
				
				if ((flag & 0x04) == 0 || (tempin0forced && tempin0forced->getValue()))
				{
					// Heatsink
					if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
						WarningLog("error adding heatsink temperature sensor");
				}
				else if ((flag & 0x40) == 0 || (tempin1forced && tempin1forced->getValue()))
				{
					// Ambient
					if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
						WarningLog("error adding ambient temperature sensor");
				}
				
				// Northbridge
				if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
					WarningLog("error adding system temperature sensor");
				
				break;
			}
				
			case W83627DHG:        
			case W83627DHGP:
			{
				// do not add temperature sensor registers that read PECI
				UInt8 sel = readByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
				
				if ((sel & 0x07) == 0 || (tempin0forced && tempin0forced->getValue())) 
				{
					// Heatsink
					if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
						WarningLog("error adding heatsink temperature sensor");
				}
				else if ((sel & 0x70) == 0 || (tempin1forced && tempin1forced->getValue()))
				{
					// Ambient
					if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
						WarningLog("error adding ambient temperature sensor");
				}
				
				// Northbridge
				if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
					WarningLog("error adding system temperature sensor");
				
				break;
			}
				
			default:
			{
				// no PECI support, add all sensors
				
				// Heatsink
				if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
					WarningLog("error adding heatsink temperature sensor");
				// Ambient
                
				if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
					WarningLog("error adding ambient temperature sensor");
                
				// Northbridge
				if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
					WarningLog("error adding system temperature sensor");
				
				break;
			}
		}
	}
	
	// CPU Vcore
	if (!addSensor(KEY_CPU_VOLTAGE, TYPE_FP2E, 2, kSuperIOVoltageSensor, 0))
		WarningLog("error adding CPU voltage sensor");
	
	// FANs
	for (int i = 0; i < fanLimit; i++) 
		fanValueObsolete[i] = true;
	
	updateTachometers();
	
	for (int i = 0; i < fanLimit; i++) {
		OSString* name = 0;
		
		if (configuration) {
			char key[7];
			
			snprintf(key, 7, "FANIN%X", i);
			
			name = OSDynamicCast(OSString, configuration->getObject(key));
		}
		
		UInt64 nameLength = name ? name->getLength() : 0;
		
		if (readTachometer(i) > 10 || nameLength > 0)
			if (!addTachometer(i, (nameLength > 0 ? name->getCStringNoCopy() : 0)))
				WarningLog("error adding tachometer sensor %d", i);
	}
	
	return true;
}

const char *W836x::getModelName()
{
	switch (model) 
	{
        case W83627DHG:     return "W83627DHG";
        case W83627DHGP:    return "W83627DHG-P";
        case W83627EHF:     return "W83627EHF";
        case W83627HF:      return "W83627HF";
        case W83627THF:     return "W83627THF";
        case W83667HG:      return "W83667HG";
        case W83667HGB:     return "W83667HG-B";
        case W83687THF:     return "W83687THF";
		case W83627SF:      return "W83627SF";
        case W83697HF:      return "W83697HF";
		case W83637HF:      return "W83637HF";
        case W83627UHG:     return "W83627UHG";
        case W83697SF:      return "W83697SF";
	}
	
	return "unknown";
}