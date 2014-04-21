//
//  HWMSensorController.m
//  HWMonitor
//
//  Created by Kozlek on 01/03/14.
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

#import "HWMSensorController.h"
#import "HWMSensor.h"

#import <ReactiveCocoa/ReactiveCocoa.h>

@implementation HWMSensorController

@dynamic enabled;
@dynamic output;
@dynamic input;

@synthesize hasBeenDeletedSignal;

-(void)initialize
{
    [[RACObserve(self, enabled) takeUntil:self.hasBeenDeletedSignal] subscribeNext:^(id x) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            if (!self.enabled.boolValue) {
                self.input = nil;
            }
            [self updateCurrentLevel];
        }];
    }];

    [[RACObserve(self, input) takeUntil:self.hasBeenDeletedSignal] subscribeNext:^(id x) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self updateCurrentLevel];
        }];
    }];

    [[RACObserve(self, input.value) takeUntil:self.hasBeenDeletedSignal] subscribeNext:^(id x) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            if (self.enabled.boolValue) {
                [self updateCurrentLevel];
            }
        }];
    }];
}

-(void)updateCurrentLevel
{
    //
}

-(void)forceCurrentLevel
{
    //
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
