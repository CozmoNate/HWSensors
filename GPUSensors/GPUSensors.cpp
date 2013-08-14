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

IOReturn GPUSensors::probeEvent()
{
    HWSensorsDebugLog("Probe event...");
    
    bool acceleratorFound = false;
    
    if (OSDictionary *matching = serviceMatching("IOAccelerator")) {
        if (OSIterator *iterator = getMatchingServices(matching)) {
            while (IOService *service = (IOService*)iterator->getNextObject()) {
                if (pciDevice == service->getParentEntry(gIOServicePlane)) {
                    acceleratorFound = true;
                    break;
                }
            }
            
            OSSafeRelease(iterator);
        }
        
        OSSafeRelease(matching);
    }
    
    if (acceleratorFound) {
        releaseTimerEventSource;
        onAcceleratorFound(pciDevice);
    }
    else if (probeCounter++ == 45) {
        releaseTimerEventSource;
        onTimeoutExceeded(pciDevice);
    }
    else {
        if (probeCounter > 0 && !(probeCounter % 15))
            HWSensorsInfoLog("still waiting for IOAccelerator to start...");
        
        timerEventSource->setTimeoutMS(300);
    }
    
    return kIOReturnSuccess;
}

void GPUSensors::onAcceleratorFound(IOService *provider)
{
    
}

void GPUSensors::onTimeoutExceeded(IOService *provider)
{
    
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
    
    timerEventSource->setTimeoutMS(500);
    
    return true;
}

void GPUSensors::stop(IOService *provider)
{
    HWSensorsDebugLog("Stop...");
    
    releaseTimerEventSource;
    
    super::stop(provider);
}
