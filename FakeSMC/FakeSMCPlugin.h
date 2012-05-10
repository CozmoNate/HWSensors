//
//  FakeSMCPlugin.h
//  HWSensors
//
//  Created by mozo on 11/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

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
    
	static FakeSMCSensor *withOwner(FakeSMCPlugin *aOwner, const char* aKey, const char* aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference = 0.0f, float aGain = 0.0f, float aOffset = 0.0f);
    
   	virtual bool		initWithOwner(FakeSMCPlugin *aOwner, const char* aKey, const char* aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference = 0.0f, float aGgain = 0.0f, float aOffset = 0.0f);
    
    const char          *getKey();
    const char          *getType();
    UInt8               getSize();
	UInt32              getGroup();
	UInt32              getIndex();
    float               getReference();
    float               getGain();
    float               getOffset();
    
    void                encodeValue(float value, void *outBuffer);
};

class FakeSMCPlugin : public IOService {
	OSDeclareAbstractStructors(FakeSMCPlugin)
	
protected:
	IOService               *fakeSMC;
    OSDictionary            *sensors;
    
    bool                    isKeyHandled(const char *key);
    
    virtual FakeSMCSensor   *addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference = 0.0f, float gain = 0.0f, float offset = 0.0f);
    virtual bool            addSensor(FakeSMCSensor *sensor);
	virtual FakeSMCSensor   *addTachometer(UInt32 index, const char *name = 0);
	virtual FakeSMCSensor   *getSensor(const char *key);
    
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