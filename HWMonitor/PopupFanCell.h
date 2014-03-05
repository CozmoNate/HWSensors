//
//  PopupFanCell.h
//  HWMonitor
//
//  Created by Kozlek on 14.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupSensorCell.h"

@class HWMSmcFanSensor, HWMColorTheme;


@interface PopupFanCell : PopupSensorCell
{
    NSPopover *_popover;
}

- (IBAction)showFanController:(id)sender;

@end
