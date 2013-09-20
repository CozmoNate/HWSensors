//
//  ATASensorCell.h
//  HWMonitor
//
//  Created by Kozlek on 14/09/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "SensorCell.h"
#import "ATAGenericDrive.h"

@interface ATASensorCell : SensorCell
{
@private
    NSTrackingRectTag _trackingRectTag;
    NSPopover *_popover;
    BOOL _cursorIsInsideTheFrame;
}

@property (nonatomic, readonly) NSString *smartOutput;
@property (strong) ATAGenericDrive *genericDrive;

@end
