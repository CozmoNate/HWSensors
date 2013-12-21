/*
 *  ACPISensors.h
 *  HWSensors
 *
 *  Created by kozlek on 12/11/10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#ifndef __HWSensors__ACPISensors__
#define __HWSensors__ACPISensors__

#include "FakeSMCPlugin.h"

#include "IOKit/acpi/IOACPIPlatformDevice.h"

#define kACPISensorsDebug   0

#define ACPISensorsDebugLog(string, args...)	do { if (kACPISensorsDebug) { IOLog ("%s (%s): [Debug] " string "\n",getName(), acpiDevice->getName() , ## args); } } while(0)
#define ACPISensorsWarningLog(string, args...) do { IOLog ("%s (%s): [Warning] " string "\n",getName(), acpiDevice->getName(), ## args); } while(0)
#define ACPISensorsErrorLog(string, args...) do { IOLog ("%s (%s): [Error] " string "\n",getName(), acpiDevice->getName() , ## args); } while(0)
#define ACPISensorsFatalLog(string, args...) do { IOLog ("%s (%s): [Fatal] " string "\n",getName(), acpiDevice->getName() , ## args); } while(0)
#define ACPISensorsInfoLog(string, args...)	do { IOLog ("%s (%s): " string "\n",getName(), acpiDevice->getName() , ## args); } while(0)

class ACPISensors : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(ACPISensors)
    
private:
	IOACPIPlatformDevice    *acpiDevice;
    OSArray                 *methods;
    bool                    useKelvins;
    
    void                    addSensorsFromDictionary(OSDictionary *dictionary, kFakeSMCCategory category);
    void                    addSensorsFromArray(OSArray *array, kFakeSMCCategory category);
    
protected:
    virtual bool           willReadSensorValue(FakeSMCSensor *sensor, float *outValue);
    
public:
    virtual bool			start(IOService *provider);
};

#endif /* defined(__HWSensors__ACPISensors__) */
