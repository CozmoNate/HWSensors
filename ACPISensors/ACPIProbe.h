//
//  ACPIProbe.h
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

#ifndef __HWSensors__ACPIPoller__
#define __HWSensors__ACPIPoller__

#include "FakeSMCPlugin.h"

#include "IOKit/acpi/IOACPIPlatformDevice.h"
#include <IOKit/IOTimerEventSource.h>

class ACPIProbeProfile : public OSObject
{
    OSDeclareDefaultStructors(ACPIProbeProfile)

public:
    char            name[32];
    double          timeout;
    UInt32          interval;
    bool            verbose;

    OSArray         *methods;

    double          startedAt;

    static ACPIProbeProfile * withParameters(OSString *name,
                                             OSArray *methods,
                                             OSNumber *interval,
                                             OSNumber *timeout,
                                             OSNumber *verbose );

    virtual bool    init(OSString *name,
                         OSArray *aMethods,
                         OSNumber *aInterval,
                         OSNumber *aTimeout,
                         OSNumber *aVerbose);

    virtual void    free();
};

class ACPIProbe : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(ACPIProbe)
    
private:
	IOACPIPlatformDevice    *acpiDevice;
    IOWorkLoop              *workloop;
    IOTimerEventSource      *timerEventSource;
    
    OSDictionary            *profiles;
    OSArray                 *profileList;

    ACPIProbeProfile        *activeProfile;
    
    void                    addProfile(OSString *name,
                                       OSArray *methods,
                                       OSNumber *interval,
                                       OSNumber *timeout,
                                       OSNumber *verbose);

    void                    logValue(const char* method, OSObject *value);

    IOReturn                woorkloopTimerEvent(void);

protected:
    
    
public:
    virtual bool			start(IOService *provider);
    virtual IOReturn        setPowerState(unsigned long powerState, IOService *device);
    virtual void            stop(IOService* provider);
    virtual void            free();

    unsigned int            getProfileCount();
    ACPIProbeProfile*       getProfile(OSString *name);
    ACPIProbeProfile*       getProfile(const char *name);
    ACPIProbeProfile*       getProfile(unsigned int index);
    ACPIProbeProfile*       getActiveProfile();
    IOReturn                setActiveProfile(const char *name);
};


#endif /* defined(__HWSensors__ACPIPoller__) */
