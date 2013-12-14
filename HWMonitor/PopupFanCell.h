//
//  PopupFanCell.h
//  HWMonitor
//
//  Created by Kozlek on 14.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupBasicCell.h"

@class HWMSmcFanSensor;

@interface PopupFanController : NSViewController

@property (nonatomic, strong) id objectValue;

@end;

@interface PopupFanCell : PopupBasicCell
{
    NSTrackingRectTag _trackingRectTag;
}

- (IBAction)showFanController:(id)sender;

@end
