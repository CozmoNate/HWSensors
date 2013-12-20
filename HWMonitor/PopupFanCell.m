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
#import "NSPopover+Message.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"

@implementation PopupFanController

@synthesize objectValue;

-(void)awakeFromNib
{
    [_slider setFloatValue:[[(HWMSmcFanSensor *)self.objectValue speed] floatValue]];
    [_textField setFloatValue:[[(HWMSmcFanSensor *)self.objectValue speed] floatValue]];
    [_textField setFont:[NSFont fontWithName:@"Let's go Digital Regular" size:18]];
}

- (IBAction)sliderHasMoved:(id)sender
{
    [_textField setFloatValue:_slider.floatValue];
     
    SEL sel = @selector(sliderHasBeenReleased:);
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:sel object:sender];
    [self performSelector:sel withObject:sender afterDelay:0.0];
}

- (void)sliderHasBeenReleased:(id)sender
{
    [(HWMSmcFanSensor *)self.objectValue setSpeed:[NSNumber numberWithFloat:_slider.floatValue]];
}

@end;

static NSPopover *gFanControllerPopover;

@implementation PopupFanCell

-(void)showFanController:(id)sender
{    
    HWMSmcFanSensor *fan = self.objectValue;
    
    if ([fan number] && [fan min] && [fan max] && [fan speed]) {
        
        if (gFanControllerPopover) {
            if (!gFanControllerPopover.isShown || [(PopupFanController*)gFanControllerPopover.contentViewController objectValue] != self.objectValue) {
                [gFanControllerPopover performClose:self];
                gFanControllerPopover = nil;
            }
            else {
                return;
            }
        }
        
        PopupFanController *controller = [[PopupFanController alloc] initWithNibName:@"PopupFanController" bundle:[NSBundle mainBundle]];
        
        [controller setObjectValue:self.objectValue];
        
        COICOPopoverView *container = (COICOPopoverView *)[controller view];
        
        [container setBackgroundColour:[self.objectValue engine].configuration.colorTheme.listBackgroundColor];
        
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
    if ([[self.objectValue engine] configuration].enableFanControl.boolValue) {
        [self performSelector:@selector(showFanController:) withObject:self afterDelay:0.5];
    }
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
