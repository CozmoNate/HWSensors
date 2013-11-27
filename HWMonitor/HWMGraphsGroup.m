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

-(void)addGraphsObject:(HWMGraph *)value
{
    [[self mutableOrderedSetValueForKey:@"graphs"] addObject:value];
}

@end
