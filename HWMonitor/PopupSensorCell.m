//
//  HWMonitorSensorCell.m
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
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

#import "PopupSensorCell.h"
#import "HWMColorTheme.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensor.h"
#import "HWMSensorsGroup.h"

static NSPopover *gPopupSensorCellPopover;

@implementation PopupSensorCell

+(NSPopover*)globalPopover
{
    if (!gPopupSensorCellPopover) {
        gPopupSensorCellPopover = [[NSPopover alloc] init];
    }

    return gPopupSensorCellPopover;
}

+(void)destroyGlobalPopover
{
    if (gPopupSensorCellPopover) {
        if (gPopupSensorCellPopover.isShown) {
            [gPopupSensorCellPopover close];
        }
        gPopupSensorCellPopover = nil;
    }
}

+(void)setGlobalPopover:(NSPopover *)popover
{
    [PopupSensorCell destroyGlobalPopover];

    gPopupSensorCellPopover = popover;
}

-(void)initialize
{
    [super initialize];

    [self addObserver:self forKeyPath:@"objectValue.alarmLevel" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)deallocate
{
    [super deallocate];

    [self removeObserver:self forKeyPath:@"objectValue.alarmLevel"];
}

-(void)colorThemeChanged:(HWMColorTheme *)newColorTheme
{
    if ([self.objectValue alarmLevel] != kHWMSensorLevelExceeded) {
        [self.textField setTextColor:newColorTheme.itemTitleColor];
        [self.subtitleField setTextColor:newColorTheme.itemSubTitleColor];
    }

    if ([self.objectValue alarmLevel] == kHWMSensorLevelNormal) {
        [self.valueField setTextColor:newColorTheme.itemValueTitleColor];
    }
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"objectValue.alarmLevel"]) {

        switch ([self.objectValue alarmLevel]) {
            case kHWMSensorLevelNormal:
                [self.valueField setTextColor:[self.objectValue engine].configuration.colorTheme.itemValueTitleColor];
                break;

            case kHWMSensorLevelModerate:
                [self.valueField setTextColor:[NSColor colorWithCalibratedRed:0.7f green:0.3f blue:0.03f alpha:1.0f]];
                break;

            case kHWMSensorLevelHigh:
                [self.valueField setTextColor:[NSColor redColor]];
                break;

            case kHWMSensorLevelExceeded:
                [self.textField setTextColor:[NSColor redColor]];
                [self.subtitleField setTextColor:[NSColor redColor]];
                [self.valueField setTextColor:[NSColor redColor]];
                break;
        }

    }

    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

@end
