//
//  ACPIProbe.cpp
//  HWSensors
//
//  Created by Kozlek on 04/09/13.
//
//  The MIT License (MIT)
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


#include "ACPIProbe.h"
#include "ACPISensors.h"

#include "timer.h"

OSDefineMetaClassAndStructors(ACPIProbeProfile, OSObject)

ACPIProbeProfile * ACPIProbeProfile::withParameters(OSString *name, OSArray *methods, OSNumber *interval, OSNumber *timeout, OSNumber *verbose)
{
    if (!methods || !methods->getCount() || !interval || !interval->unsigned64BitValue()) {
        return NULL;
    }

    ACPIProbeProfile *profile = new ACPIProbeProfile;

    if (!profile || !profile->init(name, methods, interval, timeout, verbose)) {
        OSSafeReleaseNULL(profile);
    }

    return profile;
}

bool ACPIProbeProfile::init(OSString *aName, OSArray *aMethods, OSNumber *aInterval, OSNumber *aTimeout, OSNumber *aVerbose)
{
    if (!OSObject::init() || !aName || !aName->getLength() || !aInterval || !aInterval->unsigned64BitValue()) {
        return false;
    }

    snprintf(this->name, 32, "%s", aName->getCStringNoCopy());

    this->methods = OSArray::withArray(aMethods);

    this->interval = aInterval->unsigned32BitValue();
    this->timeout = aTimeout ? (double)aTimeout->unsigned64BitValue() / 1000.0 : 0;
    this->verbose = aVerbose && aVerbose->unsigned8BitValue() > 0 ? true : false;

    return true;
}

void ACPIProbeProfile::free()
{
    OSSafeReleaseNULL(this->methods);
    OSObject::free();
}

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(ACPIProbe, FakeSMCPlugin)

void ACPIProbe::addProfile(OSString *name, OSArray *methods, OSNumber *interval, OSNumber *timeout, OSNumber *verbose)
{
    if (ACPIProbeProfile *profile = ACPIProbeProfile::withParameters(name, methods, interval, timeout, verbose)) {
        profiles->setObject(name, profile);
        profileList->setObject(profile);
        ACPISensorsInfoLog("'%s' profile loaded", name->getCStringNoCopy());
    }
}

ACPIProbeProfile* ACPIProbe::getProfile(OSString *name)
{
    return (ACPIProbeProfile*)profiles->getObject(name);
}

ACPIProbeProfile* ACPIProbe::getProfile(const char *name)
{
    return (ACPIProbeProfile*)profiles->getObject(name);
}

ACPIProbeProfile* ACPIProbe::getProfile(unsigned int index)
{
    return (ACPIProbeProfile*)profileList->getObject(index);
}

unsigned int ACPIProbe::getProfileCount()
{
    return profileList->getCount();
}

void ACPIProbe::logValue(const char* method, OSObject *value)
{
    if (OSNumber *number = OSDynamicCast(OSNumber, value)) {
        ACPISensorsInfoLog("%s = %lld", method, number->unsigned64BitValue());
    }
    else if (OSString *string = OSDynamicCast(OSString, value)) {
        ACPISensorsInfoLog("%s = %s", method, string->getCStringNoCopy());
    }
    else if (OSData *data = OSDynamicCast(OSData, value)) {
        IOLog ("%s (%s): %s = 0x", getName(), acpiDevice->getName(), method);
        const UInt8 *buffer = (const UInt8*)data->getBytesNoCopy();
        for (unsigned int i = 0; i < data->getLength(); i++) {
            IOLog ("%02x", buffer[i]);
        }
        IOLog ("\n");
    }
    else if (OSArray *array = OSDynamicCast(OSArray, value)) {
        for (unsigned int i = 0; i < array->getCount(); i++) {
            char name[64];
            
            snprintf(name, 64, "%s[%d]", method, i);
            
            logValue(name, array->getObject(i));
        }
    }
    else {
        ACPISensorsInfoLog("call %s (data not shown)", method);
    }
}

ACPIProbeProfile* ACPIProbe::getActiveProfile()
{
    return activeProfile;
}

IOReturn ACPIProbe::setActiveProfile(const char *name)
{
    if (profiles && profiles->getCount()) {

        if (ACPIProbeProfile *profile = (ACPIProbeProfile *)profiles->getObject(name)) {
            activeProfile = profile;
            activeProfile->startedAt = 0;

            timerEventSource->cancelTimeout();
            timerEventSource->setTimeoutMS(100);

            ACPISensorsInfoLog("'%s' profile activated", name);
            return kIOReturnSuccess;
        }
        else {
            return kIOReturnBadArgument;
        }
    }

    return kIOReturnAborted;
}

IOReturn ACPIProbe::woorkloopTimerEvent(void)
{
    double time = ptimer_read_seconds();

    if (activeProfile->timeout > 0 && activeProfile->startedAt == 0) {
        activeProfile->startedAt = time;
    }

    if (activeProfile && (activeProfile->timeout == 0 || (time - activeProfile->startedAt < activeProfile->timeout))) {
        
        OSDictionary *values = OSDictionary::withCapacity(0);
        
        for (unsigned int i = 0; i < activeProfile->methods->getCount(); i++) {

            if (OSString *method = (OSString*)activeProfile->methods->getObject(i)) {
                
                OSObject *object = NULL;

                IOReturn result = acpiDevice->evaluateObject(method->getCStringNoCopy(), &object);
                
                if (kIOReturnSuccess == result && object) {

                    values->setObject(method->getCStringNoCopy(), object);
                    
                    if (activeProfile->verbose)
                        logValue(method->getCStringNoCopy(), object);
                }
                else {
                    ACPISensorsErrorLog("failed to evaluate method \"%s\", return %d", method->getCStringNoCopy(), result);
                }
            }
        }
        
        setProperty("Values", values);
        
        timerEventSource->setTimeoutMS(activeProfile->interval);
    }
    
    return kIOReturnSuccess;
}

