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

#define releaseTimerEventSource(timer) \
if (timer) { \
timer->cancelTimeout(); \
workloop->removeEventSource(timer); \
timer = NULL; \
}

#define kGPUSensorsAcceleratorWaitCycle     1000.0f
#define kGPUSensorsAcceleratorDelayTime     5000

IOReturn GPUSensors::probeEvent()
{
    HWSensorsDebugLog("Probe event...");

    if (probIsAcceleratorAlreadyLoaded()) {
        releaseTimerEventSource(timerEventSource);

        if (!(timerEventSource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &GPUSensors::delayedStartEvent)))) {
            HWSensorsFatalLog("failed to initialize delayed startup timer event source");
            return false;
        }

        if (kIOReturnSuccess != workloop->addEventSource(timerEventSource))
        {
            HWSensorsFatalLog("failed to add delayed startup timer event source into workloop");
            timerEventSource->release();
            return false;
        }

        // Wait a little before start
        timerEventSource->setTimeoutMS(kGPUSensorsAcceleratorDelayTime);
    }
    else if (probeCounter++ == (1000.0f / (kGPUSensorsAcceleratorWaitCycle * 45.0f))) {
        releaseTimerEventSource(timerEventSource);
        HWSensorsInfoLog("still waiting for IOAccelerator to start...");
        onTimeoutExceeded(pciDevice);
    }
    else {
//        if (probeCounter > 0 && !(probeCounter % ((int)(1000.0f / (float)kGPUSensorsAcceleratorWaitCycle) * 15)))
//            HWSensorsInfoLog("still waiting for accelerator to start...");

        timerEventSource->setTimeoutMS(kGPUSensorsAcceleratorWaitCycle);
    }
    
    return kIOReturnSuccess;
}

IOReturn GPUSensors::delayedStartEvent()
{
    HWSensorsDebugLog("delayed start...");

    onAcceleratorFound(pciDevice);
    releaseTimerEventSource(timerEventSource);
    return kIOReturnSuccess;
}

bool GPUSensors::shouldWaitForAccelerator()
{
    return false;
}

bool GPUSensors::probIsAcceleratorAlreadyLoaded()
{
    return true;
}

bool GPUSensors::onStartUp(IOService *provider)
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

    int arg_value = 1;

    if (PE_parse_boot_argn("-gpusensors-disable", &arg_value, sizeof(arg_value))) {
        return false;
    }
    
    if (!provider || !super::start(provider))
        return false;
    
    if (!(pciDevice = OSDynamicCast(IOPCIDevice, provider))) {
        HWSensorsFatalLog("no PCI device");
        return false;
    }

    if (!onStartUp(provider))
        return false;

    if (shouldWaitForAccelerator()) {
        if (!(workloop = getWorkLoop())) {
            HWSensorsFatalLog("failed to obtain workloop");
            return false;
        }
        
        if (!(timerEventSource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &GPUSensors::probeEvent)))) {
            HWSensorsFatalLog("failed to initialize startup check timer event source");
            return false;
        }
        
        if (kIOReturnSuccess != workloop->addEventSource(timerEventSource))
        {
            HWSensorsFatalLog("failed to add startup check timer event source into workloop");
            timerEventSource->release();
            return false;
        }
        
        timerEventSource->setTimeoutMS(100);
    }
    else return managedStart(provider);
    
    return true;
}

void GPUSensors::stop(IOService *provider)
{
    HWSensorsDebugLog("Stop...");
    
    releaseTimerEventSource(timerEventSource);
    
    super::stop(provider);
}
