//
//  PopupFanController.h
//  HWMonitor
//
//  Created by Kozlek on 08.02.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

@class HWMColorTheme, HWMSmcFanSensor;

@interface PopupFanController : NSViewController
{
    NSDate *_lastUpdated;
}

@property (nonatomic, strong) HWMSmcFanSensor * sensor;
@property (nonatomic, strong) HWMColorTheme * colorTheme;

@property (assign) IBOutlet NSButton *controlledCheck;
@property (assign) IBOutlet NSSlider *targetSlider;
@property (assign) IBOutlet NSTextField *targetTextField;

- (IBAction)sliderHasMoved:(id)sender;

@end
