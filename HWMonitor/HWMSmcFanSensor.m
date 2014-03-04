//
//  HWMSmcFanSensor.m
//  HWMonitor
//
//  Created by Kozlek on 22/11/13.
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

#import "HWMSmcFanSensor.h"
#import "smc.h"
#import "SmcHelper+HWMonitorHelper.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"

#import "FakeSMCDefinitions.h"

@implementation HWMSmcFanSensor

@dynamic descriptor;
@dynamic number;
@dynamic controller;

//-(void)updateManualControlKey
//{
//    SMCVal_t info;
//
//    if (kIOReturnSuccess == SMCReadKey((io_connect_t)self.service.unsignedLongValue, KEY_FAN_MANUAL, &info)) {
//
//        NSNumber *value;
//
//        if ((value = [SmcHelper decodeNumericValueFromBuffer:&info.bytes length:info.dataSize type:info.dataType])) {
//
//            UInt16 manual = value.unsignedShortValue;
//            bool enabled = bit_get(manual, BIT(self.number.unsignedShortValue)) ? YES : NO;
//
//            if (enabled != self.controlled.boolValue) {
//
//                bit_write(self.controlled.boolValue, manual, BIT(self.number.unsignedShortValue));
//
//                [SmcHelper privilegedWriteNumericKey:@KEY_FAN_MANUAL value:[NSNumber numberWithUnsignedShort:manual]];
//            }
//        }
//    }
//}
//
//-(void)setControlled:(NSNumber *)controlled
//{
//    if (controlled) {
//
//        [self updateManualControlKey];
//
//        // Set back fan min then disabling manual control on Macs
//        if (self.engine.isRunningOnMac && !controlled.boolValue && (!self.controlled || self.controlled.boolValue)) {
//            [SmcHelper privilegedWriteNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_MIN, self.number.unsignedCharValue] value:self.min];
//        }
//    }
//
//    [self willChangeValueForKey:@"controlled"];
//    [self setPrimitiveValue:controlled forKey:@"controlled"];
//    [self didChangeValueForKey:@"controlled"];
//
//    // If control enabled, set stored fan speed back
//    if (self.controlled && self.controlled.boolValue) {
//        [self setSpeed:self.speed];
//    }
//}
//
//-(void)setSpeed:(NSNumber *)speed
//{
//    if (self.controlled && self.controlled.boolValue && self.max && self.min && self.number && speed) {
//
//        [self updateManualControlKey];
//        
//        if (self.engine.isRunningOnMac) {
//            // Write into fan min key this will force SMC to set fan speed to our desired speed
//            [SmcHelper privilegedWriteNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_MIN, self.number.unsignedCharValue] value:speed];
//        }
//        else {
//            // Write target speed key
//            [SmcHelper privilegedWriteNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_TARGET, self.number.unsignedCharValue] value:speed];
//        }
//    }
//    
//    [self willChangeValueForKey:@"speed"];
//    [self setPrimitiveValue:speed forKey:@"speed"];
//    [self didChangeValueForKey:@"speed"];
//}

@end
