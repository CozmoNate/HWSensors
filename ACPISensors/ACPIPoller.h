//
//  ACPIPoller.h
//  HWSensors
//
//  Created by Kozlek on 04/09/13.
//
//

#ifndef __HWSensors__ACPIPoller__
#define __HWSensors__ACPIPoller__

#include "FakeSMCPlugin.h"

#include "IOKit/acpi/IOACPIPlatformDevice.h"
#include <IOKit/IOTimerEventSource.h>

class ACPIPoller : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(ACPIPoller)
    
private:
	IOACPIPlatformDevice    *acpiDevice;
    OSArray                 *methods;
    IOWorkLoop*             workloop;
    IOTimerEventSource*     timerEventSource;
    
    UInt64                  startTime;
    UInt64                  pollingTimeout;
    UInt32                  pollingInterval;
    
    bool                    loggingEnabled;
    
    IOReturn                woorkloopTimerEvent(void);
    void                    logValue(const char* method, OSObject *value);
protected:
    
    
public:
    virtual bool			start(IOService *provider);
    virtual void            stop(IOService* provider);
};


#endif /* defined(__HWSensors__ACPIPoller__) */
