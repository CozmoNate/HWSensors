//
//  HWMSensorsGroup.m
//  HWMonitor
//
//  Created by Kozlek on 25/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMSensorsGroup.h"
#import "HWMIcon.h"
#import "HWMSensor.h"
#import "HWMConfiguration.h"

@implementation HWMSensorsGroup

@dynamic selector;
@dynamic icon;
@dynamic sensors;

-(void)addSensorsObject:(HWMSensor *)value
{
    [[self mutableOrderedSetValueForKey:@"sensors"] addObject:value];
}

-(void)moveSensorsObject:(HWMSensor *)value toIndex:(NSUInteger)toIndex
{
    if (toIndex > self.sensors.count) {
        toIndex = self.sensors.count;
    }

    NSUInteger fromIndex = [self.sensors indexOfObject:value];

    [[self mutableOrderedSetValueForKey:@"sensors"] moveObjectsAtIndexes:[NSIndexSet indexSetWithIndex:fromIndex] toIndex:fromIndex < toIndex || toIndex == self.sensors.count ? toIndex - 1 : toIndex];
}

@end
