//
//  ACPIPoller.cpp
//  HWSensors
//
//  Created by Kozlek on 04/09/13.
//
//  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


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
    if (pollingTimeout > 0 && !startTime)
        startTime = ptimer_read_seconds();
    
    double time = ptimer_read_seconds();
    
    if (pollingTimeout == 0 || (time - startTime < pollingTimeout)) {
        
        OSDictionary *values = OSDictionary::withCapacity(0);
        
        for (unsigned int i = 0; i < methods->getCount(); i++) {
            if (OSString *method = (OSString*)methods->getObject(i)) {
                
                OSObject *object = NULL;
                IOReturn result = acpiDevice->evaluateObject(method->getCStringNoCopy(), &object);
                
                if (kIOReturnSuccess == result && object) {
                    values->setObject(method->getCStringNoCopy(), object);
                    
                    if (loggingEnabled)
                        logValue(method->getCStringNoCopy(), object);
                }
                else ACPISensorsErrorLog("failed to evaluate method \"%s\", return %d", method->getCStringNoCopy(), result);
            }
        }
        
        setProperty("Values", values);
        
        timerEventSource->setTimeoutMS((UInt32)(pollingInterval * 1000.0));
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
    OSNumber *interval = NULL;
    OSNumber *timeout = NULL;
    OSBoolean *logging = NULL;
    OSArray *list = NULL;
    
    
    // Try to load configuration from info.plist first
    if (OSDictionary *configuration = getConfigurationNode())
    {
        interval = OSDynamicCast(OSNumber, configuration->getObject("PollingInterval"));
        timeout = OSDynamicCast(OSNumber, configuration->getObject("PollingTimeout"));
        logging = OSDynamicCast(OSBoolean, configuration->getObject("LoggingEnabled"));
        list = OSDynamicCast(OSArray, configuration->getObject("Methods"));
    }
    // Try to load configuration provided by ACPI device
    else {
        OSObject *object = NULL;
        
        if (kIOReturnSuccess == acpiDevice->evaluateObject("INVL", &object) && object)
            interval = OSDynamicCast(OSNumber, object);
        
        if (kIOReturnSuccess == acpiDevice->evaluateObject("TOUT", &object) && object)
            timeout = OSDynamicCast(OSNumber, object);
        
        if (kIOReturnSuccess == acpiDevice->evaluateObject("LOGG", &object) && object) {
            if (OSNumber *number = OSDynamicCast(OSNumber, object)) {
                logging = OSBoolean::withBoolean(number->unsigned8BitValue() == 1);
            }
        }
        
        if (kIOReturnSuccess == acpiDevice->evaluateObject("LIST", &object) && object)
            list = OSDynamicCast(OSArray, object);
    }
    
    if (interval) {
        pollingInterval = (double)interval->unsigned64BitValue() / (double)1000.0;
        ACPISensorsInfoLog("polling interval %lld ms", interval->unsigned64BitValue());
        
        if (pollingInterval) {
            
            if (timeout) {
                pollingTimeout = (double)timeout->unsigned64BitValue() / 1000.0;
                ACPISensorsInfoLog("polling timeout %lld ms", timeout->unsigned64BitValue());
            }
            
            if (logging) {
                loggingEnabled = logging->isTrue();
                ACPISensorsInfoLog("logging %s", loggingEnabled ? "enabled" : "disabled");
            }
            
            if (list) {
                for (unsigned int i = 0; i < list->getCount(); i++) {
                    if (OSString *method = OSDynamicCast(OSString, list->getObject(i))) {
                        if (method->getLength() && kIOReturnSuccess == acpiDevice->validateObject(method->getCStringNoCopy())) {
                            methods->setObject(method);
                            ACPISensorsInfoLog("method \"%s\" registered", method->getCStringNoCopy());
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
        
        timerEventSource->setTimeoutMS(100);
        
        //ACPISensorsInfoLog("%d method%s registered", methods->getCount(), methods->getCount() > 1 ? "s" : "");
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