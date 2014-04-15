//
//  GPUSensors.cpp
//  HWSensors
//
//  Created by Kozlek on 06/08/13.
//
//

#include "GPUSensors.h"

#include <IOKit/IOTimerEventSource.h>

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(GPUSensors, FakeSMCPlugin)

#define releaseTimerEventSource \
if (timerEventSource) { \
timerEventSource->cancelTimeout(); \
workloop->removeEventSource(timerEventSource); \
timerEventSource = NULL; \
}

#define kGPUSensorsAcceleratorWaitCycle     300

IOReturn GPUSensors::probeEvent()
{
    HWSensorsDebugLog("Probe event...");

    if (acceleratorLoadedCheck()) {
        releaseTimerEventSource;
        onAcceleratorFound(pciDevice);
    }
    else if (probeCounter++ == (1000.0f / (float)kGPUSensorsAcceleratorWaitCycle) * 45) {
        releaseTimerEventSource;
        onTimeoutExceeded(pciDevice);
    }
    else {
        if (probeCounter > 0 && !(probeCounter % ((int)(1000.0f / (float)kGPUSensorsAcceleratorWaitCycle) * 15)))
            HWSensorsInfoLog("still waiting for accelerator to start...");
        
        timerEventSource->setTimeoutMS(kGPUSensorsAcceleratorWaitCycle);
    }
    
    return kIOReturnSuccess;
}

bool GPUSensors::shouldWaitForAccelerator()
{
    return false;
}

bool GPUSensors::acceleratorLoadedCheck()
{
    return true;
}

bool GPUSensors::startupCheck(IOService *provider)
{
    return true;
}

bool GPUSensors::managedStart(IOService *provider)
{
    return true;
}

void GPUSensors::onAcceleratorFound(IOService *provider)
{
    managedStart(provider);
}

void GPUSensors::onTimeoutExceeded(IOService *provider)
{
    managedStart(provider);
}

bool GPUSensors::start(IOService *provider)
{
    HWSensorsDebugLog("Starting...");
    
    if (!provider || !super::start(provider))
        return false;
    
    if (!(pciDevice = OSDynamicCast(IOPCIDevice, provider))) {
        HWSensorsFatalLog("no PCI device");
        return false;
    }

    if (!startupCheck(provider))
        return false;

    if (shouldWaitForAccelerator()) {
        if (!(workloop = getWorkLoop())) {
            HWSensorsFatalLog("failed to obtain workloop");
            return false;
        }
        
        if (!(timerEventSource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &GPUSensors::probeEvent)))) {
            HWSensorsFatalLog("failed to initialize timer event source");
            return false;
        }
        
        if (kIOReturnSuccess != workloop->addEventSource(timerEventSource))
        {
            HWSensorsFatalLog("failed to add timer event source into workloop");
            timerEventSource->release();
            return false;
        }
        
        timerEventSource->setTimeoutMS(kGPUSensorsAcceleratorWaitCycle * 2);
    }
    else return managedStart(provider);
    
    return true;
}

void GPUSensors::stop(IOService *provider)
{
    HWSensorsDebugLog("Stop...");
    
    releaseTimerEventSource;
    
    super::stop(provider);
}
