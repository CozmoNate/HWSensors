//
//  GraphsSensorCell.m
//  HWMonitor
//
//  Created by kozlek on 25.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "GraphsSensorCell.h"

#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMGraph.h"
#import "HWMSensor.h"

@implementation GraphsSensorCell

-(id)init
{
    self = [super init];

    if (self) {
        [self initialize];
    }

    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if (self) {
        [self initialize];
    }

    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];

    if (self) {
        [self initialize];
    }

    return self;
}

-(void)dealloc
{
    [self removeObserver:self forKeyPath:@"objectValue.sensor.alarmLevel"];
}

-(void)initialize
{
    [self addObserver:self forKeyPath:@"objectValue.sensor.alarmLevel" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)resetCursorRects
{
    [self discardCursorRects];

    //[self addCursorRect:self.frame cursor:[NSCursor openHandCursor]];

    if (_checkBox) {
        [self addCursorRect:_checkBox.frame cursor:[NSCursor pointingHandCursor]];
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"objectValue.sensor.alarmLevel"]) {

        switch ([self.objectValue sensor].alarmLevel) {
            case kHWMSensorLevelNormal:
                [self.valueField setTextColor:[NSColor whiteColor]];
                break;

            case kHWMSensorLevelModerate:
                [self.valueField setTextColor:[NSColor colorWithCalibratedRed:0.7f green:0.3f blue:0.03f alpha:1.0f]];
                break;

            case kHWMSensorLevelHigh:
                [self.valueField setTextColor:[NSColor redColor]];
                break;

            case kHWMSensorLevelExceeded:
                [self.textField setTextColor:[NSColor redColor]];
                [self.valueField setTextColor:[NSColor redColor]];
                break;
        }

    }
}

@end
