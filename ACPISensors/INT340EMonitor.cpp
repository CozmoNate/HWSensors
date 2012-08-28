//
//  INT340E.cpp
//  HWSensors
//
//  Created by Kozlek on 24.08.12.
//
//

#include "INT340EMonitor.h"

#include "FakeSMCDefinitions.h"

//#define kHWSensorsDebug TRUE

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(INT340EMonitor, FakeSMCPlugin)

bool INT340EMonitor::updateTemperatures()
{
    OSObject *object;
    
    if (kIOReturnSuccess == acpiDevice->evaluateObject("TSDD", &object) && object) {
        OSSafeRelease(temperatures);
        
        temperatures = OSDynamicCast(OSArray, object);
        
        //setProperty("temperatures", temperatures);
        
        return true;
    }
    else HWSensorsWarningLog("failed to evaluate TSDD method");
    
    return false;
}

float INT340EMonitor::readTemperature(UInt32 index)
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

float INT340EMonitor::getSensorValue(FakeSMCSensor *sensor)
{
    switch(sensor->getGroup()) {
        case kFakeSMCTemperatureSensor:
            return readTemperature(sensor->getIndex());
    }
    
    return 0;
}

void INT340EMonitor::parseTemperatureName(OSString *name, UInt32 index)
{
    if (name && readTemperature(index)) {
        char key[5];
        char str[64];
        
        for (UInt8 i = 0; i < 8; i++) {
            snprintf(str, 64, "CPU Core %x DTS", i);
            
            if (name->isEqualTo(str)) {
                snprintf(key, 5, KEY_FORMAT_CPU_DIODE_TEMPERATURE, i);
                break;
            }
        }
        
        if (name->isEqualTo("CPU Core Package DTS") || name->isEqualTo("CPU Package Temperature"))
            snprintf(key, 5, KEY_CPU_PACKAGE_TEMPERATURE);
        
        if (name->isEqualTo("CPU Temperature"))
            snprintf(key, 5, KEY_CPU_PROXIMITY_TEMPERATURE);
        
        if (name->isEqualTo("PCH Temperature") || name->isEqualTo("PCH DTS Temperature from PCH"))
            snprintf(key, 5, KEY_PCH_DIE_TEMPERATURE);
        
        if (name->isEqualTo("MCH DTS Temperature from PCH"))
            snprintf(key, 5, KEY_MCH_DIODE_TEMPERATURE);
        
        if (name->isEqualTo("Ambient Temperature"))
            snprintf(key, 5, KEY_AMBIENT_TEMPERATURE);
        
        for (UInt8 i = 0; i < 4; i++) {
            snprintf(str, 64, "TS-on-DIMM%x Temperature", i);
            
            if (name->isEqualTo(str)) {
                snprintf(key, 5, KEY_FORMAT_DIMM_TEMPERATURE, i);
                break;
            }
        }
        
        for (UInt8 i = 0; i < 8; i++) {
            snprintf(str, 64, "TZ0%x _TMP", i);
            
            if (name->isEqualTo(str)) {
                snprintf(key, 5, KEY_FORMAT_THERMALZONE_TEMPERATURE, i + 1);
                break;
            }
        }
        
        if (strlen(key))
            addSensor(key, TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, index);
    }
    
}

bool INT340EMonitor::start(IOService * provider)
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
                        count++;
                    }
                }
            }
            else {
                HWSensorsWarningLog("failed to evaluate TSDL table");
                return false;
            }
            
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
                        count++;
                    }
                }
            }
            else {
                HWSensorsWarningLog("failed to evaluate TMPV table");
                return false;
            }
            
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