//
//  FakeSMCPlugin.cpp
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


#include "FakeSMCPlugin.h"

#include "FakeSMCDefinitions.h"
#include "FakeSMCKey.h"
#include "FakeSMCKeyStore.h"

#include <IOKit/IOLib.h>

#pragma mark
#pragma mark FakeSMCPlugin

const struct FakeSMCSensorDefinitionEntry gDefaultSensorDefinitions[] =
{
    {"Ambient",                 "TA0P", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0},
    {"CPU Die",                 "TC%XD", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"CPU Package",             "TC%XC", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0xA, 0x6},
    //{"CPU Core",                "TC%XC", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"CPU GFX",                 "TC%XG", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"CPU Heatsink",            "TC%XH", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"CPU Proximity",           "TC%XP", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"Northbridge Die",         "TN%XD", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"Northbridge Proximity",   "TN%XP", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"MCH Die",                 "TN%XC", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"MCH Heatsink",            "TN%XH", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"PCH Die",                 "TP%XD", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"PCH Proximity",           "TP%XP", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"Memory Module",           "TM%XS", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"Memory Proximity",        "TM%XP", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    {"LCD",                     "TL0P", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0},
    {"Airport",                 "TW0P", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0},
    {"Battery",                 "TB%XP", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
    {"Mainboard",               "Tm0P", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0},
    /*{"GPU Die",                 "TG%XD", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
     {"GPU Heatsink",            "TG%XH", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
     {"GPU Proximity",           "TG%Xp", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},
     {"GPU Memory",              "TG%XM", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 4},*/
    {"Thermal Zone",            "TZ%XC", SMC_TYPE_SP78, SMC_TYPE_SPXX_SIZE, kFakeSMCCategoryTemperature, 0, 0xF},
    
    // Multipliers
    {"CPU Core",                "MlC%X", SMC_TYPE_FP88, SMC_TYPE_FPXX_SIZE, kFakeSMCCategoryMultiplier, 0, 0xF},
    {"CPU Package",             "MlCP", SMC_TYPE_FP88, SMC_TYPE_FPXX_SIZE, kFakeSMCCategoryMultiplier, 0, 0},
    
    // Clocks
    {"CPU Core",                "CC%XC", SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 0xF},
    {"CPU Package",             "CCPC", SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 0},
    /*{"GPU Core",                "CG%XC", SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 4},
     {"GPU Memory",              "CG%XM", SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 4},
     {"GPU Shaders",             "CG%XS", SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 4},
     {"GPU ROPs",                "CG%XR", SMC_TYPE_UI32, SMC_TYPE_UI32_SIZE, kFakeSMCCategoryFrequency, 0, 4},*/
    
    // Voltages
    {"CPU Core",                "VC0C", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0},
    {"CPU GFX",                 "VC%XG", "fp2e", 2, kFakeSMCCategoryVoltage, 0, 0xF},
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

#include "OEMInfo.h"

static IORecursiveLock *gPluginLock = 0;

#define super FakeSMCKeyHandler
OSDefineMetaClassAndAbstractStructors(FakeSMCPlugin, FakeSMCKeyHandler)

#pragma mark -
#pragma mark FakeSMCPlugin::methods

/**
 *  Method to get platform (matherboard) manufacturer name provided by FakeSMCKeyStore
 *
 *  @return OSString containg platform manufacturer name
 */
OSString *FakeSMCPlugin::getPlatformManufacturer(void)
{
    return OSDynamicCast(OSString, keyStore->getProperty(kOEMInfoManufacturer));
}

/**
 *  Method to get platform (matherboard) product name provided by FakeSMCKeyStore
 *
 *  @return OSString containg platform product name
 */
OSString *FakeSMCPlugin::getPlatformProduct(void)
{
    return OSDynamicCast(OSString, keyStore->getProperty(kOEMInfoProduct));
}

/**
 *  Engage global FakeSMCPlugin access lock
 */
void FakeSMCPlugin::lockAccessForPlugins(void)
{
    IORecursiveLockLock(gPluginLock);
}

/**
 *  Disengage global FakeSMCPlugin access lock
 */
void FakeSMCPlugin::unlockAccessForPlugins(void)
{
    IORecursiveLockUnlock(gPluginLock);
}

/**
 *  Synchronized method to check key availability
 *
 *  @param key Key name
 *
 *  @return True if the key exists False otherwise
 */
bool FakeSMCPlugin::isKeyExists(const char *key)
{
    lockAccessForPlugins();

    bool keyExists = keyStore->getKey(key);

    unlockAccessForPlugins();

    return keyExists;
}

/**
 *  Synchronized method to check if specified key value is provided by handler service
 *
 *  @param key Key name
 *
 *  @return True if the key is handled False otherwise
 */
bool FakeSMCPlugin::isKeyHandled(const char *key)
{
    lockAccessForPlugins();

    bool keyHandled = false;

    if (FakeSMCKey *smcKey = keyStore->getKey(key))
        keyHandled = smcKey->getHandler();

    unlockAccessForPlugins();

    return keyHandled;
}

/**
 *  Synchronized method to set a value for specified key. If the key is handled by other service value will be set and didWriteSensorValue callback will be invoked for handler service
 *
 *  @param key   Key name for wich we want to set value
 *  @param type  Key type can be changed in realtime but not recomended
 *  @param size  Value size in bytes
 *  @param value Pointer to buffer containg new value for specified key. Value should be already encoded regarding key type
 *
 *  @return True on successfull action False otherwise
 */
bool FakeSMCPlugin::setKeyValue(const char *key, const char *type, UInt8 size, void *value)
{
    lockAccessForPlugins();

    bool added = keyStore->addKeyWithValue(key, type, size, value);

    unlockAccessForPlugins();

    return added;
}

/**
 *  Synchronized method to copy sensor value to specified buffer
 *
 *  @param key   Key name
 *  @param value Buffer to copy value to. Buffer should be already allocated with proper size to fit key value
 *
 *  @return True on success False otherwise
 */
bool FakeSMCPlugin::getKeyValue(const char *key, void *value)
{
    lockAccessForPlugins();

    FakeSMCKey *smcKey = keyStore->getKey(key);

    if (smcKey) {
        memcpy(value, smcKey->getValue(), smcKey->getSize());
    }

    unlockAccessForPlugins();

    return smcKey != NULL;
}

/**
 *  Synchronized method to add a new key to FakeSMCKeyStore and set its handler to the plugin
 *
 *  @param key       Key name
 *  @param type      Key type
 *  @param size      Key value size
 *  @param group     Key group
 *  @param index     Key index not related to key position in FakeSMCKeyStore key list
 *  @param reference Reference modifier value
 *  @param gain      Gain modifier value
 *  @param offset    Offset modifier value
 *
 *  @return new FakeSMCSensor object or NULL otherwise
 */
FakeSMCSensor *FakeSMCPlugin::addSensorForKey(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    lockAccessForPlugins();

    if (FakeSMCSensor *sensor = FakeSMCSensor::withOwner(this, key, type, size, group, index, reference, gain, offset)) {
        if (addSensor(sensor)) {
            unlockAccessForPlugins();
            return sensor;
        }
        else OSSafeReleaseNULL(sensor);
    }

    unlockAccessForPlugins();

	return NULL;
}

/**
 *  Synchronized method to add a new key to FakeSMCKeyStore and set its handler to the plugin
 *
 *  @param abbreviation Human readable key abbreviation used in config file
 *  @param category     Category (enum kFakeSMCCategory) key belongs to so abbreviation can be checked against the list from specified category only
 *  @param group        Key group
 *  @param index        Key index not related to key position in FakeSMCKeyStore key list
 *  @param reference Reference modifier value
 *  @param gain         Gain modifier value
 *  @param offset       Offset modifier value
 *
 *  @return new FakeSMCSensor object or NULL otherwise
 */
FakeSMCSensor *FakeSMCPlugin::addSensorUsingAbbreviation(const char *abbreviation, FakeSMCSensorCategory category, UInt32 group, UInt32 index, float reference, float gain, float offset)
{
    lockAccessForPlugins();

    FakeSMCSensor *sensor = NULL;

    if (abbreviation && strlen(abbreviation) >= 3) {
        for (int i = 0; gDefaultSensorDefinitions[i].name; i++) {

            FakeSMCSensorDefinitionEntry entry = gDefaultSensorDefinitions[i];

            if (entry.category == category && 0 == strcasecmp(entry.name, abbreviation)) {
                if (entry.count) {
                    for (int counter = 0; counter < entry.count; counter++) {

                        char key[5];
                        snprintf(key, 5, entry.key, entry.shift + counter);

                        if (!isKeyExists(key)) {
                            sensor = addSensorForKey(key, entry.type, entry.size, group, index, reference, gain, offset);
                            break;
                        }
                    }
                }
                else {
                    sensor = addSensorForKey(entry.key, entry.type, entry.size, group, index, reference, gain, offset);
                }
            }
        }
    }

    unlockAccessForPlugins();

    return sensor;
}

/**
 *  Synchronized method to add a new key from configuration node, set handler to this plugin, set also modifiers: reference, gain and offset read from config if present
 *
 *  @param node     OSDictionary configuration node contains key configuration
 *  @param category Category (enum kFakeSMCCategory) key belongs to so abbreviation can be checked against the list from specified category only
 *  @param group    Key group
 *  @param index    Key index not related to key position in FakeSMCKeyStore key list
 *
 *  @return new FakeSMCSensor object or NULL otherwise
 */
FakeSMCSensor *FakeSMCPlugin::addSensorFromNode(OSObject *node, FakeSMCSensorCategory category, UInt32 group, UInt32 index)
{
    lockAccessForPlugins();

    FakeSMCSensor *sensor = NULL;

    if (node) {

        float reference = 0, gain = 0, offset = 0;
        OSString *abbreviation = NULL;

        if (OSDictionary *dictionary = OSDynamicCast(OSDictionary, node)) {
            if ((abbreviation = OSDynamicCast(OSString, dictionary->getObject("name")))) {
                FakeSMCSensor::parseModifiers(dictionary, &reference, &gain, &offset);
            }
        }
        else abbreviation = OSDynamicCast(OSString, node);

        if (abbreviation)
            sensor = addSensorUsingAbbreviation(abbreviation->getCStringNoCopy(), category, group, index, reference, gain, offset);
    }

    unlockAccessForPlugins();
    
    return sensor;
}

/**
 *  Synchronized method to add previously allocated and configured sensor into FakeSMCKeyStore, set represented key handler to this plugin
 *
 *  @param sensor Sensor object
 *
 *  @return True on success False otherwise
 */
bool FakeSMCPlugin::addSensor(FakeSMCSensor *sensor)
{
    lockAccessForPlugins();

    bool added = keyStore->addKeyWithHandler(sensor->getKey(), sensor->getType(), sensor->getSize(), this);

    if (added) {
        sensors->setObject(sensor->getKey(), sensor);
    }

    unlockAccessForPlugins();

    return added;
}

/**
 *  Synchronized method to add tachometer sensor type into FakeSMCKeyStore. This will update fan counter key.
 *
 *  @param index    Key index not related to key position in FakeSMCKeyStore key list
 *  @param name     SMC fan name string up to 12 bytes long
 *  @param type     SMC fan type
 *  @param zone     SMC fan zone
 *  @param location SMC fan location
 *  @param fanIndex SMC fan index is returned on success
 *
 *  @return new FakeSMCSensor object or NULL otherwise
 */
FakeSMCSensor *FakeSMCPlugin::addTachometer(UInt32 index, const char *name, FanType type, UInt8 zone, FanLocationType location, UInt8 *fanIndex)
{
    lockAccessForPlugins();

    UInt8 vacantFanIndex = keyStore->takeVacantFanIndex();

    if (vacantFanIndex < UINT8_MAX) {
        char key[5];
        snprintf(key, 5, KEY_FORMAT_FAN_SPEED, vacantFanIndex);

        if (FakeSMCSensor *sensor = addSensorForKey(key, SMC_TYPE_FPE2, SMC_TYPE_FPXX_SIZE, kFakeSMCTachometerSensor, index)) {
            FanTypeDescStruct fds;

            bzero(&fds, sizeof(fds));

            fds.type = type;
            fds.ui8Zone = zone;
            fds.location = location;

            if (name)
                strlcpy(fds.strFunction, name, DIAG_FUNCTION_STR_LEN);
            else
                snprintf(fds.strFunction, DIAG_FUNCTION_STR_LEN, "MB Fan %X", index);

            snprintf(key, 5, KEY_FORMAT_FAN_ID, vacantFanIndex);

            if (!setKeyValue(key, SMC_TYPE_FDS, sizeof(fds), &fds))
                HWSensorsWarningLog("failed to add tachometer name for key %s", key);

            if (fanIndex) *fanIndex = vacantFanIndex;

            unlockAccessForPlugins();

            return sensor;
        }
        else HWSensorsErrorLog("failed to add tachometer sensor for key %s", key);
    }
    else HWSensorsErrorLog("failed to take vacant Fan index");

    unlockAccessForPlugins();

	return 0;
}

/**
 *  Method to get a key by name
 *
 *  @param key Key name
 *
 *  @return FakeSMCSensor object or NULL if the key not found
 */
FakeSMCSensor *FakeSMCPlugin::getSensor(const char* key)
{
	return OSDynamicCast(FakeSMCSensor, sensors->getObject(key));
}

/**
 *  Callback method invoked before key value will be read. Can be used by plugin to provide custom key value that can be calculated or obtained in the moment of key read action. Blocks key reading thread until returned
 *
 *  @param sensor   FakeSMCSensor object representing specific SMC key will be read
 *  @param outValue floating point key value will be exposed to SMC
 *
 *  @return if True is returned outValue will be used as a value for sensor handled SMC key. if False is returned SMC key value will not be changed
 */
bool FakeSMCPlugin::willReadSensorValue(FakeSMCSensor *sensor, float *outValue)
{
    return false;
}

/**
 *  Callback method invoked after the new value has been written to specific key. Can be used by plugin to handle key writes. Blocks key writing thread until returned
 *
 *  @param sensor FakeSMCSensor object representing specific SMC key has been written
 *  @param value  floating point value written to SMC key
 *
 *  @return not used
 */
bool FakeSMCPlugin::didWriteSensorValue(FakeSMCSensor *sensor, float value)
{
    return false;
}

/**
 *  Synchronized method that tries to take vacant GPU index available from internal list
 *
 *  @return Index taken (starting from 0) or UINT8_MAX on failure
 */
UInt8 FakeSMCPlugin::takeVacantGPUIndex(void)
{
    lockAccessForPlugins();

    UInt8 index = keyStore->takeVacantGPUIndex();

    unlockAccessForPlugins();

    return index;
}

/**
 *  Synchronized method that tries to take specific GPU index available from internal list
 *
 *  @param index GPU index to take
 *
 *  @return True if the index was taken successfully False otherwise
 */
bool FakeSMCPlugin::takeGPUIndex(UInt8 index)
{
    lockAccessForPlugins();

    bool taken = keyStore->takeGPUIndex(index);

    unlockAccessForPlugins();

    return taken;
}

/**
 *  Synchronized method that releases previousely taken GPU index by this or another plugin
 *
 *  @param index Index to release
 */
void FakeSMCPlugin::releaseGPUIndex(UInt8 index)
{
    lockAccessForPlugins();

    keyStore->releaseGPUIndex(index);

    unlockAccessForPlugins();
}

/**
 *  Synchronized method that takes vacant Fan index available from internal list. Will update fan counter key on success
 *
 *  @return Index taken (starting from 0) or UINT8_MAX on failure
 */
UInt8 FakeSMCPlugin::takeVacantFanIndex(void)
{
    lockAccessForPlugins();

    UInt8 index = keyStore->takeVacantFanIndex();

    unlockAccessForPlugins();

    return index;
}

/**
 *  Synchronized method that releases previousely taken Fan index by this or another plugin. Will update fan counter key on success
 *
 *  @param index Index to release
 */
void FakeSMCPlugin::releaseFanIndex(UInt8 index)
{
    lockAccessForPlugins();

    keyStore->releaseFanIndex(index);

    unlockAccessForPlugins();
}

/**
 *  Will try to decode floating point value first, will fallback to decode integer value
 *
 *  @param name     Key name
 *  @param outValue Decoded value will be returned os success
 *
 *  @return True on success False otherwise
 */
bool FakeSMCPlugin::decodeFloatValueForKey(const char *name, float *outValue)
{
    if (!outValue)
        return false;

    if (FakeSMCKey *key = keyStore->getKey(name)) {
        if (FakeSMCKey::decodeFloatValue(key->getType(), key->getSize(), key->getValue(), outValue)) {
            return true;
        }
        else {

            int intValue = 0;

            if (FakeSMCKey::decodeIntValue(key->getType(), key->getSize(), key->getValue(), &intValue)) {
                *outValue = (float)intValue;
                return true;
            }
        }
    }

    return false;
}

/**
 *  Will try to decode integer value first, will fallback to decode floating point value
 *
 *  @param name     Key name
 *  @param outValue Decoded value will be returned os success
 *
 *  @return True on success False otherwise
 */
bool FakeSMCPlugin::decodeIntValueForKey(const char *name, int *outValue)
{
    if (!outValue)
        return false;

    if (FakeSMCKey *key = keyStore->getKey(name)) {
        if (FakeSMCKey::decodeIntValue(key->getType(), key->getSize(), key->getValue(), outValue)) {
            return true;
        }
        else {

            float floatValue = 0;

            if (FakeSMCKey::decodeFloatValue(key->getType(), key->getSize(), key->getValue(), &floatValue)) {
                *outValue = (int)floatValue;
                return true;
            }
        }
    }
    
    return false;
}

OSDictionary *FakeSMCPlugin::getConfigurationNode(OSDictionary *root, OSString *name)
{
    OSDictionary *configuration = NULL;

    if (root && name) {
        HWSensorsDebugLog("looking up for configuration node: %s", name->getCStringNoCopy());

        if (!(configuration = OSDynamicCast(OSDictionary, root->getObject(name))))
            if (OSString *link = OSDynamicCast(OSString, root->getObject(name)))
                configuration = getConfigurationNode(root, link);
    }

    return configuration;
}

OSDictionary *FakeSMCPlugin::getConfigurationNode(OSDictionary *root, const char *name)
{
    OSDictionary *configuration = NULL;

    if (root && name) {
        OSString *nameNode = OSString::withCStringNoCopy(name);

        configuration = getConfigurationNode(root, nameNode);

        OSSafeReleaseNULL(nameNode);
    }

    return configuration;
}

/**
 *  Look for configuration node for specific model in plugin properties. Will search manufacturer and product configuration nodes. Will return "Default" configuration node from product node if the model was not found or "Default" node from manufacturer node if product was not found
 *
 *  @param model Model to load configuration for
 *
 *  @return OSDictionary object containg configuration node found or NULL otherwise
 */
OSDictionary *FakeSMCPlugin::getConfigurationNode(OSString *model)
{
    OSDictionary *configuration = NULL;

    if (OSDictionary *list = OSDynamicCast(OSDictionary, getProperty("Platform Profile")))
    {
        if (OSString *manufacturer = getPlatformManufacturer())
            if (OSDictionary *manufacturerNode = OSDynamicCast(OSDictionary, list->getObject(manufacturer)))
                if (!(configuration = getConfigurationNode(manufacturerNode, getPlatformProduct())))
                    if (!(configuration = getConfigurationNode(manufacturerNode, model)))
                        configuration = getConfigurationNode(manufacturerNode, "Default");

        if (!configuration && !(configuration = getConfigurationNode(list, model)))
            configuration = getConfigurationNode(list, "Default");
    }

    return configuration;
}

/**
 Get called when the system is going to sleep. Base implementation do nothing
 */
void FakeSMCPlugin::willPowerOff()
{
    // Override
}

/**
 Get called when the system is powered on. Base implementation do nothing
 */
void FakeSMCPlugin::hasPoweredOn()
{
    // Override
}

/**
 *  For internal use, do not override
 *
 */
bool FakeSMCPlugin::init(OSDictionary *properties)
{
    if (!gPluginLock)
        gPluginLock = IORecursiveLockAlloc();

    if (!super::init(properties))
        return false;

    sensors = OSDictionary::withCapacity(0);

    if (!sensors)
        return false;

	return true;
}

IOReturn FakeSMCPlugin::setPowerState(unsigned long powerState, IOService *device)
{
    switch (powerState) {
            // Power Off
        case 0:
            willPowerOff();
            break;
            
            // Power On
        case 1:
            hasPoweredOn();
            
            break;
            
        default:
            break;
    }
    
    return(IOPMAckImplied);
}
/**
 *  For internal use, do not override
 *
 */
bool FakeSMCPlugin::start(IOService *provider)
{
	if (!super::start(provider))
        return false;

    if (OSDictionary *matching = serviceMatching(kFakeSMCKeyStoreService)) {
        if (!(keyStore = OSDynamicCast(FakeSMCKeyStore, waitForMatchingService(matching, kFakeSMCDefaultWaitTimeout)))) {
            HWSensorsFatalLog("still waiting for FakeSMCKeyStore...");
            return false;
        }

        OSSafeReleaseNULL(matching);
    }

	return true;
}

inline UInt8 index_of_hex_char(char c)
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

/**
 *  For internal use, do not override
 *
 */
void FakeSMCPlugin::stop(IOService* provider)
{
    HWSensorsDebugLog("removing handler");
    
    lockAccessForPlugins();
    
    OSArray * keyStoreKeys = keyStore->getKeys();

    if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(keyStoreKeys)) {
        while (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, iterator->getNextObject())) {
            if (key->getHandler() == this) {
                if (FakeSMCSensor *sensor = getSensor(key->getKey())) {
                    if (sensor->getGroup() == kFakeSMCTachometerSensor) {
                        UInt8 index = index_of_hex_char(sensor->getKey()[1]);
                        HWSensorsDebugLog("releasing Fan%X", index);
                        keyStore->releaseFanIndex(index);
                    }
                }

                key->setHandler(NULL);
            }
        }
        OSSafeReleaseNULL(iterator);
    }
    
    OSSafeReleaseNULL(keyStoreKeys);

    HWSensorsDebugLog("releasing sensors collection");

    sensors->flushCollection();
    
	super::stop(provider);
    
    unlockAccessForPlugins();
}

