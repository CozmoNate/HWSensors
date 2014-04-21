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

#import "FakeSMCDefinitions.h"
#import "HWMonitorDefinitions.h"

@implementation HWMSmcFanControlLevel

@dynamic input;
@dynamic output;
@dynamic controller;
@dynamic next;
@dynamic previous;

-(void)setInput:(NSNumber *)input
{
    [self willChangeValueForKey:@keypath(self, input)];
    [self setPrimitiveValue:input forKey:@keypath(self, input)];
    [self didChangeValueForKey:@keypath(self, input)];

    [NSObject cancelPreviousPerformRequestsWithTarget:self.controller selector:@selector(updateCurrentLevel) object:nil];

    [self.controller performSelector:@selector(updateCurrentLevel) withObject:nil afterDelay:0.5];
}

-(void)setOutput:(NSNumber *)output
{
    [self willChangeValueForKey:@keypath(self, output)];
    [self setPrimitiveValue:[NSNumber numberWithFloat:ROUND50(output.floatValue)] forKey:@keypath(self, output)];
    [self didChangeValueForKey:@keypath(self, output)];

    [self.controller calculateOutputRange];

    [NSObject cancelPreviousPerformRequestsWithTarget:self.controller selector:@selector(forceCurrentLevel) object:nil];

    [self.controller performSelector:@selector(forceCurrentLevel) withObject:nil afterDelay:0.5];
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
        [next willChangeValueForKey:@keypath(self, deletable)];
        [next didChangeValueForKey:@keypath(self, deletable)];
    }

    if (prev) {
        [prev willChangeValueForKey:@keypath(self, deletable)];
        [prev didChangeValueForKey:@keypath(self, deletable)];
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

    [self willChangeValueForKey:@keypath(self, deletable)];
    [self didChangeValueForKey:@keypath(self, deletable)];
    [level willChangeValueForKey:@keypath(self, deletable)];
    [level didChangeValueForKey:@keypath(self, deletable)];

    return level;
}

-(void)initialize
{
    
}

-(void)awakeFromFetch
{
    [super awakeFromFetch];
    [self initialize];
}

-(void)awakeFromInsert
{
    [super awakeFromInsert];
    [self initialize];
}

@end
