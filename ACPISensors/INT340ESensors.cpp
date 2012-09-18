//
//  INT340E.cpp
//  HWSensors
//
//  Created by kozlek on 24.08.12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "INT340ESensors.h"

#include "FakeSMCDefinitions.h"

//#define kHWSensorsDebug TRUE

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(INT340ESensors, FakeSMCPlugin)

bool INT340ESensors::updateTemperatures()
{
    OSObject *object;
    
    if (kIOReturnSuccess == acpiDevice->evaluateObject("TSDD", &object) && object) {
        OSSafeRelease(temperatures);
        
        temperatures = OSDynamicCast(OSArray, object);
        
        //setProperty("temperatures", temperatures);
        
        return true;
    }
    
    HWSensorsWarningLog("failed to evaluate TSDD method");
    
    return false;
}

bool INT340ESensors::updateTachometers()
{
    OSObject *object;
    
    if (kIOReturnSuccess == acpiDevice->evaluateObject("OSDD", &object) && object) {
        OSSafeRelease(tachometers);
        
        tachometers = OSDynamicCast(OSArray, object);
        
        //setProperty("temperatures", temperatures);
        
        return true;
    }
    
    HWSensorsWarningLog("failed to evaluate OSDD method");
    
    return false;
}

float INT340ESensors::readTemperature(UInt32 index)
{
    mach_timespec_t now;
    
    clock_get_system_nanotime((clock_sec_t*)&now.tv_sec, (clock_nsec_t*)&now.tv_nsec);
    
    if (CMP_MACH_TIMESPEC(&temperatureNextUpdate, &now) <= 0) {
        mach_timespec_t next;
        
        temperatureNextUpdate.tv_sec = now.tv_sec;
        temperatureNextUpdate.tv_nsec = now.tv_nsec;
        next.tv_sec = 1;
        next.tv_nsec = 0;
        
        ADD_MACH_TIMESPEC(&temperatureNextUpdate, &next);
        
        updateTemperatures();
    }
    
    if (temperatures) {
        if (OSNumber *number = OSDynamicCast(OSNumber, temperatures->getObject(index))) {
            UInt64 value = number->unsigned32BitValue();
            return (value == 0x80000000) ? 0 : (float)((value - 0xAAC) / 0xA);
        }
    }
    
    return 0;
}

float INT340ESensors::readTachometer(UInt32 index)
{
    mach_timespec_t now;
    
    clock_get_system_nanotime((clock_sec_t*)&now.tv_sec, (clock_nsec_t*)&now.tv_nsec);
    
    if (CMP_MACH_TIMESPEC(&tachometerNextUpdate, &now) <= 0) {
        mach_timespec_t next;
        
        tachometerNextUpdate.tv_sec = now.tv_sec;
        tachometerNextUpdate.tv_nsec = now.tv_nsec;
        next.tv_sec = 1;
        next.tv_nsec = 0;
        
        ADD_MACH_TIMESPEC(&tachometerNextUpdate, &next);
        
        updateTachometers();
    }
    
    if (tachometers) {
        if (OSNumber *number = OSDynamicCast(OSNumber, tachometers->getObject(index))) {
            UInt64 value = number->unsigned32BitValue();
            return (value == 0x80000000) ? 0 : (float)value;
        }
    }
    
    return 0;
}

float INT340ESensors::getSensorValue(FakeSMCSensor *sensor)
{
    switch(sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            return readTemperature(sensor->getIndex());
        case kFakeSMCTachometerSensor:
            return readTachometer(sensor->getIndex());
    }
    
    return 0;
}

void INT340ESensors::parseTemperatureName(OSString *name, UInt32 index)
{
    if (name && readTemperature(index)) {
        char key[5];
        char str[64];
        
        if (name->isEqualTo("CPU Core Package DTS") || name->isEqualTo("CPU Package Temperature"))
            snprintf(key, 5, KEY_CPU_PACKAGE_TEMPERATURE);
        else if (name->isEqualTo("CPU Temperature"))
            snprintf(key, 5, KEY_CPU_PROXIMITY_TEMPERATURE);
        else if (name->isEqualTo("PCH Temperature") || name->isEqualTo("PCH DTS Temperature from PCH"))
            snprintf(key, 5, KEY_PCH_DIE_TEMPERATURE);
        else if (name->isEqualTo("MCH DTS Temperature from PCH"))
            snprintf(key, 5, KEY_MCH_DIODE_TEMPERATURE);
        else if (name->isEqualTo("Ambient Temperature"))
            snprintf(key, 5, KEY_AMBIENT_TEMPERATURE);
        else if (!strlen(key)) {
            for (UInt8 i = 0; i < 4; i++) {
                snprintf(str, 64, "TS-on-DIMM%x Temperature", i);
                
                if (name->isEqualTo(str)) {
                    snprintf(key, 5, KEY_FORMAT_DIMM_TEMPERATURE, i);
                    break;
                }
            }
            
            if (!strlen(key)) {
                for (UInt8 i = 0; i < 8; i++) {
                    snprintf(str, 64, "TZ0%x _TMP", i);
                    
                    if (name->isEqualTo(str)) {
                        snprintf(key, 5, KEY_FORMAT_THERMALZONE_TEMPERATURE, i + 1);
                        break;
                    }
                    
                    snprintf(str, 64, "CPU Core %x DTS", i);
                    
                    if (name->isEqualTo(str)) {
                        snprintf(key, 5, KEY_FORMAT_CPU_DIODE_TEMPERATURE, i);
                        break;
                    }
                }
            }
        }
        
        if (strlen(key))
            addSensor(key, TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, index);
    }
    
}

