/*
 *  PC8739x.h
 *  HWSensors
 *
 *  Created by mozo on 16/10/10.
 *  Copyright 2010 slice. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include "SuperIOMonitor.h"
//temp and fans
const UInt16 NSC_HARDWARE_MONITOR_REGS[2][4]	= {{0x61, 0x62, 0x63, 0x64}, {0x68, 0x68, 0x68, 0x68}};

// ITE Environment Controller
const UInt8 NSC_ADDRESS_REGISTER_OFFSET		= 0x00;
const UInt8 NSC_DATA_REGISTER_OFFSET		= 0x01;
const UInt8 NSC_BANK_SELECT_REGISTER		= 0x07;
const UInt8 NSC_CHIP_ID_REGISTER			= 0x20;
const UInt8 NSC_CHIP_REVISION_REGISTER		= 0x27;
const UInt8 NSC_LDN_PRESENT					= 0x30;


const UInt8 NSC_HARDWARE_MONITOR_LDN		= 0x0F;
const UInt8 NSC_MEM							= 0xF4;

enum PC8739xModel
{
	PC8739xx = 0xfc00
};

class PC8739x : public SuperIOMonitor
{
    OSDeclareDefaultStructors(PC8739x)
	
private:
	volatile UInt8*			mmioBase;
	IOMemoryMap *			mmio;
	UInt8					revision;
	
	UInt8					readByte(UInt8 bank, UInt8 reg);
	void					writeByte(UInt8 bank, UInt8 reg, UInt8 value);
	
	virtual bool			probePort();
    virtual bool			startPlugin();
	
	virtual long			readTemperature(unsigned long index);
	virtual long			readTachometer(unsigned long index);
	
	virtual const char *	getModelName();
	
public:

};