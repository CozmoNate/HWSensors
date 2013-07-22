//
//  TZSensors.cpp
//  HWSensors
//
//  Created by Kozlek on 22/07/13.
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

#include "TZSensors.h"

#define super FakeSMCPlugin
OSDefineMetaClassAndStructors(TZSensors, FakeSMCPlugin)

float TZSensors::getSensorValue(FakeSMCSensor *sensor)
{
    UInt32 value = 0;
    
    switch(sensor->getGroup()) {
            
        case kFakeSMCTemperatureSensor:
            if (kIOReturnSuccess == acpiDevice->evaluateInteger("_TMP", &value)) {
                return (float)(value - 0xAAC) / (float)(0xA);
            }
            break;
            
        default:
            break;
    }
    
    return 0;
}

bool TZSensors::start(IOService * provider)
{
	if (!super::start(provider))
        return false;
    
	acpiDevice = (IOACPIPlatformDevice *)provider;
	
	if (!acpiDevice) {
        HWSensorsFatalLog("ACPI device not ready");
        return false;
    }
    
    OSObject *object = NULL;

    // Tachometers
    if(kIOReturnSuccess == acpiDevice->evaluateObject("_TMP", &object) && object) {
        for (UInt8 i = 0; i < 0xf; i++) {
            char key[5];
            
            snprintf(key, 5, KEY_FORMAT_THERMALZONE_TEMPERATURE, i);
            
            if (addSensor(key, TYPE_SP78, TYPE_SPXX_SIZE, kFakeSMCTemperatureSensor, 0)) {
                break;
            }
        }
    }
    
    registerService();
    
    HWSensorsInfoLog("started on %s", acpiDevice->getName());
    
    return true;
}