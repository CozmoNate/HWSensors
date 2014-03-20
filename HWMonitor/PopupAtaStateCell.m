//
//  PopupAtaStateCell.m
//  HWMonitor
//
//  Created by Kozlek on 07.03.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopupAtaStateCell.h"
#import "HWMSensor.h"

@implementation PopupAtaStateCell

-(void)drawStateImageInFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSImage *image = nil;
    CGFloat fraction = 1.0;

    switch (self.state) {
        case kHWMSensorLevelDisabled:
            image = [NSImage imageNamed:NSImageNameStatusNone];
            fraction = 0.75;
            break;

        case kHWMSensorLevelModerate:
        case kHWMSensorLevelHigh:
            image = [NSImage imageNamed:NSImageNameStatusPartiallyAvailable];
            break;

        case kHWMSensorLevelExceeded:
            image = [NSImage imageNamed:NSImageNameStatusUnavailable];
            break;

            //case kHWMSensorLevelDisabled:
        default:
            image = [NSImage imageNamed:NSImageNameStatusAvailable];
            break;
    }

    if (image) {
        [image drawInRect:NSMakeRect(cellFrame.origin.x + (cellFrame.size.width - image.size.width) / 2,
                                     cellFrame.origin.y + (cellFrame.size.height - image.size.height) / 2,
                                     image.size.width,
                                     image.size.height)
                 fromRect:NSZeroRect
                operation:NSCompositeSourceOver
                 fraction:fraction];
    }
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    [self drawStateImageInFrame:cellFrame inView:controlView];
}

- (void)highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    //if (flag)
    [self drawStateImageInFrame:cellFrame inView:controlView];
}

@end
