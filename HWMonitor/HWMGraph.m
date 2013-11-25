//
//  HWMGraph.m
//  HWMonitor
//
//  Created by Kozlek on 24/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMGraph.h"
#import "HWMSensor.h"


@implementation HWMGraph

@dynamic hidden;
@dynamic sensor;

-(NSArray *)history
{
    return _history;
}

- (void)captureValueToHistorySetLimit:(NSUInteger)limit
{
    if (!_history) {
        _history = [[NSMutableArray alloc] init];
    }

    [_history addObject:self.sensor.value];

    if (_history.count > limit) {
        [_history removeObjectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _history.count - limit - 1)]];
    }
}

@end
