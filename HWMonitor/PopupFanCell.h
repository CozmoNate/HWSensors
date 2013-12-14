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
{
    NSDate *_lastUpdated;
}

@property (nonatomic, strong) id objectValue;

@property (assign) IBOutlet NSSlider *slider;
@property (assign) IBOutlet NSTextField *textField;

- (IBAction)sliderHasMoved:(id)sender;

@end;

@interface PopupFanCell : PopupBasicCell
{
    NSTrackingRectTag _trackingRectTag;
}

- (IBAction)showFanController:(id)sender;

@end
