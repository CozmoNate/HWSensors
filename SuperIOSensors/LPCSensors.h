/*
 *  SuperIOFamily.h
 *  HWSensors
 *
 *  Created by kozlek on 08/10/10.
 */

//  The MIT License (MIT)
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

#ifndef _LPCSensors_H
#define _LPCSensors_H

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
//#include <IOKit/IOTimerEventSource.h>

#include "FakeSMCPlugin.h"

class LPCSensors : public FakeSMCPlugin {
	OSDeclareAbstractStructors(LPCSensors)
	
private:

    
protected:    
	UInt16					address;
	UInt8					port;
	UInt32					model;
    
	const char              *modelName;
    const char              *vendorName;
    
    SInt8                   gpuIndex;
    
    bool                    checkConfigurationNode(OSObject *node, const char *name);
    bool                    addSensorFromConfigurationNode(OSObject *node, const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index);
    
    virtual bool            addTemperatureSensors(OSDictionary *configuration);
    virtual bool            addVoltageSensors(OSDictionary *configuration);
    virtual bool            addTachometerSensors(OSDictionary *configuration);
    
    virtual UInt8           temperatureSensorsLimit();
    virtual UInt8           voltageSensorsLimit();
    virtual UInt8           tachometerSensorsLimit();
    
    virtual float			readTemperature(UInt32 index);
	virtual float			readVoltage(UInt32 index);
	virtual float			readTachometer(UInt32 index);
    
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
    virtual bool            initialize();
		
public:
	virtual bool			init(OSDictionary *properties=0);
	virtual IOService*		probe(IOService *provider, SInt32 *score);
    virtual bool			start(IOService *provider);
    virtual void            stop(IOService* provider);
};

#endif