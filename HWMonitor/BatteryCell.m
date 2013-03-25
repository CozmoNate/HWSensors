//
//  BatteryCell.m
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "BatteryCell.h"

@implementation BatteryCell

-(void)setColorTheme:(ColorTheme *)colorTheme
{
    _colorTheme = colorTheme;
    
    [[self textField] setTextColor:_colorTheme.itemTitleColor];
    [[self subtitleField] setTextColor:_colorTheme.itemSubTitleColor];
    [[self valueField] setTextColor:_colorTheme.itemValueTitleColor];
}

- (void)setGaugeLevel:(NSUInteger)gaugeLevel
{
    _gaugeLevel = gaugeLevel;
    
    if (self.imageView) {
        if (!self.imageView.image) {
            [self.imageView setImage:[[NSImage alloc] initWithSize:self.imageView.bounds.size]];
        }
        
        [self.imageView.image lockFocus];
        
        NSRectFillUsingOperation(self.imageView.bounds, NSCompositeClear);
        
        [[_colorTheme.itemTitleColor highlightWithLevel:0.2] setStroke];
        
        [[NSBezierPath bezierPathWithRect:NSMakeRect(self.imageView.image.size.width / 2 - self.imageView.image.size.width / 4 / 2, self.imageView.image.size.height - 0.5, self.imageView.image.size.width / 4, 1)] stroke];
        [[NSBezierPath bezierPathWithRoundedRect:NSMakeRect(0.5, 1.5, self.imageView.image.size.width - 1, self.imageView.image.size.height - 3) xRadius:0.0 yRadius:0.0] stroke];
        
        if (_gaugeLevel < 20) {
            [[[NSColor redColor] shadowWithLevel:_colorTheme.useDarkIcons ? 0.0 : 0.1] setFill];
        }
        else {
            [[[NSColor greenColor] shadowWithLevel:_colorTheme.useDarkIcons ? 0.0 : 0.1] setFill];
        }
        
        [[NSBezierPath bezierPathWithRect:NSMakeRect(1.75, 2.75, self.imageView.image.size.width - 3.5, (self.imageView.image.size.height - 5.5) * (double)_gaugeLevel * 0.01)] fill];
        
        [self.imageView.image unlockFocus];
        
        [self.imageView setNeedsDisplay:YES];
    }
}

@end
