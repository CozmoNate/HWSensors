//
//  ATASensorCell.m
//  HWMonitor
//
//  Created by Kozlek on 14/09/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PopupAtaSmartSensorCell.h"
#import "HWMAtaSmartSensor.h"
#import "HWMColorTheme.h"
#import "HWMItem.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "NSPopover+Message.h"
#import "PopupAtaSmartReportController.h"
#import "NSTableHeaderCell+PopupThemedHeader.h"

@implementation PopupAtaSmartSensorCell

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
    [self performSelector:@selector(showSmartOutput:) withObject:self afterDelay:0.5];
}

-(void)mouseDown:(NSEvent *)theEvent
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    //[self hideSmartPopover];

    [super mouseDown:theEvent];
}

-(void)mouseExited:(NSEvent *)theEvent
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    //[self hideSmartPopover];

    [super mouseExited:theEvent];
}

- (void)showSmartOutput:(id)sender
{
    //[PopupSensorCell destroyGlobalPopover];

    HWMAtaSmartSensor *sensor = self.objectValue;

    [NSTableHeaderCell setGlobalColorTheme:sensor.engine.configuration.colorTheme];

    __block PopupAtaSmartReportController *controller = nil;

    _popover = [PopupSensorCell globalPopover];

    if (_popover && _popover.contentViewController && [_popover.contentViewController isKindOfClass:[PopupAtaSmartReportController class]]) {

        controller = (PopupAtaSmartReportController *)_popover.contentViewController;
    }
    else {
        [PopupSensorCell destroyGlobalPopover];
        _popover = [PopupSensorCell globalPopover];

        controller = [[PopupAtaSmartReportController alloc] initWithNibName:@"PopupAtaSmartReportController" bundle:nil];

        _popover.contentViewController = controller;
    }

    [controller setSensor:sensor];

    _popover.animates = YES;
    _popover.behavior = NSPopoverBehaviorTransient;

    _popover.delegate = controller;

    [_popover showRelativeToRect:self.frame ofView:self preferredEdge:NSMinXEdge];

    //dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^(void){
        [_popover setContentSize:NSMakeSize(controller.view.frame.size.width,
                                            (controller.tableView.rowHeight + 1) * (sensor.attributes.count + 1) + 7)];
    //});
}

- (void)hideSmartPopover
{
    if (_popover) {
        if (_popover == [PopupSensorCell globalPopover]) {
            [PopupSensorCell destroyGlobalPopover];
        }
        
        [_popover close];
        _popover = nil;
    }
}

@end
