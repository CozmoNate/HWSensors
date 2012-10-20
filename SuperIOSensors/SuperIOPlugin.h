/*
 *  SuperIOFamily.h
 *  HWSensors
 *
 *  Created by kozlek on 08/10/10.
 */

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

#ifndef _SuperIOPlugin_H
#define _SuperIOPlugin_H

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>

#include "FakeSMCPlugin.h"

// cool values (:
#define kSuperIOTemperatureSensor   1501 
#define kSuperIOVoltageSensor       1502

class FakeSMCPlugin;

class SuperIOSensor : public FakeSMCSensor {
    OSDeclareDefaultStructors(SuperIOSensor)
	
protected:
    float               reference;
    float               gain;
    float               offset;
	
public:
	static SuperIOSensor *withOwner(FakeSMCPlugin *aOwner, const char* aKey, const char* aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference = 0.0f, float aGain = 0.0f, float aOffset = 0.0f);
    
   	virtual bool		initWithOwner(FakeSMCPlugin *aOwner, const char* aKey, const char* aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference = 0.0f, float aGgain = 0.0f, float aOffset = 0.0f);
    
    float               getReference();
    float               getGain();
    float               getOffset();
};

class SuperIOPlugin : public FakeSMCPlugin {
	OSDeclareAbstractStructors(SuperIOPlugin)
	
private:

    
protected:    
	UInt16					address;
	UInt8					port;
	UInt32					model;
    
	const char              *modelName;
    const char              *vendorName;
    
    bool                    parseConfigurationNode(OSObject *object, OSString **name, float *reference, float *gain, float *offset);
    bool                    matchSensorToNodeName(OSString *node, const char *name, const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset);
    
    virtual SuperIOSensor   *addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset);
    
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
};

#endif