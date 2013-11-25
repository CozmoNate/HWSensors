//
//  HWMGraphsGroup.m
//  HWMonitor
//
//  Created by Kozlek on 25/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMGraphsGroup.h"
#import "HWMGraph.h"
#import "HWMIcon.h"


@implementation HWMGraphsGroup

@dynamic selector;
@dynamic icon;
@dynamic graphs;

-(void)addGraphsObject:(HWMGraph *)value
{
    [[self mutableOrderedSetValueForKey:@"graphs"] addObject:value];
}

@end
