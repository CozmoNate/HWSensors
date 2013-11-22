//
//  ATASensorCell.h
//  HWMonitor
//
//  Created by Kozlek on 14/09/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#include "PopupSensorCell.h"

@interface PopupAtaSmartSensorCell : PopupSensorCell
{
    NSTrackingRectTag _trackingRectTag;
    NSPopover *_popover;
}

@property (nonatomic, readonly) NSString *smartOutput;

- (IBAction)showSmartOutput:(id)sender;

@end
