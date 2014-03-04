//
//  PopupFanCell.m
//  HWMonitor
//
//  Created by Kozlek on 14.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupFanCell.h"
#import "HWMSmcFanSensor.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMSmcFanController.h"
#import "Localizer.h"
#import "PopupFanController.h"

@implementation PopupFanCell

- (void)colorThemeChanged:(HWMColorTheme*)newColorTheme
{
    [super colorThemeChanged:newColorTheme];
    [PopupSensorCell destroyGlobalPopover];
}

- (void)updateTrackingAreas
{
    [super updateTrackingAreas];

    for (NSTrackingArea *area in [self trackingAreas]) {
		[self removeTrackingArea:area];
    }

    NSTrackingAreaOptions options = NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways;

    [self addTrackingArea:[[NSTrackingArea alloc] initWithRect:NSZeroRect options:options owner:self userInfo:nil]];
}

-(void)mouseEntered:(NSEvent *)theEvent
{
    HWMSmcFanSensor *fan = self.objectValue;

    if (fan.controller) {
        [self performSelector:@selector(showFanController:) withObject:self afterDelay:0.5];
    }

    [super mouseEntered:theEvent];
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


-(void)showFanController:(id)sender
{
    HWMSmcFanSensor *fan = self.objectValue;

    if ([fan number] && fan.controller.min && fan.controller.max) {

        //[PopupSensorCell destroyGlobalPopover];

        PopupFanController *controller = nil;

        _popover = [PopupSensorCell globalPopover];

        if (_popover && _popover.contentViewController && [_popover.contentViewController isKindOfClass:[PopupFanController class]]) {

            controller = (PopupFanController *)_popover.contentViewController;
        }
        else {
            [PopupSensorCell destroyGlobalPopover];

            _popover = [PopupSensorCell globalPopover];

            controller = [[PopupFanController alloc] initWithNibName:@"PopupFanController" bundle:[NSBundle mainBundle]];

            [Localizer localizeView:controller.view];

            _popover.contentViewController = controller;
        }

        _popover.delegate = nil;

        _popover.behavior = NSPopoverBehaviorTransient;
        _popover.appearance = fan.engine.configuration.colorTheme.useDarkIcons ? NSPopoverAppearanceMinimal : NSPopoverAppearanceHUD;

        [_popover showRelativeToRect:self.frame ofView:self preferredEdge:NSMinXEdge];
        [controller setController:fan.controller];
    }
}

@end
