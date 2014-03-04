//
//  PopupFanController.m
//  HWMonitor
//
//  Created by Kozlek on 08.02.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopupFanController.h"
#import "PopupLevelCell.h"

#import "HWMSmcFanSensor.h"
#import "HWMSmcFanController.h"
#import "HWMSmcFanControlLevel.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "NSPopover+Message.h"
#import "Localizer.h"

#import "NSTableView+HWMEngineHelper.h"
#import "NSView+NSLayoutConstraintFilter.h"

@interface PopupFanController ()

@end

@implementation PopupFanController

@synthesize controller = _controller;

-(void)setController:(HWMSmcFanController *)controller
{

    if (_controller) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self removeObserver:self forKeyPath:@"controller.levels"];
        }];
    }

    //[self willChangeValueForKey:@"controller"];
    _controller = controller;
    //[self didChangeValueForKey:@"controller"];

    if (_controller) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self addObserver:self forKeyPath:@"controller.levels" options:NSKeyValueObservingOptionNew context:nil];
        }];
    }

    COICOPopoverView *container = (COICOPopoverView *)[self view];
    HWMColorTheme *colorTheme = [HWMEngine defaultEngine].configuration.colorTheme;

    [container setBackgroundColour:colorTheme.useDarkIcons.boolValue ?
     [colorTheme.listBackgroundColor colorWithAlphaComponent:0.5]:
     nil /*[self.colorTheme.listBackgroundColor shadowWithLevel:0.05]*/];


    [self observeValueForKeyPath:@"controller.levels" ofObject:nil change:nil context:nil];

//    float min = [[_sensor valueForKey:@"min"] floatValue];
//    float max = [[_sensor valueForKey:@"max"] floatValue];
//    float speed = [[_sensor valueForKey:@"speed"] floatValue];
//
//    __block NSInteger rounded = ROUND_50(speed);
//
//    [_targetSlider setMinValue:min];
//    [_targetSlider setMaxValue:max];
//
//    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
//        [_targetSlider setFloatValue:rounded];
//    }];
//
//    [_targetTextField setIntegerValue:rounded];
//    [_targetTextField setFont:[NSFont fontWithName:@"Let's go Digital Regular" size:20]];
}

//-(void)setColorTheme:(HWMColorTheme *)colorTheme
//{
//    _colorTheme = colorTheme;
//
//    COICOPopoverView *container = (COICOPopoverView *)[self view];
//
//    [container setBackgroundColour:_colorTheme.useDarkIcons.boolValue ?
//     [_colorTheme.listBackgroundColor colorWithAlphaComponent:0.5]:
//     nil /*[self.colorTheme.listBackgroundColor shadowWithLevel:0.05]*/];

//    NSColor *textColor = _colorTheme.useDarkIcons.boolValue ?
//    _colorTheme.itemValueTitleColor :
//    [_colorTheme.itemValueTitleColor highlightWithLevel:0.35];
//
//    [_targetTextField setTextColor:textColor];
//}


-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"controller.levels"]) {
        NSArray *oldLevelsSnapshot = [_levelsSnapshot copy];
        _levelsSnapshot = [self.controller.levels.array copy];
        NSLayoutConstraint *constraint = [_levelsTableView.enclosingScrollView constraintForAttribute:NSLayoutAttributeHeight];

        [NSAnimationContext beginGrouping];
        [[NSAnimationContext currentContext] setDuration:0.15];
        [_levelsTableView updateWithObjectValues:_levelsSnapshot previousObjectValues:oldLevelsSnapshot];
        [[constraint animator] setConstant:_levelsSnapshot.count * 28];
        [NSAnimationContext endGrouping];
    }
}

-(void)dealloc
{
    if (_controller) {
        [self removeObserver:self forKeyPath:@"controller.levels"];
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

    [Localizer localizeView:cell];

    HWMColorTheme *colorTheme = [HWMEngine defaultEngine].configuration.colorTheme;
    NSColor *textColor = colorTheme.useDarkIcons.boolValue ? colorTheme.itemValueTitleColor : [colorTheme.itemValueTitleColor highlightWithLevel:0.35];

    [cell.inputTextField setTextColor:textColor];
    [cell.outputTextField setTextColor:textColor];

    return cell;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

@end
