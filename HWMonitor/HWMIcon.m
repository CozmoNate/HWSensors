//
//  HWMIcon.m
//  HWMonitor
//
//  Created by Kozlek on 16/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMIcon.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMEngine.h"

@implementation HWMIcon

@dynamic alternate;
@dynamic favorite;
@dynamic regular;

@synthesize image = _image;

-(NSImage *)image
{
    if (!_image) {
        _image = self.engine.configuration.colorTheme.useDarkIcons.boolValue ? self.alternate : self.regular;
    }

    return _image;
}

-(void)prepareForDeletion
{
    [self removeObserver:self.engine forKeyPath:@"engine.configuration.colorTheme"];
}

-(void)setEngine:(HWMEngine *)engine
{
    if (self.engine) {
        [self removeObserver:self.engine forKeyPath:@"engine.configuration.colorTheme"];
    }

    [super setEngine:engine];

    if (self.engine) {
        [self addObserver:self forKeyPath:@"engine.configuration.colorTheme" options:NSKeyValueObservingOptionNew context:nil];
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"engine.configuration.colorTheme"]) {
        [self willChangeValueForKey:@"image"];
        _image = nil;
        [self didChangeValueForKey:@"image"];
    }
}

@end
