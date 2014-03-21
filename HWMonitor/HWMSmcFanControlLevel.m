//
//  HWMSmcFanControlLevel.m
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

#import "HWMSmcFanControlLevel.h"
#import "HWMSmcFanController.h"


@implementation HWMSmcFanControlLevel

@dynamic input;
@dynamic output;
@dynamic controller;
@dynamic next;
@dynamic previous;

-(void)setInput:(NSNumber *)input
{
    [self willChangeValueForKey:@"input"];
    [self setPrimitiveValue:input forKey:@"input"];
    [self didChangeValueForKey:@"input"];

    [self.controller inputValueChanged];
}

-(void)setOutput:(NSNumber *)output
{
    [self willChangeValueForKey:@"output"];
    [self setPrimitiveValue:output forKey:@"output"];
    [self didChangeValueForKey:@"output"];

    [self.controller inputValueChanged];
}

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

-(NSNumber *)deletable
{
    return self.previous || self.next ? @YES : @NO;
}

-(void)removeThisLevel
{
    HWMSmcFanControlLevel *next = self.next;
    HWMSmcFanControlLevel *prev = self.previous;

    [self setPrevious:nil];
    [self setNext:nil];
    [self setController:nil];

    if (next) {
        [next setPrevious:prev];
        [next willChangeValueForKey:@"deletable"];
        [next didChangeValueForKey:@"deletable"];
    }

    if (prev) {
        [prev willChangeValueForKey:@"deletable"];
        [prev didChangeValueForKey:@"deletable"];
    }

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

    [self willChangeValueForKey:@"deletable"];
    [self didChangeValueForKey:@"deletable"];
    [level willChangeValueForKey:@"deletable"];
    [level didChangeValueForKey:@"deletable"];

    return level;
}

@end
