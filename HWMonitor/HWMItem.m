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

@synthesize hasUpdates = _changed;
@synthesize engine;

-(BOOL)hasUpdates
{
    if (_changed) {
        _changed = NO;
        
        return YES;
    }
    
    return NO;
}

-(void)awakeFromFetch
{
    [super awakeFromFetch];

    [self addObserver:self forKeyPath:@"hidden" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)awakeFromInsert
{
    [super awakeFromInsert];

    [self addObserver:self forKeyPath:@"hidden" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)prepareForDeletion
{
    [super prepareForDeletion];

    [self removeObserver:self forKeyPath:@"hidden"];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"hidden"]) {
        [self.engine setNeedsUpdateSensorLists];
    }
}

@end
