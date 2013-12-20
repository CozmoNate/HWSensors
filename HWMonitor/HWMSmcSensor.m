//
//  HWMSmcSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

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

        return [SmcHelper decodeNumericValueFromBuffer:info.bytes length:info.dataSize type:info.dataType];
    }

    return @0;
}

@end
