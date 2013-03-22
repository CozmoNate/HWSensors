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

-(void)setGaugeLevel:(NSUInteger)gaugeLevel
{
    _gaugeLevel = gaugeLevel;
    
    if (self.imageView) {

        if (!self.imageView.image) {
            [self.imageView setImage:[[NSImage alloc] initWithSize:self.imageView.bounds.size]];
        }
        
        [self.imageView.image lockFocus];
        

        [[_colorTheme.itemTitleColor highlightWithLevel:0.2] setStroke];
        
        [[NSBezierPath bezierPathWithRect:NSMakeRect(self.imageView.image.size.width / 3, self.imageView.image.size.height - 0.5, self.imageView.image.size.width / 3, 1)] stroke];
        [[NSBezierPath bezierPathWithRoundedRect:NSMakeRect(0.5, 0.5, self.imageView.image.size.width - 1, self.imageView.image.size.height - 2) xRadius:1.0 yRadius:1.0] stroke];
        
        if (gaugeLevel < 20) {
            [[[NSColor redColor] shadowWithLevel:0.1] setFill];
        }
        else {
            [[[NSColor greenColor] shadowWithLevel:0.1] setFill];
        }
        
        [[NSBezierPath bezierPathWithRect:NSMakeRect(1.75, 1.75, self.imageView.image.size.width - 3.5, (self.imageView.image.size.height - 4.5) * (double)_gaugeLevel * 0.01)] fill];
        
        [self.imageView.image unlockFocus];
    }
}

@end
