//
//  HWMItem.m
//  HWMonitor
//
//  Created by Kozlek on 07.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMItem.h"
#import "HWMEngine.h"

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

    [self addObserver:self forKeyPath:@keypath(self, hidden) options:NSKeyValueObservingOptionNew context:nil];
}

-(void)awakeFromInsert
{
    [super awakeFromInsert];

    [self addObserver:self forKeyPath:@keypath(self, hidden) options:NSKeyValueObservingOptionNew context:nil];
}

-(void)prepareForDeletion
{
    [super prepareForDeletion];

    [self removeObserver:self forKeyPath:@keypath(self, hidden)];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@keypath(self, hidden)]) {
        [self.engine setNeedsUpdateSensorLists];
    }
}

@end
