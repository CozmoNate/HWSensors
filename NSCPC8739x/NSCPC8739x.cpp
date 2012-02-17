/*
 *  PC8739x.cpp
 *  HWSensors
 *
 *  Created by mozo on 16/10/10.
 *  Copyright 2010 slice. All rights reserved.
 *
 */

#include "NSCPC8739x.h"

#include <architecture/i386/pio.h>
#include "FakeSMC.h"

#define Debug FALSE

#define LogPrefix "PC8739x: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super SuperIOMonitor
OSDefineMetaClassAndStructors(PC8739x, SuperIOMonitor)

UInt8 PC8739x::readByte(UInt8 bank, UInt8 reg) 
{
	// Not needed cose we already selected FAN device
	//outb(m_RegisterPort, SUPERIO_DEVICE_SELECT_REGISTER);
	//outb(m_ValuePort, ldn);
	outb(registerPort, reg);
	return inb(valuePort);
}

void PC8739x::writeByte(UInt8 bank, UInt8 reg, UInt8 value)
{
	//outb(m_RegisterPort, SUPERIO_DEVICE_SELECT_REGISTER);
	//outb(m_ValuePort, ldn);
	outb(registerPort, reg);
	outb(valuePort, value); 
}

long PC8739x::readTemperature(unsigned long index)
{
	return mmioBase[NSC_HARDWARE_MONITOR_REGS[0][index]];
}

long PC8739x::readTachometer(unsigned long index)
{
	return (0xff - (mmioBase[NSC_HARDWARE_MONITOR_REGS[1][index]] & 0xff)) * 20;
}

bool PC8739x::probePort()
{	

	UInt8 id = listenPortByte(SUPERIO_CHIP_ID_REGISTER);	
	revision = listenPortByte(NSC_CHIP_REVISION_REGISTER);
	DebugLog("testing NSC id=%04x rev=%04x", id, revision);
	if (id == 0 || id == 0xff || revision == 0 || revision == 0xff)
		return false;
		
	if (id == 0xfc) {
		selectLogicalDevice(NSC_HARDWARE_MONITOR_LDN);
		
		if (!getLogicalDeviceAddress(SUPERIO_BASE_ADDRESS_REGISTER)){
			DebugLog("NSC no getLogicalDeviceAddress");
			//return false;
		}
//		m_Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
		if (!listenPortByte(NSC_LDN_PRESENT)){
			DebugLog(" no NSC_LDN_PRESENT");
			return false;
		}
		
		switch (revision) {
			default:
				model = PC8739xx;
				break;
		}
		InfoLog("NSC: Found supported chip ID=0x%x REVISION=0x%x ", id, revision);
		return true;
	}
	
	return false;
}

bool PC8739x::startPlugin()
{
    InfoLog("found NSC %s, revision 0x%x", getModelName(), revision);
	
	OSDictionary* configuration = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration"));
	
	UInt32 adr = 
    (listenPortByte(NSC_MEM) & 0xff) + 
    ((listenPortByte(NSC_MEM + 1) << 8) & 0xff00) +
    ((listenPortByte(NSC_MEM + 2) & 0xff) << 16) + 
    ((listenPortByte(NSC_MEM + 3) & 0xff) << 24);
	
	IOPhysicalAddress bar = (IOPhysicalAddress)(adr & ~0xf);	
	
	if(IOMemoryDescriptor *theDescriptor = IOMemoryDescriptor::withPhysicalAddress(bar, 0x200, kIODirectionOutIn))
		if(mmio = theDescriptor->map()){
			mmioBase = (volatile UInt8 *)mmio->getVirtualAddress();
		}
		else {
			WarningLog("MCHBAR failed to map");
			return false;
		}			
	
	// Heatsink
	if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
		WarningLog("error adding heatsink temperature sensor");
    
	// Northbridge
	if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
		WarningLog("error adding system temperature sensor");
	
	// DIMM
	if (!addSensor(KEY_DIMM_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
		WarningLog("error adding DIMM temperature sensor");
	
	// AUX
	if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 3))
		WarningLog("error adding AUX temperature sensor");
	
	// Tachometers
    //	for (int i = 0; i < 5; i++) {  //only one
    OSString* name = 0;
	int i=0;
    if (configuration) {
        char key[7];
        
        snprintf(key, 7, "FANIN%X", i);
        
        name = OSDynamicCast(OSString, configuration->getObject(key));
    }
    
    UInt32 nameLength = name ? name->getLength() : 0;
    
    if (readTachometer(i) > 10 || nameLength > 0)
        if (!addTachometer(i, (nameLength > 0 ? name->getCStringNoCopy() : 0)))
            WarningLog("error adding tachometer sensor %d", i);
    //	}	
	
	return true;
}

const char *PC8739x::getModelName()
{
	switch (model) 
	{
        case PC8739xx: return "PC8739xx";
	}
	
	return "unknown";
}