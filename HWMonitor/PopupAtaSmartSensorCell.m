//
//  ATASensorCell.m
//  HWMonitor
//
//  Created by Kozlek on 14/09/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import "PopupAtaSmartSensorCell.h"
#import "HWMAtaSmartSensor.h"
#import "HWMColorTheme.h"
#import "HWMItem.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "NSPopover+Message.h"
#import "PopupAtaSmartReportController.h"
#import "NSTableHeaderCell+PopupThemedHeader.h"
#import "NSView+NSLayoutConstraintFilter.h"

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

    PopupAtaSmartReportController *controller = nil;

    _popover = [PopupSensorCell globalPopover];

    if (_popover && _popover.contentViewController && [_popover.contentViewController isKindOfClass:[PopupAtaSmartReportController class]]) {

        controller = (PopupAtaSmartReportController *)_popover.contentViewController;
    }
    else {
        [PopupSensorCell destroyGlobalPopover];
        _popover = [PopupSensorCell globalPopover];

        controller = [[PopupAtaSmartReportController alloc] initWithNibName:NSStringFromClass([PopupAtaSmartReportController class]) bundle:nil];

        _popover.contentViewController = controller;
    }

    _popover.delegate = nil;

    _popover.behavior = NSPopoverBehaviorTransient;
    _popover.appearance = sensor.engine.configuration.colorTheme.useBrightIcons.boolValue ? NSPopoverAppearanceHUD : NSPopoverAppearanceMinimal;

    _popover.delegate = controller;

    [controller setSensor:sensor];

    //NSLayoutConstraint *constraint = [controller.tableView.enclosingScrollView constraintForAttribute:NSLayoutAttributeHeight];
    //[[constraint animator] setConstant:(controller.tableView.rowHeight + 1) * (sensor.attributes.count + 1)];
    [_popover showRelativeToRect:self.frame ofView:self preferredEdge:NSMinXEdge];
    [_popover setContentSize:NSMakeSize(controller.view.frame.size.width, (controller.tableView.rowHeight + 1) * (sensor.attributes.count + 1) + 7)];
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
