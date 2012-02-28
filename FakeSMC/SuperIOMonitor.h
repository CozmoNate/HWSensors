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

// Ports
const UInt8 SUPERIO_STANDART_PORT[]					= { 0x2e, 0x4e };

// Registers
const UInt8 SUPERIO_CONFIGURATION_CONTROL_REGISTER	= 0x02;
const UInt8 SUPERIO_DEVICE_SELECT_REGISTER			= 0x07;
const UInt8 SUPERIO_CHIP_ID_REGISTER				= 0x20;
const UInt8 SUPERIO_CHIP_REVISION_REGISTER			= 0x21;
const UInt8 SUPERIO_BASE_ADDRESS_REGISTER			= 0x60;

enum SuperIOSensorGroup {
	kSuperIOTemperatureSensor,
	kSuperIOTachometerSensor,
	kSuperIOVoltageSensor
};

class SuperIOMonitor;

class SuperIOSensor : public OSObject {
	 OSDeclareDefaultStructors(SuperIOSensor)
	
protected:
	SuperIOMonitor      *owner;
	char                *name;
	char                *type;
	UInt8               size;
	SuperIOSensorGroup	group;
	UInt32              index;
	
public:
	static SuperIOSensor *withOwner(SuperIOMonitor *aOwner, const char* aKey, const char* aType, UInt8 aSize, SuperIOSensorGroup aGroup, UInt32 aIndex);
    
   	virtual bool		initWithOwner(SuperIOMonitor *aOwner, const char* aKey, const char* aType, UInt8 aSize, SuperIOSensorGroup aGroup, UInt32 aIndex);
   	virtual void		free();
	
	const char *		getName();
	const char *		getType();
	UInt8               getSize();
	SuperIOSensorGroup	getGroup();
	UInt32              getIndex();
	virtual UInt16		getValue();
};

class SuperIOMonitor : public IOService {
	OSDeclareAbstractStructors(SuperIOMonitor)
	
protected:
	IOService *				fakeSMC;
    
    bool                    isActive;
		
	UInt16					address;
	UInt8					registerPort;
	UInt8					valuePort;
	
	UInt32					model;
	
	OSArray *				sensors;
	
	UInt8					listenPortByte(UInt16 reg);
	UInt16					listenPortWord(UInt16 reg);
	void					selectLogicalDevice(UInt8 num);
	bool					getLogicalDeviceAddress(UInt8 reg = SUPERIO_BASE_ADDRESS_REGISTER);
	
	virtual UInt8			getPortsCount();
	virtual void			selectPort(unsigned char index);
	virtual void			enter();
	virtual void			exit();
	virtual bool			probePort();
   	virtual bool			startPlugin();
	
	virtual const char *	getModelName();
	
	SuperIOSensor *			addSensor(const char* key, const char* type, UInt8 size, SuperIOSensorGroup group, UInt32 index);
	SuperIOSensor *			addTachometer(UInt32 index, const char* id = 0);
	SuperIOSensor *			getSensor(const char* key);
		
public:
	virtual UInt16			readTemperature(UInt32 index);
	virtual UInt16			readVoltage(UInt32 index);
	virtual UInt16			readTachometer(UInt32 index);
	
	virtual bool			init(OSDictionary *properties=0);
	virtual IOService*		probe(IOService *provider, SInt32 *score);
    virtual bool			start(IOService *provider);
	virtual void			stop(IOService *provider);
	virtual void			free(void);
	
	virtual IOReturn		callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
	
};

#endif