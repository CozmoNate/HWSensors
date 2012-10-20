//
//  FakeSMCPlugin.h
//  HWSensors
//
//  Created by kozlek on 11/02/12.
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

#ifndef HWSensors_FakeSMCFamily_h
#define HWSensors_FakeSMCFamily_h

#include <IOKit/IOService.h>

#define kFakeSMCTemperatureSensor   1
#define kFakeSMCTachometerSensor    2
#define kFakeSMCVoltageSensor       3
#define kFakeSMCFrequencySensor     4
#define kFakeSMCMultiplierSensor    5

class FakeSMCPlugin;

class FakeSMCSensor : public OSObject {
    OSDeclareDefaultStructors(FakeSMCSensor)
	
protected:
	FakeSMCPlugin       *owner;
    char                key[5];
	char                type[5];
    UInt8               size;
	UInt32              group;
	UInt32              index;
    float               reference;
    float               gain;
    float               offset;
	
public:
    
	static FakeSMCSensor *withOwner(FakeSMCPlugin *aOwner, const char* aKey, const char* aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex);
    
   	virtual bool		initWithOwner(FakeSMCPlugin *aOwner, const char* aKey, const char* aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex);
    
    const char          *getKey();
    const char          *getType();
    UInt8               getSize();
	UInt32              getGroup();
	UInt32              getIndex();
    
    void                encodeValue(float value, void *outBuffer);
};

class FakeSMCPlugin : public IOService {
	OSDeclareAbstractStructors(FakeSMCPlugin)
	
protected:
	IOService               *fakeSMC;
    OSDictionary            *sensors;
    
    bool                    isKeyHandled(const char *key);
    
    virtual FakeSMCSensor   *addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index);
    virtual bool            addSensor(FakeSMCSensor *sensor);
	virtual FakeSMCSensor   *addTachometer(UInt32 index, const char *name = 0, UInt8 *fanIndex = 0);
	virtual FakeSMCSensor   *getSensor(const char *key);
    
    OSDictionary            *getConfigurationNode(OSDictionary *root, OSString *name);
    OSDictionary            *getConfigurationNode(OSDictionary *root, const char *name);
    OSDictionary            *getConfigurationNode(OSString *manufacturer, OSString *product, OSString *model);
    
    virtual SInt8           getVacantGPUIndex();
    virtual float           getSensorValue(FakeSMCSensor *sensor);
    
public:    
	virtual bool			init(OSDictionary *properties=0);
	virtual IOService       *probe(IOService *provider, SInt32 *score);
    virtual bool			start(IOService *provider);
	virtual void			stop(IOService *provider);
	virtual void			free(void);
	
	virtual IOReturn		callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};

#endif