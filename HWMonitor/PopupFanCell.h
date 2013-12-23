//
//  PopupFanCell.h
//  HWMonitor
//
//  Created by Kozlek on 14.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupSensorCell.h"

@class HWMSmcFanSensor, HWMColorTheme;

@interface PopupFanController : PopupPopoverController
{
    NSDate *_lastUpdated;
}

@property (assign) IBOutlet NSSlider *slider;
@property (assign) IBOutlet NSTextField *textField;

- (IBAction)sliderHasMoved:(id)sender;

@end;

@interface PopupFanCell : PopupSensorCell <NSPopoverDelegate>
{
    NSPopover *_popover;
}

- (IBAction)showFanController:(id)sender;

@end
