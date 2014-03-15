//
//  HWMSmcFanController.m
//  HWMonitor
//
//  Created by Kozlek on 03/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

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
    HWMSmcFanControlLevel *level = [NSEntityDescription insertNewObjectForEntityForName:@"SmcFanControlLevel" inManagedObjectContext:self.managedObjectContext];

    [level setInput:input];
    [level setOutput:output];

    if (self.levels.count) {
        [level setPrevious:self.levels.lastObject];
    }

    [level setController:self];

    return level;
}

-(void)inputValueChanged
{
    if (self.enabled.boolValue) {
        if (!_currentLevel) {
            for (HWMSmcFanControlLevel *level in self.levels) {
                if ([self.input.value isGreaterThan:level.input]) {
                    _currentLevel = level;
                }
            }

            if (!_currentLevel) {
                _currentLevel = self.levels.firstObject;
            }
        }
        else if (_currentLevel.previous && [self.input.value isLessThan:_currentLevel.previous.input]) {
            _currentLevel = _currentLevel.previous;
        }
        else if (_currentLevel.next && [self.input.value isGreaterThan:_currentLevel.next.input]) {
            _currentLevel = _currentLevel.next;
        }

        [self updateFanSpeed];
    }
}

-(void)updateFanSpeed
{
    [self updateManualControlKey];

    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        if (_currentLevel) {
            if (self.output.engine.isRunningOnMac) {
                // Write fan min key this will force SMC to set fan speed to our desired speed
                [SmcHelper privilegedWriteNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_MIN, ((HWMSmcFanSensor*)self.output).number.unsignedCharValue] value:_currentLevel.output];
            }
            else {
                // Write target speed key
                [SmcHelper privilegedWriteNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_TARGET, ((HWMSmcFanSensor*)self.output).number.unsignedCharValue] value:_currentLevel.output];
            }

            // Check fan speed every 5 minutes to be sure it's not run off
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(300 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^(void){
                // +/- 25 rpm
                if (ABS(self.output.value.integerValue - _currentLevel.output.integerValue) > 25) {
                    [self updateFanSpeed];
                }
            });
        }
    }];
}

-(void)updateManualControlKey
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
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
    }];
}

- (void)insertObject:(HWMSmcFanControlLevel *)value inLevelsAtIndex:(NSUInteger)idx
{
    [[self mutableOrderedSetValueForKey:@"levels"] insertObject:value atIndex:idx];
}

@end
