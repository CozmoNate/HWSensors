/*
 *  ACPIMonitor.h
 *  HWSensors
 *
 *  Created by mozo on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include "IOKit/acpi/IOACPIPlatformDevice.h"

#include "FakeSMCPlugin.h"

class ACPIMonitor : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(ACPIMonitor)
    
private:
	IOACPIPlatformDevice *	acpiDevice;
    
	OSDictionary*			sensors;
	
	bool                    addSensor(const char* method, const char* key, const char* type, unsigned char size);
	bool                    addTachometer(const char* method, const char* caption);
    
public:
    virtual bool			init(OSDictionary *properties=0);
    virtual bool			start(IOService *provider);
	virtual void			stop(IOService *provider);
	virtual void			free(void);

	virtual IOReturn        callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};
