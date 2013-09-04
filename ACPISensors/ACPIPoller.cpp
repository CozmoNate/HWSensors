//
//  ACPIPoller.cpp
//  HWSensors
//
//  Created by Kozlek on 04/09/13.
//
//

#include "ACPIPoller.h"
#include "ACPISensors.h"

#include "timer.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPIPoller, FakeSMCPlugin)

void ACPIPoller::logValue(const char* method, OSObject *value)
{
    if (OSNumber *number = OSDynamicCast(OSNumber, value)) {
        ACPISensorsInfoLog("%s = %lld", method, number->unsigned64BitValue());
    }
    else if (OSArray *array = OSDynamicCast(OSArray, value)) {
        for (unsigned int i = 0; i < array->getCount(); i++) {
            char name[64];
            
            snprintf(name, 64, "%s[%d]", method, i);
            
            logValue(name, array->getObject(i));
        }
    }
}

IOReturn ACPIPoller::woorkloopTimerEvent(void)
{
    if (pollingTimeout == 0 || (ptimer_read() - startTime < pollingTimeout)) {
        
        OSDictionary *values = OSDictionary::withCapacity(0);
        
        for (unsigned int i = 0; i < methods->getCount(); i++) {
            if (OSString *method = (OSString*)methods->getObject(i)) {
                
                OSObject *object = NULL;
                
                if (kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy(), &object) && object) {
                    values->setObject(method->getCStringNoCopy(), object);
                    
                    if (loggingEnabled)
                        logValue(method->getCStringNoCopy(), object);
                }
            }
        }
        
        setProperty("Values", values);
        
        timerEventSource->setTimeoutMS(pollingInterval);
    }
    
    return kIOReturnSuccess;
}

bool ACPIPoller::start(IOService * provider)
{
    ACPISensorsDebugLog("starting...");
    
	if (!super::start(provider))
        return false;
    
	if (!(acpiDevice = OSDynamicCast(IOACPIPlatformDevice, provider))) {
        ACPISensorsFatalLog("ACPI device not ready");
        return false;
    }
    
    methods = OSArray::withCapacity(0);
    
    // Try to load configuration from info.plist first
    if (OSDictionary *configuration = getConfigurationNode())
    {
        if (OSNumber *interval = OSDynamicCast(OSNumber, configuration->getObject("PollingInterval"))) {
            pollingInterval = interval->unsigned32BitValue();
        }
        
        if (OSNumber *timeout = OSDynamicCast(OSNumber, configuration->getObject("PollingTimeout"))) {
            pollingTimeout = (UInt64)timeout->unsigned32BitValue() * NSEC_PER_MSEC;
        }
        
        if (OSBoolean *logging = OSDynamicCast(OSBoolean, configuration->getObject("LoggingEnabled"))) {
            loggingEnabled = logging->isTrue();
        }
        
        if (pollingInterval) {
            if (OSArray *list = OSDynamicCast(OSArray, configuration->getObject("Methods"))) {
                for (unsigned int i = 0; i < list->getCount(); i++) {
                    if (OSString *method = OSDynamicCast(OSString, list->getObject(i))) {
                        if (method->getLength() && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                            methods->setObject(method);
                        }
                        else ACPISensorsErrorLog("unable to register method \"%s\"", method->getCStringNoCopy());
                    }
                }
            }
        }
    }
    
    if (methods->getCount()) {
        // woorkloop
        if (!(workloop = getWorkLoop())) {
            HWSensorsFatalLog("Failed to obtain workloop");
            return false;
        }
        
        if (!(timerEventSource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &ACPIPoller::woorkloopTimerEvent)))) {
            ACPISensorsFatalLog("failed to initialize timer event source");
            return false;
        }
        
        if (kIOReturnSuccess != workloop->addEventSource(timerEventSource))
        {
            ACPISensorsFatalLog("failed to add timer event source into workloop");
            return false;
        }
        
        if (pollingTimeout > 0)
            startTime = ptimer_read();
        
        timerEventSource->setTimeoutMS(pollingInterval);
        
        ACPISensorsInfoLog("%d method%s registered", methods->getCount(), methods->getCount() > 1 ? "s" : "");
    }
    
	registerService();
    
    ACPISensorsInfoLog("started");
    
	return true;
}

void ACPIPoller::stop(IOService *provider)
{
    timerEventSource->cancelTimeout();
    workloop->removeEventSource(timerEventSource);
    
    super::stop(provider);
}