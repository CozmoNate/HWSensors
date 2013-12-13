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

#include "FakeSMCDefinitions.h"
#include "FakeSMCKeyStore.h"
#include "FakeSMCKeyHandler.h"

#define kFakeSMCTemperatureSensor   1
#define kFakeSMCVoltageSensor       2
#define kFakeSMCTachometerSensor    3
#define kFakeSMCFrequencySensor     4
#define kFakeSMCMultiplierSensor    5
#define kFakeSMCCurrentSensor       6
#define kFakeSMCPowerSensor         7

enum kFakeSMCCategory {
    kFakeSMCCategoryNone = 0,
    kFakeSMCCategoryTemperature,
    kFakeSMCCategoryMultiplier,
    kFakeSMCCategoryFrequency,
    kFakeSMCCategoryVoltage,
    kFakeSMCCategoryCurrent,
    kFakeSMCCategoryPower,
    kFakeSMCCategoryFan,
};

struct FakeSMCSensorDefinitionEntry {
    const char      *name;
    const char      *key;
    const char      *type;
    UInt8           size;
    kFakeSMCCategory category;
    UInt8           shift;
    UInt8           count;
};

const struct FakeSMCSensorDefinitionEntry FakeSMCSensorDefinitions[] =
{
    {"Ambient",                 "TA0P", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0},
    {"CPU Die",                 "TC%XD", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"CPU Package",             "TC%XC", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0xA, 0x6},
    //{"CPU Core",                "TC%XC", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"CPU GFX",                 "TC%XG", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"CPU Heatsink",            "TC%XH", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"CPU Proximity",           "TC%XP", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"Northbridge Die",         "TN%XD", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"Northbridge Proximity",   "TN%XP", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"MCH Die",                 "TN%XC", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"MCH Heatsink",            "TN%XH", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"PCH Die",                 "TP%XD", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"PCH Proximity",           "TP%XP", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"Memory Module",           "TM%XS", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"Memory Proximity",        "TM%XP", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"LCD",                     "TL0P", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0},
    {"Airport",                 "TW0P", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0},
    {"Battery",                 "TB%XP", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"Mainboard",               "Tm0P", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0},
    /*{"GPU Die",                 "TG%XD", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"GPU Heatsink",            "TG%XH", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"GPU Proximity",           "TG%Xp", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"GPU Memory",              "TG%XM", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},*/
    {"Thermal Zone",            "TZ%XC", TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    
    // Multipliers
    {"CPU Core",                "MlC%X", TYPE_FP88, TYPE_FPXX_SIZE, kFakeSMCCategoryMultiplier, 0, 0xF},
    {"CPU Package",             "MlCP", TYPE_FP88, TYPE_FPXX_SIZE, kFakeSMCCategoryMultiplier, 0, 0},
    
    // Clocks
    {"CPU Core",                "CC%XC", TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 0xF},
    {"CPU Package",             "CCPC", TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 0},
    /*{"GPU Core",                "CG%XC", TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 4},
    {"GPU Memory",              "CG%XM", TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 4},
    {"GPU Shaders",             "CG%XS", TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 4},
    {"GPU ROPs",                "CG%XR", TYPE_UI32, TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 4},*/
    
    // Voltages
    {"CPU Core",                "VC0C", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"CPU VTT",                 "VV1R", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"PCH",                     "VN1R", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Memory",                  "VM0R", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"MCH",                     "VN0C", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Main 3V",                 "VV2S", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Main 5V",                 "VV1S", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Main 12V",                "VV9S", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Auxiliary 3V",            "VV7S", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Standby 3V",              "VV3S", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Standby 5V",              "VV8S", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"PCIe 12V",                "VeES", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"+12V Rail",               "VP0R", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"12V Vcc",                 "Vp0C", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Power Supply",            "Vp%XC", "fp4c", 2, kFakeSMCCategoryVoltage, 1, 0xE},
    {"Mainboard S0 Rail",       "VD0R", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Mainboard S5 Rail",       "VD5R", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"CMOS Battery",            "Vb0R", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"Battery",                 "VBAT", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"CPU VRM",                 "VS%XC", "fp4c", 2, kFakeSMCCategoryVoltage, 0, 0xF},
    
    /*{"GPU Core",                "VC%XG", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 4},*/
    
    // Currents
    {"CPU Core",                "IC0C", "sp78", 2, kFakeSMCCategoryCurrent, 0, 0},
    {"CPU VccIO",               "IC1C", "sp5a", 2, kFakeSMCCategoryCurrent, 0, 0},
    {"CPU VccSA",               "IC2C", "sp5a", 2, kFakeSMCCategoryCurrent, 0, 0},
    {"CPU DRAM",                "IC5R", "sp4b", 2, kFakeSMCCategoryCurrent, 0, 0},
    {"CPU PLL",                 "IC8R", "sp5a", 2, kFakeSMCCategoryCurrent, 0, 0},
    {"CPU",                     "IC%XC", "sp78", 2, kFakeSMCCategoryCurrent, 0, 0xF},
    {"CPU GFX",                 "IC0G", "sp5a", 2, kFakeSMCCategoryCurrent, 0, 0},
    {"Memory Bank",             "IM%XS", "sp5a", 2, kFakeSMCCategoryCurrent, 0, 0xF},
    {"MCH",                     "IN0C", "sp87", 2, kFakeSMCCategoryCurrent, 0, 0},
    
    /*{"GPU",                     "IG%XC", "sp78", 2, kFakeSMCCategoryCurrent, 0, 4},*/
    
//    [NSArray arrayWithObjects:@"IM0R",       @"Memory Rail", nil],
//    [NSArray arrayWithObjects:@"IW0E",       @"Airport Rail", nil],
//    [NSArray arrayWithObjects:@"IB0R",       @"Battery Rail", nil],
//    [NSArray arrayWithObjects:@"Ie:081S",    @"PCIe Slot %X", nil],
//    [NSArray arrayWithObjects:@"IM:A4AS",    @"PCIe Booster %X", nil],
//    [NSArray arrayWithObjects:@"ID0R",       @"Mainboard S0 Rail", nil],
//    [NSArray arrayWithObjects:@"ID5R",       @"Mainboard S5 Rail", nil],
    
    // Powers
    {"CPU Core",                "PC%XC", "sp96", 2, kFakeSMCCategoryPower, 0, 0x8},
    {"CPU",                     "PC%XC", "sp96", 2, kFakeSMCCategoryPower, 0xA, 6},
    {"CPU GFX",                 "PC%XG", "sp96", 2, kFakeSMCCategoryPower, 0, 0x4},
    {"CPU Package Cores",       "PCPC", "sp96", 2, kFakeSMCCategoryPower, 0, 0},
    {"CPU Package Graphics",    "PCPG", "sp96", 2, kFakeSMCCategoryPower, 0, 0},
    {"CPU Package Total",       "PCTR", "sp96", 2, kFakeSMCCategoryPower, 0, 0},
    {"CPU Package DRAM",        "PCPD", "sp96", 2, kFakeSMCCategoryPower, 0, 0},
//    [NSArray arrayWithObjects:@"PC1R",       @"CPU Rail", nil],
//    [NSArray arrayWithObjects:@"PC5R",       @"CPU 1.5V S0 Rail", nil],
//    [NSArray arrayWithObjects:@"PM0R",       @"Memory Rail", nil],
//    [NSArray arrayWithObjects:@"PM:A4AS",    @"Memory Bank %X", nil],
//    [NSArray arrayWithObjects:@"Pe:041S",    @"PCIe Slot %X", nil],
//    [NSArray arrayWithObjects:@"Pe:A4AS",    @"PCIe Booster %X", nil],
    
    /*{"GPU",                     "PG%XC", "sp96", 2, kFakeSMCCategoryPower, 0, 4},*/
    
//    [NSArray arrayWithObjects:@"PG0R",       @"GPU Rail", nil],
//    [NSArray arrayWithObjects:@"PG:132R",    @"GPU %X Rail", nil],
//    [NSArray arrayWithObjects:@"PD0R",       @"Mainboard S0 Rail", nil],
//    [NSArray arrayWithObjects:@"PD5R",       @"Mainboard S5 Rail", nil],
//    [NSArray arrayWithObjects:@"Pp0C",       @"Power Supply 12V", nil],
    {"System Total",            "PDTR", "sp96", 2, kFakeSMCCategoryPower, 0, 0},
//    [NSArray arrayWithObjects:@"PZ:041G",    @"Zone %X Average", nil],
    
    {NULL, NULL, NULL, 0, kFakeSMCCategoryNone, 0, 0}
};

UInt8   fakeSMCPluginGetIndexFromChar(char c);
bool    fakeSMCPluginEncodeNumericValue(float value, const char *type, const UInt8 size, void *outBuffer);
bool 	fakeSMCPluginIsValidIntegerType(const char *type);
bool    fakeSMCPluginIsValidFloatingType(const char *type);
bool    fakeSMCPluginDecodeNumericValue(const char *type, const UInt8 size, const void *data, float *outValue);

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
    static bool         parseModifiers(OSDictionary *node, float *reference, float *gain, float *offset);
    
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
    
    void                encodeNumericValue(float value, void *outBuffer);
};

class FakeSMCPlugin : public FakeSMCKeyHandler {
	OSDeclareAbstractStructors(FakeSMCPlugin)

protected:
    OSDictionary            *sensors;
    FakeSMCKeyStore         *keyStore;
    
    OSString                *getPlatformManufacturer(void);
    OSString                *getPlatformProduct(void);
    
    void                    enableExclusiveAccessMode(void);
    void                    disableExclusiveAccessMode(void);
    
    bool                    isKeyExists(const char *key);
    bool                    isKeyHandled(const char *key);
    
    SInt8                   takeVacantGPUIndex(void);
    bool                    takeGPUIndex(UInt8 index);
    void                    releaseGPUIndex(UInt8 index);
    SInt8                   takeVacantFanIndex(void);
    void                    releaseFanIndex(UInt8 index);

    bool                    setKeyValue(const char *key, const char *type, UInt8 size, void *value);
    
    virtual FakeSMCSensor   *addSensor(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference = 0.0f, float gain = 0.0f, float offset = 0.0f);
    virtual FakeSMCSensor   *addSensor(const char *abbriviation, kFakeSMCCategory category, UInt32 group, UInt32 index, float reference = 0.0f, float gain = 0.0f, float offset = 0.0f);
    virtual FakeSMCSensor   *addSensor(OSObject *node, kFakeSMCCategory category, UInt32 group, UInt32 index);
    virtual bool            addSensor(FakeSMCSensor *sensor);
	virtual FakeSMCSensor   *addTachometer(UInt32 index, const char *name = 0, FanType type = FAN_RPM, UInt8 zone = 0, FanLocationType location = CENTER_MID_FRONT, SInt8 *fanIndex = 0);
	virtual FakeSMCSensor   *getSensor(const char *key);
    
    OSDictionary            *getConfigurationNode(OSDictionary *root, OSString *name);
    OSDictionary            *getConfigurationNode(OSDictionary *root, const char *name);
    OSDictionary            *getConfigurationNode(OSString *model = NULL);

    virtual float           getSensorValue(FakeSMCSensor *sensor);
    virtual void            setSensorValue(FakeSMCSensor *sensor, float value);
    
public:    
	virtual bool			init(OSDictionary *properties=0);
    virtual bool			start(IOService *provider);
	virtual void			stop(IOService *provider);
	virtual void			free(void);

    virtual IOReturn        getValueCallback(const char *key, const char *type, const UInt8 size, void *buffer);
    virtual IOReturn        setValueCallback(const char *key, const char *type, const UInt8 size, const void *buffer);
};

#endif