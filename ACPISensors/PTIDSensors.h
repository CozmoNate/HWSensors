//
//  INT340E.h
//  HWSensors
//
//  Created by kozlek on 24.08.12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
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

#ifndef __HWSensors__INT340E__
#define __HWSensors__INT340E__

#include <IOKit/IOService.h>
#include "IOKit/acpi/IOACPIPlatformDevice.h"

#include "FakeSMCPlugin.h"

#include <kern/clock.h>

class INT340ESensors : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(INT340ESensors)
    
private:
	IOACPIPlatformDevice    *acpiDevice;
    UInt64                  version;
    
    OSArray                 *temperatures;
    OSArray                 *tachometers;
    
    mach_timespec_t         temperatureNextUpdate;
    mach_timespec_t         tachometerNextUpdate;
    
    bool                    updateTemperatures();
    bool                    updateTachometers();
    
    float                   readTemperature(UInt32 index);
    float                   readTachometer(UInt32 index);
    
    void                    parseTemperatureName(OSString *name, UInt32 index);
    void                    parseTachometerName(OSString *name, UInt32 index);
    
protected:
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
public:
    virtual bool			start(IOService *provider);
};

#endif /* defined(__HWSensors__INT340E__) */
