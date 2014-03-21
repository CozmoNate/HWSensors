//
//  BatteryCell.m
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//


/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import "PopupBatteryCell.h"
#import "HWMColorTheme.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMItem.h"
#import "HWMSensor.h"

@implementation PopupBatteryCell

-(void)initialize
{
    [super initialize];

    [self addObserver:self forKeyPath:@"objectValue.value" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)deallocate
{
    [super deallocate];

    [self removeObserver:self forKeyPath:@"objectValue.value"];
}

- (void)setGaugeLevel:(NSNumber *)gaugeLevel
{
    _gaugeLevel = gaugeLevel;
    
    if (self.imageView && !NSEqualSizes(self.imageView.bounds.size, NSZeroSize)) {

        if (!_gaugeImage) {
            _gaugeImage = [[NSImage alloc] initWithSize:self.imageView.bounds.size];
            [self.imageView setImage:_gaugeImage];
        }

        [_gaugeImage lockFocus];
        
        //NSRectFillUsingOperation(self.imageView.bounds, NSCompositeClear);
        [[NSColor clearColor] set];
        NSRectFill(self.imageView.bounds);

        [[self.objectValue engine].configuration.colorTheme.itemTitleColor setStroke];
        
        [[NSBezierPath bezierPathWithRect:NSMakeRect(self.imageView.image.size.width / 2 - self.imageView.image.size.width / 4 / 2, self.imageView.image.size.height - 0.5, self.imageView.image.size.width / 4, 1)] stroke];
        [[NSBezierPath bezierPathWithRoundedRect:NSMakeRect(0.5, 1.5, self.imageView.image.size.width - 1, self.imageView.image.size.height - 3) xRadius:0.0 yRadius:0.0] stroke];

        NSColor *fillColor = nil;

        if ([_gaugeLevel integerValue] < 20) {
            fillColor = [NSColor redColor];
        }
        else if ([_gaugeLevel integerValue] < 35) {
            fillColor = [NSColor orangeColor];
        }
        else {
            fillColor = [NSColor greenColor];
        }

        fillColor = [self.objectValue engine].configuration.colorTheme.useBrightIcons.boolValue ? [fillColor highlightWithLevel:0.1] : [fillColor shadowWithLevel:0.1];

        [fillColor setFill];
        
        [[NSBezierPath bezierPathWithRect:NSMakeRect(1.75, 2.75, self.imageView.image.size.width - 3.5, (self.imageView.image.size.height - 5.5) * [_gaugeLevel doubleValue]  * 0.01)] fill];
        
        [_gaugeImage unlockFocus];
        
        [self.imageView setNeedsDisplay:YES];
    }
}

-(void)colorThemeChanged:(HWMColorTheme *)newColorTheme
{
    [super colorThemeChanged:newColorTheme];
    [self setGaugeLevel:self.gaugeLevel];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"objectValue.value"])
    {
        [self setGaugeLevel:[(HWMSensor*)self.objectValue value]];
    }

    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

@end
