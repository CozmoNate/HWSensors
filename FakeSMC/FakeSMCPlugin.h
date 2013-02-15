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

#include "FakeSMCDefinitions.h"
#include <IOKit/IOService.h>

#define kFakeSMCTemperatureSensor   1
#define kFakeSMCVoltageSensor       2
#define kFakeSMCTachometerSensor    3
#define kFakeSMCFrequencySensor     4
#define kFakeSMCMultiplierSensor    5

struct FakeSMCSensorParams {
    const char *name;
    const char *key;
    const char *type;
    UInt8       size;
};

#define FakeSMCTemperatureCount 20

const struct FakeSMCSensorParams FakeSMCTemperature[FakeSMCTemperatureCount] =
{
    {"CPU", KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE},
    {"CPU Proximity", KEY_CPU_PROXIMITY_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE},
    {"System", KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE},
    {"PCH", KEY_PCH_DIE_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient", KEY_AMBIENT_TEMPERATURE, TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 1", "TA1P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 2", "TA2P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 3", "TA3P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 4", "TA4P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 5", "TA5P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 6", "TA6P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 7", "TA7P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 8", "TA8P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient 9", "TA9P", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient A", "TAAP", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient B", "TABP", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient C", "TACP", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient D", "TADP", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient E", "TAEP", TYPE_SP78, TYPE_SPXX_SIZE},
    {"Ambient F", "TAFP", TYPE_SP78, TYPE_SPXX_SIZE},
};

#define FakeSMCVolatgeCount 41

const struct FakeSMCSensorParams FakeSMCVolatge[FakeSMCVolatgeCount] =
{
    {"CPU", KEY_CPU_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE},
    {"Memory", KEY_MEMORY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE},
    {"Main 12V", KEY_MAIN_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE},
    {"PCIe 12V", KEY_PCIE_12V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Main 5V", KEY_MAIN_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Standby 5V", KEY_STANDBY_5V_VOLTAGE, TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Main 3V", KEY_MAIN_3V3_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE},
    {"Auxiliary 3V", KEY_AUXILIARY_3V3V_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE},
    {"CMOS Battery", KEY_POWERBATTERY_VOLTAGE, TYPE_FP2E, TYPE_FPXX_SIZE},
    {"CPU VRM Supply", "VS0C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 1", "VS1C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 2", "VS2C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 3", "VS3C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 4", "VS4C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 5", "VS5C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 6", "VS6C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 7", "VS7C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 8", "VS8C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply 9", "VS9C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply A", "VSAC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply B", "VSBC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply C", "VSCC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply D", "VSDC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply E", "VSEC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"CPU VRM Supply F", "VSFC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply", "Vp0C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 1", "Vp1C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 2", "Vp2C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 3", "Vp3C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 4", "Vp4C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 5", "Vp5C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 6", "Vp6C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 7", "Vp7C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 8", "Vp8C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply 9", "Vp9C", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply A", "VpAC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply B", "VpBC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply C", "VpCC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply D", "VpDC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply E", "VpEC", TYPE_FP4C, TYPE_FPXX_SIZE},
    {"Power Supply F", "VpFC", TYPE_FP4C, TYPE_FPXX_SIZE},
};

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
    
   	virtual bool		initWithOwner(FakeSMCPlugin *aOwner, const char* aKey, const char* aType, UInt8 aSize, UInt32 aGroup, UInt32 aIndex, float aReference, float aGain, float aOffset);
    
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

private:
    IOService               *headingProvider;
    IOService               *storageProvider;
    OSDictionary            *sensors;
    
protected:
    OSString                *getPlatformManufacturer();
    OSString                *getPlatformProduct();
    
    bool                    isKeyExists(const char *key);
    bool                    isKeyHandled(const char *key);
    
    SInt8                   takeVacantGPUIndex();
    bool                    releaseGPUIndex(UInt8 index);
    SInt8                   takeVacantFanIndex();
    bool                    releaseFanIndex(UInt8 index);
    
    bool                    setKeyValue(const char *key, const char *type, UInt8 size, const char *value);
    
    virtual FakeSMCSensor   *addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference = 0.0f, float gain = 0.0f, float offset = 0.0f);
    virtual bool            addSensor(FakeSMCSensor *sensor);
	virtual FakeSMCSensor   *addTachometer(UInt32 index, const char *name = 0, SInt8 *fanIndex = 0);
	virtual FakeSMCSensor   *getSensor(const char *key);
    
    OSDictionary            *getConfigurationNode(OSDictionary *root, OSString *name);
    OSDictionary            *getConfigurationNode(OSDictionary *root, const char *name);
    OSDictionary            *getConfigurationNode(OSString *model = NULL);
    
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