void INT340ESensors::parseTachometerName(OSString *name, UInt32 index)
{
    if (name && readTemperature(index)) {
        if (name->isEqualTo("CPU Fan Speed"))
            this->addTachometer(index, "CPU Fan Speed");
        else if (name->isEqualTo("Fan RPM"))
            this->addTachometer(index);
        else if (name->isEqualTo("RPM"))
            switch (version) {
                case 0x30000:
                    this->addTachometer(index);
                    break;
                case 0x20001:
                    this->addTachometer(index - 1);
                    break;
            }
    }
}

bool INT340ESensors::start(IOService * provider)
{
	if (!super::start(provider))
        return false;
    
	acpiDevice = (IOACPIPlatformDevice *)provider;
	
	if (!acpiDevice) {
        HWSensorsWarningLog("ACPI device not ready");
        return false;
    }
    
    // Update timers
    clock_get_system_nanotime((clock_sec_t*)&temperatureNextUpdate.tv_sec, (clock_nsec_t*)&temperatureNextUpdate.tv_nsec);
    
    acpiDevice->evaluateInteger("IVER", &version);
    
    if (version == 0) {
        OSString *name = OSDynamicCast(OSString, getProperty("IONameMatched"));
        
        if (name && name->isEqualTo("INT3F0D"))
            version = 0x30000;
        else
            return false;
    }
    
    setProperty("version", version, 64);
    
    // Parse sensors
    switch (version) {
        case 0x30000: {
            OSObject *object = NULL;
            
            // Temperatures
            if(kIOReturnSuccess == acpiDevice->evaluateObject("TSDL", &object) && object) {
                
                OSArray *description = OSDynamicCast(OSArray, object);
                
                if (OSIterator *iterator = OSCollectionIterator::withCollection(description)) {
                    
                    HWSensorsDebugLog("Parsing temperatures...");
                    
                    UInt32 count = 0;
                    
                    while (OSObject *item = iterator->getNextObject()) {
                        parseTemperatureName(OSDynamicCast(OSString, item), count / 2);
                        count += 2;
                    }
                }
            }
            else HWSensorsWarningLog("failed to evaluate TSDL table");
            
            // Tachometers
            if(kIOReturnSuccess == acpiDevice->evaluateObject("OSDL", &object) && object) {
                
                OSArray *description = OSDynamicCast(OSArray, object);
                
                if (OSIterator *iterator = OSCollectionIterator::withCollection(description)) {
                    
                    HWSensorsDebugLog("Parsing tachometers...");
                    
                    UInt32 count = 0;
                    
                    while (OSObject *item = iterator->getNextObject()) {
                        parseTachometerName(OSDynamicCast(OSString, item), count / 3);
                        count += 3;
                    }
                }
            }
            else HWSensorsWarningLog("failed to evaluate OSDL table");
            
            break;
        }
            
        case 0x20001: {
            OSObject *object = NULL;
            
            // Temperatures
            if(kIOReturnSuccess == acpiDevice->evaluateObject("TMPV", &object) && object) {
                
                OSArray *description = OSDynamicCast(OSArray, object);
                
                if (OSIterator *iterator = OSCollectionIterator::withCollection(description)) {
                    
                    HWSensorsDebugLog("Parsing temperatures...");
                    
                    UInt32 count = 0;
                    
                    while (OSObject *item = iterator->getNextObject()) {
                        parseTemperatureName(OSDynamicCast(OSString, item), count + 1);
                        count += 3;
                    }
                }
            }
            else HWSensorsWarningLog("failed to evaluate TMPV table");
            
            // Tachometers
            if(kIOReturnSuccess == acpiDevice->evaluateObject("OSDV", &object) && object) {
                
                OSArray *description = OSDynamicCast(OSArray, object);
                
                if (OSIterator *iterator = OSCollectionIterator::withCollection(description)) {
                    
                    HWSensorsDebugLog("Parsing tachometers...");
                    
                    UInt32 count = 0;
                    
                    while (OSObject *item = iterator->getNextObject()) {
                        parseTachometerName(OSDynamicCast(OSString, item), count + 2);
                        count++;
                    }
                }
            }
            else HWSensorsWarningLog("failed to evaluate OSDV table");
            
            break;
        }
            
        default:
            HWSensorsWarningLog("usupported interface version: 0x%x", (UInt32)version);
            return false;
    }
    
    registerService();
    
    HWSensorsInfoLog("started");
    
	return true;
}