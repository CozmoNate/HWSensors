//
//  HWMGraphsGroup.m
//  HWMonitor
//
//  Created by Kozlek on 27/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMGraphsGroup.h"
#import "HWMConfiguration.h"
#import "HWMGraph.h"
#import "HWMIcon.h"
#import "HWMSensorsGroup.h"


@implementation HWMGraphsGroup

@dynamic configuration;
@dynamic graphs;
@dynamic icon;

@synthesize selectors;

-(void)addGraphsObject:(HWMGraph *)value
{
    [[self mutableOrderedSetValueForKey:@"graphs"] addObject:value];
}

- (void)moveGraphsObject:(HWMGraph *)value toIndex:(NSUInteger)toIndex
{
    if (toIndex > self.graphs.count) {
        toIndex = self.graphs.count;
    }
    
    NSUInteger fromIndex = [self.graphs indexOfObject:value];
    
    [[self mutableOrderedSetValueForKey:@"graphs"] moveObjectsAtIndexes:[NSIndexSet indexSetWithIndex:fromIndex] toIndex:fromIndex < toIndex || toIndex == self.graphs.count ? toIndex - 1 : toIndex];
}

@end