/**
 *  For internal use, do not override
 *
 */
void FakeSMCPlugin::free()
{
    HWSensorsDebugLog("freenig sensors collection");
    OSSafeReleaseNULL(sensors);
	super::free();
}

/**
 *  For internal use, do not override
 *
 */
IOReturn FakeSMCPlugin::readKeyCallback(const char *key, const char *type, const UInt8 size, void *buffer)
{
    if (key && buffer) {
        if (FakeSMCSensor *sensor = getSensor(key)) {
            if (size == sensor->getSize()) {

                float value;

                if (willReadSensorValue(sensor, &value)) {
                    sensor->encodeNumericValue(value, buffer);
                }

                return kIOReturnSuccess;
            }
        }
        else return kIOReturnNotFound;
    }

    return kIOReturnBadArgument;
}

/**
 *  For internal use, do not override
 *
 */
IOReturn FakeSMCPlugin::writeKeyCallback(const char *key, const char *type, const UInt8 size, const void *buffer)
{       
    if (key && type && buffer) {
        if (FakeSMCSensor *sensor = getSensor(key)) {
            if (size == sensor->getSize()) {
                float floatValue = 0;
                int intValue = 0;

                if (FakeSMCKey::decodeFloatValue(type, size, buffer, &floatValue)) {
                    didWriteSensorValue(sensor, floatValue);
                }
                else if (FakeSMCKey::decodeIntValue(type, size, buffer, &intValue)) {
                    didWriteSensorValue(sensor, intValue);
                }
                
                return kIOReturnSuccess;
            }
        }
        else return kIOReturnNotFound;
    }
    
    return kIOReturnBadArgument;
}
