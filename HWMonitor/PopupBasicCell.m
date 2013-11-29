//
//  PopupBasicCell.m
//  HWMonitor
//
//  Created by Kozlek on 22/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupBasicCell.h"
#import "HWMEngine.h"
#import "HWMItem.h"
#import "HWMConfiguration.h"

@implementation PopupBasicCell

-(id)init
{
    self = [super init];

    if (self) {
        [self didInitialized];
    }

    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if (self) {
        [self didInitialized];
    }

    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];

    if (self) {
        [self didInitialized];
    }

    return self;
}

-(void)dealloc
{
    [self willDeallocated];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"objectValue.engine.configuration.colorTheme"]) {
        [self colorThemeHasChanged:[(HWMItem*)self.objectValue engine].configuration.colorTheme];
    }
}

- (void)colorThemeHasChanged:(HWMColorTheme*)newColorTheme
{

}
- (void)didInitialized
{
    [self addObserver:self forKeyPath:@"objectValue.engine.configuration.colorTheme" options:NSKeyValueObservingOptionNew context:nil];
}

- (void)willDeallocated
{
    [self removeObserver:self forKeyPath:@"objectValue.engine.configuration.colorTheme"];
}

@end
