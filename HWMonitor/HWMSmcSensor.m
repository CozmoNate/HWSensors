//
//  HWMSmcSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMSensorsGroup.h"
#import "HWMSmcSensor.h"

#import "FakeSMCDefinitions.h"
#import "SmcHelper.h"
#import "smc.h"

@implementation HWMSmcSensor

-(NSUInteger)internalUpdateAlarmLevel
{
    float floatValue = self.value.floatValue;

    switch (self.selector.unsignedIntegerValue) {
        case kHWMGroupTemperature:
            return  floatValue >= 100 ? kHWMSensorLevelExceeded :
                    floatValue >= 85 ? kHWMSensorLevelHigh :
                    floatValue >= 70 ? kHWMSensorLevelModerate :
                    kHWMSensorLevelNormal;

        case kHWMGroupPWM:
            return  floatValue >= 70 ? kHWMSensorLevelHigh :
                    floatValue >= 50 ? kHWMSensorLevelModerate :
                    kHWMSensorLevelNormal;

        case kHWMGroupTachometer:
            return floatValue >= 2500 ? kHWMSensorLevelHigh :
                   floatValue >= 1500 ? kHWMSensorLevelModerate :
                   kHWMSensorLevelNormal;

        default:
            break;
    }

    return _alarmLevel;
}

-(NSNumber *)internalUpdateValue
{
    SMCVal_t info;

    if (kIOReturnSuccess == SMCReadKey((io_connect_t)self.service.unsignedLongValue, self.name.UTF8String, &info)) {

        return [NSNumber numberWithFloat:[SmcHelper decodeNumericValueFromBuffer:info.bytes length:info.dataSize type:[self.type cStringUsingEncoding:NSASCIIStringEncoding]]];
    }

    return @0;
}

@end
