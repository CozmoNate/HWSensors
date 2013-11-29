//
//  BatteryCell.m
//  HWMonitor
//
//  Created by kozlek on 11.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupBatteryCell.h"
#import "HWMColorTheme.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMItem.h"
#import "HWMSensor.h"

@implementation PopupBatteryCell

-(id)init
{
    self = [super init];

    if (self) {
        [self addObserver:self forKeyPath:@"objectValue.value" options:NSKeyValueObservingOptionNew context:nil];
    }

    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if (self) {
        [self addObserver:self forKeyPath:@"objectValue.value" options:NSKeyValueObservingOptionNew context:nil];
    }

    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];

    if (self) {
        [self addObserver:self forKeyPath:@"objectValue.value" options:NSKeyValueObservingOptionNew context:nil];
    }
    
    return self;
}

-(void)dealloc
{
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
        
        NSRectFillUsingOperation(self.imageView.bounds, NSCompositeClear);
        
        [[[self.objectValue engine].configuration.colorTheme.itemTitleColor highlightWithLevel:0.2] setStroke];
        
        [[NSBezierPath bezierPathWithRect:NSMakeRect(self.imageView.image.size.width / 2 - self.imageView.image.size.width / 4 / 2, self.imageView.image.size.height - 0.5, self.imageView.image.size.width / 4, 1)] stroke];
        [[NSBezierPath bezierPathWithRoundedRect:NSMakeRect(0.5, 1.5, self.imageView.image.size.width - 1, self.imageView.image.size.height - 3) xRadius:0.0 yRadius:0.0] stroke];
        
        if ([_gaugeLevel integerValue] < 20) {
            [[[NSColor redColor] shadowWithLevel:[self.objectValue engine].configuration.colorTheme.useDarkIcons ? 0.0 : 0.1] setFill];
        }
        else {
            [[[NSColor greenColor] shadowWithLevel:[self.objectValue engine].configuration.colorTheme.useDarkIcons ? 0.0 : 0.1] setFill];
        }
        
        [[NSBezierPath bezierPathWithRect:NSMakeRect(1.75, 2.75, self.imageView.image.size.width - 3.5, (self.imageView.image.size.height - 5.5) * [_gaugeLevel doubleValue]  * 0.01)] fill];
        
        [_gaugeImage unlockFocus];
        
        [self.imageView setNeedsDisplay:YES];
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"objectValue.value"])
    {
        [self setGaugeLevel:[(HWMSensor*)self.objectValue value]];
    }
}

@end
