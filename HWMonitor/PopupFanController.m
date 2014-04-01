//
//  PopupFanController.m
//  HWMonitor
//
//  Created by Kozlek on 08.02.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
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

#import "PopupFanController.h"
#import "PopupLevelCell.h"

#import "HWMSmcFanSensor.h"
#import "HWMSmcFanController.h"
#import "HWMSmcFanControlLevel.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMSensorsGroup.h"
#import "NSPopover+Message.h"
#import "Localizer.h"

#import "NSTableView+HWMEngineHelper.h"
#import "NSView+NSLayoutConstraintFilter.h"
#import "NSWindow+BackgroundBlur.h"

@interface PopupFanController ()

@end

@implementation PopupFanController

@synthesize controller = _controller;
@synthesize inputSources = _inputSources;

-(void)setController:(HWMSmcFanController *)controller
{

    if (_controller) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self removeObserver:self forKeyPath:@"controller.levels"];
            [self removeObserver:self forKeyPath:@"controller.output.engine.sensorsAndGroups"];
        }];
    }

    //[self willChangeValueForKey:@"controller"];
    _controller = controller;
    //[self didChangeValueForKey:@"controller"];

    if (_controller) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self addObserver:self forKeyPath:@"controller.levels" options:NSKeyValueObservingOptionNew context:nil];
            [self addObserver:self forKeyPath:@"controller.output.engine.sensorsAndGroups" options:NSKeyValueObservingOptionNew context:nil];
        }];
    }

    HWMColorTheme *colorTheme = [HWMEngine defaultEngine].configuration.colorTheme;

    [_inputLabel setTextColor:colorTheme.groupTitleColor];
    [_outputLabel setTextColor:colorTheme.groupTitleColor];

    [_enabledSwitch setAlphaValue:colorTheme.useBrightIcons.boolValue ? 0.80 : 1.0];
    [_inputsPopUp setAlphaValue:colorTheme.useBrightIcons.boolValue ? 0.80 : 1.0];
    [_inputsPopUp setButtonType:colorTheme.useBrightIcons.boolValue ? NSOnOffButton : NSMomentaryChangeButton];

    [self observeValueForKeyPath:@"controller.levels" ofObject:nil change:nil context:(void*)self];
    [self observeValueForKeyPath:@"controller.output.engine.sensorsAndGroups" ofObject:nil change:nil context:nil];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"controller.levels"]) {
        NSArray *oldLevelsSnapshot = [_levelsSnapshot copy];
        _levelsSnapshot = [self.controller.levels.array copy];
        NSLayoutConstraint *constraint = [_levelsTableView.enclosingScrollView constraintForAttribute:NSLayoutAttributeHeight];
        if (context) {
            [_levelsTableView updateWithObjectValues:_levelsSnapshot previousObjectValues:oldLevelsSnapshot withRemoveAnimation:NSTableViewAnimationEffectNone insertAnimation:NSTableViewAnimationEffectNone];
            [constraint setConstant:_levelsSnapshot.count * 28 + 1];
        }
        else {
            [_levelsTableView updateWithObjectValues:_levelsSnapshot previousObjectValues:oldLevelsSnapshot withRemoveAnimation:NSTableViewAnimationEffectNone insertAnimation:NSTableViewAnimationEffectNone];
            [[constraint animator] setConstant:_levelsSnapshot.count * 28 + 1];
        }
    }
    else if ([keyPath isEqualToString:@"controller.output.engine.sensorsAndGroups"]) {
        [self willChangeValueForKey:@"inputSources"];
        _inputSources = [self.controller.output.engine.sensorsAndGroups filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"className != %@ AND (selector == %@ OR selector == %@)", @"HWMSensorsGroup", @kHWMGroupTemperature, @kHWMGroupSmartTemperature]];
        [self didChangeValueForKey:@"inputSources"];
    }
}

-(void)dealloc
{
    if (_controller) {
        [self removeObserver:self forKeyPath:@"controller.levels"];
        [self removeObserver:self forKeyPath:@"controller.output.engine.sensorsAndGroups"];
    }
}

-(NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (_levelsSnapshot) {
        return _levelsSnapshot.count;
    }

    return 0;
}

-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 28;
}

-(id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return [_levelsSnapshot objectAtIndex:row];
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    PopupLevelCell *cell = [tableView makeViewWithIdentifier:@"level" owner:self];

    HWMColorTheme *colorTheme = [HWMEngine defaultEngine].configuration.colorTheme;
    
    NSColor *textColor = colorTheme.useBrightIcons.boolValue ? [colorTheme.itemValueTitleColor shadowWithLevel:0.15] : [colorTheme.itemValueTitleColor highlightWithLevel:0.35];

    [cell.inputTextField setTextColor:textColor];
    [cell.outputTextField setTextColor:textColor];
    
    return cell;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

@end
