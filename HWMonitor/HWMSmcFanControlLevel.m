//
//  HWMSmcFanControlLevel.m
//  HWMonitor
//
//  Created by Kozlek on 03/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "HWMSmcFanControlLevel.h"
#import "HWMSmcFanController.h"


@implementation HWMSmcFanControlLevel

@dynamic input;
@dynamic output;
@dynamic controller;
@dynamic next;
@dynamic previous;

-(NSNumber *)minInput
{
    if (self.previous) {
        return self.previous.input;
    }

    return @0;
}

-(NSNumber *)maxInput
{
    if (self.next) {
        return self.next.input;
    }

    return @100;
}

-(NSNumber *)minOutput
{
    if (self.previous) {
        return self.previous.output;
    }

    return self.controller.min;
}

-(NSNumber *)maxOutput
{
    if (self.next) {
        return self.next.output;
    }

    return self.controller.max;
}

-(void)removeThisLevel
{
    if (self.next) {
        [self.next setPrevious:self.previous];
    }

    [self setController:nil];

    [self.managedObjectContext deleteObject:self];
}

-(HWMSmcFanControlLevel*)insertNextLevel
{
    HWMSmcFanControlLevel *level = [NSEntityDescription insertNewObjectForEntityForName:@"SmcFanControlLevel" inManagedObjectContext:self.managedObjectContext];

    [level setInput:self.input];
    [level setOutput:self.output];

    [self.controller insertObject:level inLevelsAtIndex:[self.controller.levels indexOfObject:self] + 1];

    if (self.next) {
        [level setNext:self.next];
    }

    [level setPrevious:self];

    return level;
}

@end
