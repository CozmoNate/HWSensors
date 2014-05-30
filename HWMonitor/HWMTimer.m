//
//  HWMTimer.m
//  HWMonitor
//
//  Created by Kozlek on 26.04.14.
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

#import "HWMTimer.h"

@interface HWMTimer ()

@property (assign) dispatch_source_t dispatchSource;
@property (assign) BOOL active;

@end

@implementation HWMTimer

@synthesize dispatchSource = _dispatchSource;
@synthesize interval = _interval;

+(instancetype)timerWithInterval:(NSTimeInterval)interval queue:(dispatch_queue_t)queue block:(dispatch_block_t)block
{
    return [[HWMTimer alloc] initWithInterval:interval queue:queue block:block];
}

+(instancetype)timerWithInterval:(NSTimeInterval)interval block:(dispatch_block_t)block
{
    return [[HWMTimer alloc] initWithInterval:interval queue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0) block:block];
}

-(instancetype)initWithInterval:(NSTimeInterval)interval queue:(dispatch_queue_t)queue block:(dispatch_block_t)block
{
    self = [super init];

    if (self) {
        if (!(_dispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue))) {
            return nil;
        }

        dispatch_source_set_event_handler(_dispatchSource, block);

        [self setInterval:interval];
    }

    return self;
}

-(NSTimeInterval)interval
{
    return _interval;
}

-(void)setInterval:(NSTimeInterval)interval
{
    if (_interval != interval) {

        _interval = interval;

        if (_dispatchSource) {
            double nsecs = (double)(interval * NSEC_PER_SEC);
            // leeway 10% of interval
            dispatch_source_set_timer(_dispatchSource, dispatch_time(DISPATCH_TIME_NOW, nsecs), nsecs, nsecs / 10);
        }
    }
}

-(void)resume
{
    if (_dispatchSource && !self.active) {
        dispatch_resume(_dispatchSource);
        self.active = YES;
    }
}

-(void)suspend
{
    if (_dispatchSource && self.active) {
        dispatch_suspend(_dispatchSource);
        self.active = NO;
    }
}

-(void)dealloc
{
    if (_dispatchSource) {
        dispatch_release(_dispatchSource);
        _dispatchSource = 0;
    }
}

@end
