//
//  GPUSensors.h
//  HWSensors
//
//  Created by Kozlek on 06/08/13.
//
//

#ifndef __HWSensors__GPUSensors__
#define __HWSensors__GPUSensors__

#include "FakeSMCPlugin.h"
#include <IOKit/pci/IOPCIDevice.h>

class GPUSensors : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(GPUSensors)
	
private:
    IOWorkLoop*             workloop;
    IOTimerEventSource*     timerEventSource;
    int                     probeCounter;
    
    IOReturn                probeEvent();
    
protected:
    IOPCIDevice*            pciDevice;
    
    
    virtual bool            shouldWaitForAccelerator();
    virtual bool            acceleratorLoadedCheck();
    virtual bool            managedStart(IOService *provider);
    virtual void            onAcceleratorFound(IOService *provider);
    virtual void            onTimeoutExceeded(IOService *provider);
    
public:
    virtual bool            start(IOService *provider);
    virtual void            stop(IOService *provider);
};

#endif /* defined(__HWSensors__GPUSensors__) */
