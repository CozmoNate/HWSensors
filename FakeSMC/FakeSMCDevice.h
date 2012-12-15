/*
 *  FakeSMCDevice.h
 *  FakeSMC
 *
 *  Created by Vladimir on 20.08.09.
 *  Copyright 2009 netkas. All rights reserved.
 *
 */

#ifndef _FAKESMCDEVICE_h
#define _FAKESMCDEVICE_h

#include "FakeSMCKey.h"

#include <IOKit/acpi/IOACPIPlatformDevice.h>

#define APPLESMC_DATA_PORT				0x300

#define APPLESMC_CMD_PORT				0x304
#define APPLESMC_ERROR_CODE_PORT		0x31e
#define APPLESMC_NR_PORTS				32 /* 0x300-0x31f */
#define APPLESMC_MAX_DATA_LENGTH		32

#define APPLESMC_READ_CMD				0x10
#define APPLESMC_WRITE_CMD				0x11
#define APPLESMC_GET_KEY_BY_INDEX_CMD	0x12
#define APPLESMC_GET_KEY_TYPE_CMD		0x13

struct AppleSMCStatus {
	uint8_t cmd;
	uint8_t status;
	uint8_t	key[4];
	uint8_t read_pos;
	uint8_t data_len;
	uint8_t data_pos;
	uint8_t value[255];
	uint8_t charactic[4];
	uint8_t	status_1e;
	uint32_t key_index;
	uint8_t key_info[6];
};

class FakeSMCDevice : public IOACPIPlatformDevice
{
    OSDeclareDefaultStructors( FakeSMCDevice )
	
private:
	OSObject			*interrupt_target;
	IOInterruptAction	interrupt_handler;
	void				*interrupt_refcon;
	int					interrupt_source;
	
	struct
    ApleSMCStatus       *status;
	
	OSArray             *keys;
    OSDictionary        *types;
    OSDictionary        *exposedValues;
    
   	FakeSMCKey			*counterKey;
	
    bool				trace;
	bool				debug;
	
	virtual void		applesmc_io_cmd_writeb(void *opaque, uint32_t addr, uint32_t val);
	virtual void		applesmc_io_data_writeb(void *opaque, uint32_t addr, uint32_t val);
	virtual uint32_t	applesmc_io_data_readb(void *opaque, uint32_t addr1);
	virtual uint32_t	applesmc_io_cmd_readb(void *opaque, uint32_t addr1);
	virtual const char	*applesmc_get_key_by_index(uint32_t index, struct AppleSMCStatus *s);
	virtual void		applesmc_fill_data(struct AppleSMCStatus *s);
	virtual void		applesmc_fill_info(struct AppleSMCStatus *s);
	
public:
    virtual void		ioWrite32( UInt16 offset, UInt32 value, IOMemoryMap * map = 0 );
    virtual void		ioWrite16( UInt16 offset, UInt16 value, IOMemoryMap * map = 0 );
    virtual void		ioWrite8(  UInt16 offset, UInt8 value, IOMemoryMap * map = 0 );
    virtual UInt32		ioRead32( UInt16 offset, IOMemoryMap * map = 0 );
    virtual UInt16		ioRead16( UInt16 offset, IOMemoryMap * map = 0 );
    virtual UInt8		ioRead8(  UInt16 offset, IOMemoryMap * map = 0 );
	
	virtual IOReturn	registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refCon = 0);
    virtual IOReturn	unregisterInterrupt(int source);
    virtual IOReturn	getInterruptType(int source, int *interruptType);
    virtual IOReturn	enableInterrupt(int source);
    virtual IOReturn	disableInterrupt(int source);
	virtual IOReturn	causeInterrupt(int source);
	
	virtual bool		init(IOService *platform, OSDictionary *properties);
    virtual IOReturn	setProperties(OSObject * properties);
	
	//virtual FakeSMCKey	*addKey(const char *name, const char *type, unsigned char size);
	virtual FakeSMCKey	*addKeyWithValue(const char *name, const char *type, unsigned char size, const void *value);
	virtual FakeSMCKey	*addKeyWithHandler(const char *name, const char *type, unsigned char size, IOService *handler);
	virtual FakeSMCKey	*getKey(const char *name);
	virtual FakeSMCKey	*getKey(unsigned int index);
	virtual UInt32		getCount(void);
	
	virtual void		updateCounterKey(void);
    	
	//virtual void		setDebug(bool debug_val);
    
    virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};

#endif
