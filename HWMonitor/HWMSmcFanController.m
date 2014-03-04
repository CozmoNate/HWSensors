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

#import "FakeSMCDefinitions.h"
#import "SmcHelper+HWMonitorHelper.h"
#import "smc.h"

@implementation HWMSmcFanController

@dynamic max;
@dynamic min;
@dynamic levels;

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

-(void)setEnabled:(NSNumber *)enabled
{
    [self willChangeValueForKey:@"enabled"];
    [self setPrimitiveValue:enabled forKey:@"enabled"];
    [self updateManualControlKey];
    [self didChangeValueForKey:@"enabled"];
}

-(void)awakeFromFetch
{
    [self updateManualControlKey];
}

- (void)insertObject:(HWMSmcFanControlLevel *)value inLevelsAtIndex:(NSUInteger)idx
{
    [[self mutableOrderedSetValueForKey:@"levels"] insertObject:value atIndex:idx];
}

@end
