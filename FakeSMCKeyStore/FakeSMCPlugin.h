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
#include "FakeSMCSensor.h"

class FakeSMCPlugin : public FakeSMCKeyHandler {
	OSDeclareAbstractStructors(FakeSMCPlugin)

private:
    virtual IOReturn        readKeyCallback(const char *key, const char *type, const UInt8 size, void *buffer);
    virtual IOReturn        writeKeyCallback(const char *key, const char *type, const UInt8 size, const void *buffer);

protected:
    OSDictionary            *sensors;
    FakeSMCKeyStore         *keyStore;
    
    OSString                *getPlatformManufacturer(void);
    OSString                *getPlatformProduct(void);
    
    void                    lockAccessForPlugins(void);
    void                    unlockAccessForPlugins(void);
    
    bool                    isKeyExists(const char *key);
    bool                    isKeyHandled(const char *key);
    
    UInt8                   takeVacantGPUIndex(void);
    bool                    takeGPUIndex(UInt8 index);
    void                    releaseGPUIndex(UInt8 index);
    UInt8                   takeVacantFanIndex(void);
    void                    releaseFanIndex(UInt8 index);

    bool                    setKeyValue(const char *key, const char *type, UInt8 size, void *value);
    bool                    getKeyValue(const char *key, void *value);
    
    virtual FakeSMCSensor   *addSensorForKey(const char *key, const char *type, UInt8 size, UInt32 group, UInt32 index, float reference = 0.0f, float gain = 0.0f, float offset = 0.0f);
    virtual FakeSMCSensor   *addSensorUsingAbbreviation(const char *abbreviation, FakeSMCSensorCategory category, UInt32 group, UInt32 index, float reference = 0.0f, float gain = 0.0f, float offset = 0.0f);
    virtual FakeSMCSensor   *addSensorFromNode(OSObject *node, FakeSMCSensorCategory category, UInt32 group, UInt32 index);
    	virtual FakeSMCSensor   *addTachometer(UInt32 index, const char *name = 0, FanType type = FAN_RPM, UInt8 zone = 0, FanLocationType location = CENTER_MID_FRONT, UInt8 *fanIndex = 0);
    virtual bool            addSensor(FakeSMCSensor *sensor);
	virtual FakeSMCSensor   *getSensor(const char *key);
    
    OSDictionary            *getConfigurationNode(OSDictionary *root, OSString *name);
    OSDictionary            *getConfigurationNode(OSDictionary *root, const char *name);
    OSDictionary            *getConfigurationNode(OSString *model = NULL);

    virtual bool            willReadSensorValue(FakeSMCSensor *sensor, float *outValue);
    virtual bool            didWriteSensorValue(FakeSMCSensor *sensor, float value);
    
    virtual void            willPowerOff();
    virtual void            hasPoweredOn();
    
public:
    bool                    decodeFloatValueForKey(const char *name, float *outValue);
    bool                    decodeIntValueForKey(const char *name, int *outValue);

	virtual bool			init(OSDictionary *properties=0);
    virtual bool			start(IOService *provider);
    virtual IOReturn        setPowerState(unsigned long powerState, IOService *device);
	virtual void			stop(IOService *provider);
	virtual void			free(void);
};

#endif