bool ACPIProbe::start(IOService * provider)
{
    ACPISensorsDebugLog("starting...");
    
	if (!super::start(provider))
        return false;
    
	if (!(acpiDevice = OSDynamicCast(IOACPIPlatformDevice, provider))) {
        ACPISensorsFatalLog("ACPI device not ready");
        return false;
    }

    profiles = OSDictionary::withCapacity(0);
    profileList = OSArray::withCapacity(0);

    OSObject *object = NULL;


    // Try to load configuration provided by ACPI device

        if (kIOReturnSuccess == acpiDevice->evaluateObject("LIST", &object) && object) {
            if (OSArray *list = OSDynamicCast(OSArray, object)) {
                for (unsigned int i = 0; i < list->getCount(); i++) {
                    if (OSString *method = OSDynamicCast(OSString, list->getObject(i))) {
                        if (kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy(), &object) && object) {
                            if (OSArray *config = OSDynamicCast(OSArray, object)) {
                                if (config->getCount() > 4) {
                                    OSString *pName = OSDynamicCast(OSString, config->getObject(0));
                                    OSNumber *pInterval = OSDynamicCast(OSNumber, config->getObject(1));
                                    OSNumber *pTimeout = OSDynamicCast(OSNumber, config->getObject(2));
                                    OSNumber *pVerbose = OSDynamicCast(OSNumber, config->getObject(3));

                                    OSArray *pMethods = OSArray::withCapacity(config->getCount() - 4);

                                    for (unsigned int offset = 4; offset < config->getCount(); offset++) {
                                        if (OSString *methodName = OSDynamicCast(OSString, config->getObject(offset))) {
                                            pMethods->setObject(methodName);
                                        }
                                    }

                                    addProfile(pName, pMethods, pInterval, pTimeout, pVerbose);
                                }
                            }

                            OSSafeReleaseNULL(object);
                        }
                    }
                }

                OSSafeReleaseNULL(list);
            }

        }
        else {
            ACPISensorsErrorLog("profile definition table (LIST) not found");
        }

    // Try to load configuration from info.plist
    if (profiles->getCount() == 0) {
        if (OSDictionary *configuration = getConfigurationNode())
        {
            OSString *pName = OSDynamicCast(OSString, configuration->getObject("ProfileName"));
            OSNumber *pInterval = OSDynamicCast(OSNumber, configuration->getObject("PollingInterval"));
            OSNumber *pTimeout = OSDynamicCast(OSNumber, configuration->getObject("PollingTimeout"));
            OSBoolean *pVerboseBool = OSDynamicCast(OSBoolean, configuration->getObject("VerboseLog"));
            OSNumber *pVerbose = OSNumber::withNumber(pVerboseBool->getValue() ? 1 : 0, 8);
            OSArray *pMethods = OSDynamicCast(OSArray, configuration->getObject("MethodsToPoll"));

            addProfile(pName, pMethods, pInterval, pTimeout, pVerbose);
        }
    }

    if (this->profiles->getCount()) {

        // Parse active profile
        if (kIOReturnSuccess == acpiDevice->evaluateObject("ACTV", &object) && object) {
            if (OSString *method = OSDynamicCast(OSString, object)) {
                if (kIOReturnSuccess == acpiDevice->evaluateObject(method->getCStringNoCopy(), &object) && object) {
                    if (OSArray *config = OSDynamicCast(OSArray, object)) {
                        if (config->getCount() > 4) {
                            if (OSString *profile = OSDynamicCast(OSString, config->getObject(0))) {
                                if (!(activeProfile = (ACPIProbeProfile *)profiles->getObject(profile))) {
                                    activeProfile = (ACPIProbeProfile *)profileList->getObject(0);
                                }
                            }
                        }
                    }

                    OSSafeReleaseNULL(object);
                }

                OSSafeReleaseNULL(method);
            }
        }

        // woorkloop
        if (!(workloop = getWorkLoop())) {
            HWSensorsFatalLog("Failed to obtain workloop");
            return false;
        }
        
        if (!(timerEventSource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &ACPIProbe::woorkloopTimerEvent)))) {
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

    // two power states - off and on
	static const IOPMPowerState powerStates[2] = {
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, IOPMDeviceUsable, IOPMPowerOn, IOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0 }
    };

    // register interest in power state changes
	PMinit();
	provider->joinPMtree(this);
	registerPowerDriver(this, (IOPMPowerState *)powerStates, 2);
    
	registerService();
    
    ACPISensorsInfoLog("started");
    
	return true;
}

IOReturn ACPIProbe::setPowerState(unsigned long powerState, IOService *device)
{
	switch (powerState) {
        case 0: // Power Off
            timerEventSource->cancelTimeout();
            break;

        case 1: // Power On
            if (activeProfile) {
                setActiveProfile(activeProfile->name);
            }
            break;

        default:
            break;
    }

	return(IOPMAckImplied);
}

void ACPIProbe::stop(IOService *provider)
{
    PMstop();

    timerEventSource->cancelTimeout();
    workloop->removeEventSource(timerEventSource);
    
    super::stop(provider);
}

void ACPIProbe::free()
{
    OSSafeReleaseNULL(profiles);
    OSSafeReleaseNULL(profileList);
    super::free();
}
