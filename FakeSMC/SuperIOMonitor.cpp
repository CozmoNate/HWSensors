/*
 *  SuperIOFamily.cpp
 *  HWSensors
 *
 *  Created by mozo on 08/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "SuperIOMonitor.h"

#include <architecture/i386/pio.h>

#include "FakeSMCDefinitions.h"

#define Debug FALSE

#define LogPrefix "SuperIOMonitor: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super FakeSMCPlugin
OSDefineMetaClassAndAbstractStructors(SuperIOMonitor, FakeSMCPlugin)

UInt8 SuperIOMonitor::listenPortByte(UInt16 reg)
{
	outb(registerPort, reg);
	return inb(valuePort);
}

UInt16 SuperIOMonitor::listenPortWord(UInt16 reg)
{
	return ((listenPortByte(reg) << 8) | listenPortByte(reg + 1));
}

void SuperIOMonitor::selectLogicalDevice(UInt8 num)
{
	outb(registerPort, SUPERIO_DEVICE_SELECT_REGISTER);
	outb(valuePort, num);
}

bool SuperIOMonitor::getLogicalDeviceAddress(UInt8 reg)
{
	address = listenPortWord(reg);
    
    if (address < 0x100 || (address & 0xF007) != 0)
		return false;
	
	IOSleep(100);
	
	return address == listenPortWord(reg);
}

UInt8 SuperIOMonitor::getPortsCount() 
{ 
	return 2; 
};

void SuperIOMonitor::selectPort(unsigned char index)
{
	registerPort = SUPERIO_STANDART_PORT[index]; 
	valuePort = SUPERIO_STANDART_PORT[index] + 1;
}

bool SuperIOMonitor::probePort() 
{		
	return true; 
};

bool SuperIOMonitor::addTemperatureSensors(OSDictionary *configuration)
{
    DebugLog("adding temperature sensors...");
    
    for (int i = 0; i < temperatureSensorsLimit(); i++) 
    {				
        char key[8];
        
        snprintf(key, 8, "TEMPIN%X", i);
        
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

bool SuperIOMonitor::addVoltageSensors(OSDictionary *configuration)
{
    DebugLog("adding voltage sensors...");
    
    for (int i = 0; i < voltageSensorsLimit(); i++)
    {				
        char key[5];
        
        snprintf(key, 5, "VIN%X", i);
        
        if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key))) {
            if (name->isEqualTo("CPU")) {
                if (!addSensor(KEY_CPU_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add  CPU voltage sensor");
            }
            else if (name->isEqualTo("Memory")) {
                if (!addSensor(KEY_MEMORY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add  memory voltage sensor");
            }
            else if (name->isEqualTo("DCIN_3V")) {
                if (!addSensor(KEY_DCIN_3V3_S5_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add  DCIN_3V voltage Sensor!");
            }
            else if (name->isEqualTo("DCIN_12V")) {
                if (!addSensor(KEY_DCIN_12V_S0_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add  DCIN_12V voltage Sensor!");
            }        
            else if (name->isEqualTo("CPU_PLL")) {
                if (!addSensor(KEY_CPU_PLL_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add CPU_PLL voltage Sensor!");
            }
            else if (name->isEqualTo("Battery")) {
                if (!addSensor(KEY_POWERBATTERY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                    WarningLog("can't add Battery voltage sensor!");
            }
            
            for (int j = 0; j <= 0xf; j++) {
                
                snprintf(key, 5, "VRM%X", j);
                
                if (name->isEqualTo(key)) {
                    snprintf(key, 5, KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE, j);
                    if (!addSensor(key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                        WarningLog("can't add VRM%X voltage Sensor!", j);
                }
                else {
                    snprintf(key, 5, "PWR%X", j);
                    if (name->isEqualTo(key)) {
                        snprintf(key, 5, KEY_FORMAT_POWERSUPPLY_VOLTAGE, j);
                        if (!addSensor(key, TYPE_FP4C, TYPE_FPXX_SIZE, kSuperIOVoltageSensor, i))
                            WarningLog("can't add PWR%X voltage Sensor!", j);
                    }
                }
            }
        }
    }

    return true;
}

bool SuperIOMonitor::addTachometerSensors(OSDictionary *configuration)
{
    DebugLog("adding tachometer sensors...");
    
    for (int i = 0; i < tachometerSensorsLimit(); i++) {
        OSString* name = NULL;
        
        char key[7];
            
        snprintf(key, 7, "FANIN%X", i);
            
        name = OSDynamicCast(OSString, configuration->getObject(key));
        
        UInt64 nameLength = name ? name->getLength() : 0;
        
        if (readTachometer(i) > 10 || nameLength > 0)
            if (!addTachometer(i, (nameLength > 0 ? name->getCStringNoCopy() : 0)))
                WarningLog("error adding tachometer sensor %d", i);
    }
    
    return true;
}

UInt8 SuperIOMonitor::temperatureSensorsLimit()
{
    return 3;
}

UInt8 SuperIOMonitor::voltageSensorsLimit()
{
    return 9;
}

UInt8 SuperIOMonitor::tachometerSensorsLimit()
{
    return 5;
}

SInt32 SuperIOMonitor::readTemperature(UInt32 index)
{
	return 0;
}

float SuperIOMonitor::readVoltage(UInt32 index)
{
	return 0;
}

SInt32 SuperIOMonitor::readTachometer(UInt32 index)
{
	return 0;
}

void SuperIOMonitor::enter()
{
	//
};

void SuperIOMonitor::exit()
{
	//
};

const char *SuperIOMonitor::getModelName()
{
	return "Unknown";
}

float SuperIOMonitor::getSensorValue(FakeSMCSensor *sensor)
{
    if (sensor)
        switch (sensor->getGroup()) {
            case kSuperIOTemperatureSensor:
                return (float)readTemperature(sensor->getIndex());
                
            case kSuperIOVoltageSensor:
                return readVoltage(sensor->getIndex());
                
            case kFakeSMCTachometerSensor:
                return (float)readTachometer(sensor->getIndex());
        }
    
	return 0;
}

bool SuperIOMonitor::init(OSDictionary *properties)
{
	DebugLog("initialising...");
	
	if (!super::init(properties))
		return false;
	
	model = 0;

	return true;
}

IOService *SuperIOMonitor::probe(IOService *provider, SInt32 *score)
{
	DebugLog("probing...");
	
	if (super::probe(provider, score) != this) 
		return 0;
    
    // try 2 times...
    for (UInt8 j = 0; j < 2; j++) {        
        for (UInt8 i = 0; i < getPortsCount(); i++) {	
            selectPort(i);
            
            enter();
            
            if (probePort()) {
                
                isActive = true;
                
                exit();
                
                return this;
            }
            
            exit();
        }
        
        IOSleep(100);
    }
	
	return this;
}

bool SuperIOMonitor::start(IOService *provider)
{		
	DebugLog("starting...");
	
	if (!super::start(provider)) 
        return false;
        
    InfoLog("found %s", getModelName());
        
    OSDictionary* list = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration"));
    OSDictionary* configuration = list ? OSDynamicCast(OSDictionary, list->getObject(getModelName())) : 0;
	
    if (list && !configuration) 
        configuration = OSDynamicCast(OSDictionary, list->getObject("Default"));
    
	if (configuration) {    
        addTemperatureSensors(configuration);
        addVoltageSensors(configuration);
        addTachometerSensors(configuration);
    }
        
    registerService(0);

	return true;
}