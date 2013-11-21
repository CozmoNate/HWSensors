//
//  BasicCell.m
//  HWMonitor
//
//  Created by Kozlek on 20/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "BasicCell.h"

#import "ColorTheme.h"

#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMItem.h"

@implementation BasicCell

-(id)init
{
    self = [super init];

    if (self) {
        [self addObserver:self forKeyPath:@"managedObject.engine.configuration.colorThemeName" options:NSKeyValueObservingOptionNew context:nil];
    }

    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
    // Drawing code here.
}

-(void)setColorThemeName:(NSString *)colorThemeName
{
    if (![colorThemeName isEqualToString:_colorThemeName]) {

        //[self willChangeValueForKey:@"colorTheme"];

        [self setColorTheme:[ColorTheme colorThemeByName:colorThemeName]];

        //[self didChangeValueForKey:@"colorTheme"];

    }

    _colorThemeName = colorThemeName;
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"managedObject.engine.configuration.colorThemeName"])
    {
        HWMItem* item = (HWMItem*)self.managedObject;
        [self setColorThemeName:item.engine.configuration.colorThemeName];
    }
}

@end
