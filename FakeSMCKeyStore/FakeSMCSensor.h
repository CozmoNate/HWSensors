//
//  FakeSMCSensor.hpp
//  HWSensors
//
//  Created by Natan Zalkin on 05/04/2017.
//
//

#ifndef HWSensors_FakeSMCSensor_h
#define HWSensors_FakeSMCSensor_h

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

#define kFakeSMCTemperatureSensor   1
#define kFakeSMCVoltageSensor       2
#define kFakeSMCTachometerSensor    3
#define kFakeSMCFrequencySensor     4
#define kFakeSMCMultiplierSensor    5
#define kFakeSMCCurrentSensor       6
#define kFakeSMCPowerSensor         7

/**
 *  Sensor category used to look up for proper sensor definitions
 */
enum FakeSMCSensorCategory {
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
    const char              *name;
    const char              *key;
    const char              *type;
    UInt8                   size;
    FakeSMCSensorCategory   category;
    UInt8                   shift;
    UInt8                   count;
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

#endif /* HWSensors_FakeSMCSensor_h */
