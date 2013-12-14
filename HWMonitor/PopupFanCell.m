//
//  PopupFanCell.m
//  HWMonitor
//
//  Created by Kozlek on 14.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupFanCell.h"
#import "HWMSmcFanSensor.h"
#import "Localizer.h"

@implementation PopupFanController

@synthesize objectValue;

@end;

static NSPopover *gFanControllerPopover;

@implementation PopupFanCell

-(void)showFanController:(id)sender
{
    if (gFanControllerPopover) {
        [gFanControllerPopover performClose:self];
        gFanControllerPopover = nil;
    }
    
    if ([self.objectValue number] && [self.objectValue min] && [self.objectValue max]) {
        
        PopupFanController *controller = [[PopupFanController alloc] initWithNibName:@"FanController" bundle:[NSBundle mainBundle]];
        
        [controller setObjectValue:self.objectValue];
        [Localizer localizeView:controller.view];
        
        gFanControllerPopover = [[NSPopover alloc] init];
        
        [gFanControllerPopover setContentViewController:controller];
        [gFanControllerPopover setAnimates:YES];
        [gFanControllerPopover setBehavior:NSPopoverBehaviorTransient];
        [gFanControllerPopover showRelativeToRect:self.frame ofView:self preferredEdge:NSMinXEdge];
    }
}

-(void)resetCursorRects
{
    if (_trackingRectTag) {
        [self removeTrackingRect:_trackingRectTag];
    }
    
    _trackingRectTag = [self addTrackingRect:[self frame] owner:self userData:nil assumeInside:YES];
}

-(void)mouseEntered:(NSEvent *)theEvent
{
    [self performSelector:@selector(showFanController:) withObject:self afterDelay:0.5];
}

-(void)mouseDown:(NSEvent *)theEvent
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    [super mouseDown:theEvent];
}

-(void)mouseExited:(NSEvent *)theEvent
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    [super mouseExited:theEvent];
}


@end
