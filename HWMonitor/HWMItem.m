//
//  HWMItem.m
//  HWMonitor
//
//  Created by Kozlek on 07.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMItem.h"
#import "HWMEngine.h"

#import <ReactiveCocoa/ReactiveCocoa.h>

@implementation HWMItem

@dynamic hidden;
@dynamic identifier;
@dynamic legend;
@dynamic name;
@dynamic title;
@dynamic favorites;

@synthesize engine;

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

-(void)initialize
{
    [RACObserve(self, hidden) subscribeNext:^(id x) {
        [self.engine setNeedsUpdateSensorLists];
    }];
}

@end
