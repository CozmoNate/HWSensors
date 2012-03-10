/*
 *  SuperIOFamily.h
 *  HWSensors
 *
 *  Created by mozo on 08/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _SUPERIOMONITOR_H
#define _SUPERIOMONITOR_H

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>

#include "FakeSMCPlugin.h"

// Ports
const UInt8 SUPERIO_STANDART_PORT[]					= { 0x2e, 0x4e };

// Registers
const UInt8 SUPERIO_CONFIGURATION_CONTROL_REGISTER	= 0x02;
const UInt8 SUPERIO_DEVICE_SELECT_REGISTER			= 0x07;
const UInt8 SUPERIO_CHIP_ID_REGISTER				= 0x20;
const UInt8 SUPERIO_CHIP_REVISION_REGISTER			= 0x21;
const UInt8 SUPERIO_BASE_ADDRESS_REGISTER			= 0x60;

// cool values (:
#define kSuperIOTemperatureSensor   1501 
#define kSuperIOVoltageSensor       1502

class SuperIOMonitor : public FakeSMCPlugin {
	OSDeclareAbstractStructors(SuperIOMonitor)
	
protected:
	UInt16					address;
	UInt8					port;
	UInt32					model;
    
	const char              *modelName;
    const char              *vendorName;
    
    virtual bool            addTemperatureSensors(OSDictionary *configuration);
    virtual bool            addVoltageSensors(OSDictionary *configuration);
    virtual bool            addTachometerSensors(OSDictionary *configuration);
    
    virtual UInt8           temperatureSensorsLimit();
    virtual UInt8           voltageSensorsLimit();
    virtual UInt8           tachometerSensorsLimit();
    
    virtual SInt32			readTemperature(UInt32 index);
	virtual float			readVoltage(UInt32 index);
	virtual SInt32			readTachometer(UInt32 index);
    
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
    virtual bool            initialize();
		
public:
	virtual bool			init(OSDictionary *properties=0);
	virtual IOService*		probe(IOService *provider, SInt32 *score);
    virtual bool			start(IOService *provider);
};

#endif