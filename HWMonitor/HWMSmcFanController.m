//
//  HWMSmcFanController.m
//  HWMonitor
//
//  Created by Kozlek on 03/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
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

#import "HWMSmcFanController.h"
#import "HWMSmcFanControlLevel.h"
#import "HWMSmcFanSensor.h"
#import "HWMEngine.h"

#import "FakeSMCDefinitions.h"
#import "SmcHelper+HWMonitorHelper.h"
#import "smc.h"

@implementation HWMSmcFanController

@dynamic max;
@dynamic min;
@dynamic levels;

-(HWMSmcFanControlLevel*)addOutputLevel:(NSNumber*)output forInputLevel:(NSNumber*)input
{
    @synchronized (self) {
        HWMSmcFanControlLevel *level = [NSEntityDescription insertNewObjectForEntityForName:@"SmcFanControlLevel" inManagedObjectContext:self.managedObjectContext];

        [level setInput:input];
        [level setOutput:output];

        if (self.levels.count) {
            [level setPrevious:self.levels.lastObject];
        }

        [level setController:self];

        return level;
    }
}

-(void)setEnabled:(NSNumber *)enabled
{
    [super setEnabled:enabled];
    [self updateManualControlKey];
}

-(void)updateCurrentLevel
{
    if (self.enabled.boolValue) {

        //NSLog(@"Will update control level for '%@'", self.output.name);

        HWMSmcFanControlLevel *currentLevel = _currentLevel;

        if (!currentLevel) {
            currentLevel = self.levels.firstObject;
        }

        NSNumber *inputValue = self.input ? self.input.value : nil;

        if (inputValue) {
            while (currentLevel) {

                if (currentLevel.previous && [inputValue isLessThan:currentLevel.previous.input]) {
                    currentLevel = currentLevel.previous;
                }
                else if (currentLevel.next && [inputValue isGreaterThan:currentLevel.next.input]) {
                    currentLevel = currentLevel.next;
                }
                else {
                    break;
                }

            }
        }

        if (!_currentLevel || _currentLevel != currentLevel) {
            _currentLevel = currentLevel;
            [self forceCurrentLevel];
        }
    }
    else {
        _currentLevel = nil;
    }
}

-(void)forceCurrentLevel
{
    //NSLog(@"Will force control level for '%@'", self.output.name);

    if (!_currentLevel) {
        [self updateCurrentLevel];
    }
    else {
        [self updateFanSpeed];
    }
}

-(void)updateFanSpeed
{
    if (self.enabled.boolValue) {

        //NSLog(@"Will update fan speed for '%@'", self.output.name);

        [self updateManualControlKey];

        if (_currentLevel) {
            if (self.output.engine.isRunningOnMac) {
                // Write fan min key this will force SMC to set fan speed to our desired speed
                [SmcHelper privilegedWriteNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_MIN, ((HWMSmcFanSensor*)self.output).number.unsignedCharValue] value:_currentLevel.output];
            }
            else {
                // Write target speed key
                [SmcHelper privilegedWriteNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_TARGET, ((HWMSmcFanSensor*)self.output).number.unsignedCharValue] value:_currentLevel.output];
            }
        }

        // Check fan speed every 5 minutes to be sure it isn't run off
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(300 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^(void){
            // +/- 25 rpm
            if (!self.isDeleted && self.enabled.boolValue && _currentLevel && ABS(self.output.value.integerValue - _currentLevel.output.integerValue) > 25) {
                [self updateFanSpeed];
            }
        });
    }
}

-(void)updateManualControlKey
{
    SMCVal_t info;

    if (kIOReturnSuccess == SMCReadKey((io_connect_t)self.output.service.unsignedLongValue, KEY_FAN_MANUAL, &info)) {

        NSNumber *value;

        if ((value = [SmcHelper decodeNumericValueFromBuffer:&info.bytes length:info.dataSize type:info.dataType])) {

            UInt16 manual = value.unsignedShortValue;
            bool enabled = bit_get(manual, BIT(((HWMSmcFanSensor*)self.output).number.unsignedShortValue)) ? YES : NO;

            if (enabled != self.enabled.boolValue) {

                bit_write(self.enabled.boolValue, manual, BIT(((HWMSmcFanSensor*)self.output).number.unsignedShortValue));

                [SmcHelper privilegedWriteNumericKey:@KEY_FAN_MANUAL value:[NSNumber numberWithUnsignedShort:manual]];
            }
        }
    }
}

- (void)insertObject:(HWMSmcFanControlLevel *)value inLevelsAtIndex:(NSUInteger)idx
{
    [[self mutableOrderedSetValueForKey:@"levels"] insertObject:value atIndex:idx];
}


-(void)prepareForDeletion
{
    [super prepareForDeletion];

    _currentLevel = nil;
}

@end
