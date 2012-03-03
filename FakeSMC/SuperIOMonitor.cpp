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

bool SuperIOMonitor::startPlugin()
{
    return true;
};

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
    
    if (startPlugin())
        registerService(0);
	
	return true;
}