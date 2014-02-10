//
//  PopupFanController.m
//  HWMonitor
//
//  Created by Kozlek on 08.02.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopupFanController.h"

#import "HWMSmcFanSensor.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "NSPopover+Message.h"

@interface PopupFanController ()

@end

@implementation PopupFanController

@synthesize sensor = _sensor;
@synthesize colorTheme = _colorTheme;

#define ROUND_50(x) (((int)x / 50) * 50)

-(void)setSensor:(HWMSmcFanSensor *)sensor
{
    _sensor = sensor;

    float min = [[_sensor valueForKey:@"min"] floatValue];
    float max = [[_sensor valueForKey:@"max"] floatValue];
    float speed = [[_sensor valueForKey:@"speed"] floatValue];

    __block NSInteger rounded = ROUND_50(speed);

    [_targetSlider setMinValue:min];
    [_targetSlider setMaxValue:max];

    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [_targetSlider setFloatValue:rounded];
    }];

    [_targetTextField setIntegerValue:rounded];
    [_targetTextField setFont:[NSFont fontWithName:@"Let's go Digital Regular" size:20]];
}

-(void)setColorTheme:(HWMColorTheme *)colorTheme
{
    _colorTheme = colorTheme;

    COICOPopoverView *container = (COICOPopoverView *)[self view];

    [container setBackgroundColour:_colorTheme.useDarkIcons.boolValue ?
     [_colorTheme.listBackgroundColor colorWithAlphaComponent:0.5]:
     nil /*[self.colorTheme.listBackgroundColor shadowWithLevel:0.05]*/];

    NSColor *textColor = _colorTheme.useDarkIcons.boolValue ?
    _colorTheme.itemValueTitleColor :
    [_colorTheme.itemValueTitleColor highlightWithLevel:0.35];

    [_targetTextField setTextColor:textColor];
}

- (void)sliderHasMoved:(id)sender
{
    [_targetTextField setIntegerValue:ROUND_50(_targetSlider.integerValue)];

    SEL sel = @selector(sliderHasBeenReleased:);

    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:sel object:sender];

    [self performSelector:sel withObject:sender afterDelay:0.0];
}

- (void)sliderHasBeenReleased:(id)sender
{
    [_sensor setSpeed:[NSNumber numberWithInteger:ROUND_50(_targetSlider.integerValue)]];
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    return self;
}

@end
