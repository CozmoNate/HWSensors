//
//  BatteryCell.m
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "BatteryCell.h"

@implementation BatteryCell

-(void)setGaugeLevel:(NSUInteger)gaugeLevel
{
    _gaugeLevel = gaugeLevel;
    
    if (_gaugeView) {
        [_gaugeView.image lockFocus];
        
        NSRect rect = NSMakeRect(0, 0, _gaugeView.image.size.width, _gaugeView.image.size.height);
        
        [[NSColor clearColor] setFill];
        [[NSBezierPath bezierPathWithRect:rect] fill];
        
        NSBezierPath *path = [NSBezierPath bezierPathWithRect:NSInsetRect(rect, 1, 1)];

        [[NSColor colorWithCalibratedWhite:0.0 alpha:0.5] setFill];
        [path setLineWidth:0.5];
        //[path stroke];
        
        path = [NSBezierPath bezierPathWithRect:NSMakeRect(2.5, 2.5, _gaugeView.image.size.width - 5, (_gaugeView.image.size.height - 5) * (double)_gaugeLevel * 0.01)];
        
        [[NSColor greenColor] setFill];
        [path fill];
        
        [_gaugeView.image unlockFocus];
    }
}

@end
