//
//  PopupController.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
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

#import "PopupController.h"

#import "Localizer.h"

#import "HWMonitorDefinitions.h"

#import "PopupGroupCell.h"
#import "PopupSensorCell.h"
#import "PopupAtaSmartSensorCell.h"
#import "PopupBatteryCell.h"

#import "JLNFadingScrollView.h"

#import "HWMColorTheme.h"
#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensorsGroup.h"
#import "HWMSensor.h"

#import "NSTableView+HWMEngineHelper.h"
#import "NSImage+HighResolutionLoading.h"
#import "NSWindow+BackgroundBlur.h"

@implementation PopupController

@synthesize statusItem = _statusItem;
@synthesize statusItemView = _statusItemView;
@synthesize toolbarView = _toolbarView;
@synthesize sensorsAndGroupsCollectionSnapshot = _sensorsAndGroupsCollectionSnapshot;

#pragma mark -
#pragma mark Properties

-(BOOL)hasDraggedFavoriteItem
{
    return YES;
}

-(NSArray *)sensorsAndGroupsCollectionSnapshot
{
    if (!_sensorsAndGroupsCollectionSnapshot) {
        _sensorsAndGroupsCollectionSnapshot = [self.monitorEngine.sensorsAndGroups copy];
    }

    return _sensorsAndGroupsCollectionSnapshot;
}

-(HWMEngine *)monitorEngine
{
    return [HWMEngine sharedEngine];
}

//-(void)setMonitorEngine:(HWMEngine *)monitorEngine
//{
//    if (monitorEngine != _monitorEngine) {
//
//        _monitorEngine = monitorEngine;
//
//        [_statusItemView setMonitorEngine:_monitorEngine];
//    }
//}

#pragma mark -
#pragma mark Overridden Methods

- (id)init
{
    self = [super initWithWindowNibName:NSStringFromClass([PopupController class])];
    
    if (self != nil)
    {
        _statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        
        _statusItemView = [[StatusItemView alloc] initWithFrame:NSMakeRect(0, 0, 22, 22) statusItem:_statusItem];

        [_statusItemView setImage:[NSImage loadImageNamed:@"scale" ofType:@"png"]];
        [_statusItemView setAlternateImage:[NSImage loadImageNamed:@"scale-white" ofType:@"png"]];

        [_statusItemView setAction:@selector(togglePanel:)];
        [_statusItemView setTarget:self];
    }
    
    return self;
}

- (void)dealloc
{
    [[NSStatusBar systemStatusBar] removeStatusItem:_statusItem];
}

-(void)windowDidLoad
{
    [super windowDidLoad];

    //[[_titleField cell] setBackgroundStyle:NSBackgroundStyleRaised];

    // Install status item into the menu bar
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;

    menubarWindow.statusItemView = _statusItemView;
    menubarWindow.statusItem = _statusItem;
    menubarWindow.attachedToMenuBar = YES;
    menubarWindow.hideWindowControls = YES;

    menubarWindow.toolbarView = _toolbarView;

    [menubarWindow setWorksWhenModal:YES];

    //    [Localizer localizeView:menubarWindow];
    //    [Localizer localizeView:_toolbarView];

    // Make main menu font size smaller
    //            NSFont* font = [NSFont menuFontOfSize:13];
    //            NSDictionary* fontAttribute = [NSDictionary dictionaryWithObjectsAndKeys:font, NSFontAttributeName, nil];
    //
    //            [_mainMenu setFont:font];
    //
    //            for (id subItem in [_mainMenu itemArray]) {
    //                if ([subItem isKindOfClass:[NSMenuItem class]]) {
    //                    NSMenuItem* menuItem = subItem;
    //                    NSString* title = [menuItem title];
    //
    //                    NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:title attributes:fontAttribute];
    //
    //                    [menuItem setAttributedTitle:attributedTitle];
    //                }
    //            }

    [(OBMenuBarWindow*)self.window setColorTheme:self.monitorEngine.configuration.colorTheme];
    [(JLNFadingScrollView *)_scrollView setFadeColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];

    [_tableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorPopupItemDataType]];
    [_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];

    [Localizer localizeView:self.window];
    [Localizer localizeView:_toolbarView];

    [self addObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.colorTheme) options:0 context:nil];
    [self addObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.showSensorLegendsInPopup) options:0 context:nil];
    [self addObserver:self forKeyPath:@keypath(self, monitorEngine.sensorsAndGroups) options:0 context:nil];

    [self layoutContent:YES orderFront:NO animated:NO];
}

-(void)showWindow:(id)sender
{
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;

    if (self.delegate && [self.delegate respondsToSelector:@selector(popupWillOpen:)]) {
        [self.delegate popupWillOpen:self];
    }

    if (!menubarWindow.attachedToMenuBar) {
        [NSApp activateIgnoringOtherApps:YES];
    }
    
    [self layoutContent:NO orderFront:YES animated:YES];

    self.statusItemView.isHighlighted = YES;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupDidOpen:)]) {
        [self.delegate popupDidOpen:self];
    }
}

-(void)close
{
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupWillClose:)]) {
        [self.delegate popupWillClose:self];
    }
    
    [self.window orderOut:nil];

    self.statusItemView.isHighlighted = NO;
    
    if (self.delegate && [self.delegate respondsToSelector:@selector(popupDidClose:)]) {
        [self.delegate popupDidClose:self];
    }
}

#pragma mark -
#pragma mark Methods

// Space betweeen last sensor cell and bottom side of a window
#define BOTTOM_SPACE  5

- (void)layoutContent:(BOOL)resizeToContent orderFront:(BOOL)orderFront animated:(BOOL)animated
{
    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;

    if (resizeToContent) {

        __block CGFloat height = 0;

        [self.sensorsAndGroupsCollectionSnapshot enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            height += [self tableView:_tableView heightOfRow:idx];
        }];

        height += BOTTOM_SPACE;

        CGFloat fullHeight = height + menubarWindow.toolbarHeight * 2;
        CGFloat screenHeight = menubarWindow.screen.visibleFrame.size.height;

        if (fullHeight > screenHeight) {
            height = screenHeight - menubarWindow.toolbarView.frame.size.height * 2 - BOTTOM_SPACE;
            [_scrollView setHasVerticalScroller:YES];
        }
        else {
            [_scrollView setHasVerticalScroller:NO];
        }

        if (animated) {
            [[_tableHeightConstraint animator] setConstant:height];
        }
        else {
            [_tableHeightConstraint setConstant:height];
        }
    }
    
    // Order front if needed
    if (orderFront) {
        [menubarWindow makeKeyAndOrderFront:self];
        [self.window setBackgroundBlurRadius:4];
    }
}

-(void)reloadSensorsTableView:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        NSArray *oldSensorsAndGroups = [_sensorsAndGroupsCollectionSnapshot copy];

        _sensorsAndGroupsCollectionSnapshot = nil;

        [_tableView updateWithObjectValues:self.sensorsAndGroupsCollectionSnapshot previousObjectValues:oldSensorsAndGroups];

        [self layoutContent:YES orderFront:NO animated:YES];
    }];

}

#pragma mark -
#pragma mark Actions

- (void)togglePanel:(id)sender
{
    OBMenuBarWindow* menubarWindow = (OBMenuBarWindow*)self.window;
    
    if (menubarWindow)
    {
        if (menubarWindow.isVisible && (menubarWindow.isKeyWindow || menubarWindow.attachedToMenuBar))
        {
            [self close];
        }
        else
        {
            [self showWindow:self];
        }
    }
}

- (void)showAboutPanel:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [_aboutController showWindow:sender];
    }];
}

- (void)openPreferences:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [_appController showWindow:sender];
    }];
}

- (void)showGraphsWindow:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [_graphsController showWindow:sender];
    }];
}

#pragma mark -
#pragma mark Events

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@keypath(self, monitorEngine.configuration.colorTheme)]) {

        [(OBMenuBarWindow*)self.window setColorTheme:self.monitorEngine.configuration.colorTheme];
        [(JLNFadingScrollView *)_scrollView setFadeColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];

    }
    else if ([keyPath isEqual:@keypath(self, monitorEngine.configuration.showSensorLegendsInPopup)] ||
             [keyPath isEqual:@keypath(self, monitorEngine.sensorsAndGroups)]) {

        [self reloadSensorsTableView:self];

    }
}

- (void)windowDidAttachToStatusBar:(id)sender
{
//    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;
//
//    [menubarWindow setMaxSize:NSMakeSize(menubarWindow.maxSize.width, menubarWindow.frame.size.height)];
//    [menubarWindow setMinSize:NSMakeSize(menubarWindow.minSize.width, menubarWindow.toolbarHeight + BOTTOM_SPACE)];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(NSEC_PER_SEC / 7)), dispatch_get_main_queue(), ^{
        [self layoutContent:YES orderFront:NO animated:YES];
    });
}

- (void)windowDidDetachFromStatusBar:(id)sender
{
//    OBMenuBarWindow *menubarWindow = (OBMenuBarWindow *)self.window;
//
//    [menubarWindow setMaxSize:NSMakeSize(menubarWindow.maxSize.width, menubarWindow.frame.size.height)];
//    [menubarWindow setMinSize:NSMakeSize(menubarWindow.minSize.width, menubarWindow.toolbarHeight + BOTTOM_SPACE)];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(NSEC_PER_SEC / 7)), dispatch_get_main_queue(), ^{
        [NSApp activateIgnoringOtherApps:YES];
        [self layoutContent:YES orderFront:YES animated:YES];
    });
}

- (void)windowDidBecomeKey:(id)sender
{
    for (id subveiw in _toolbarView.subviews)
    {
        if ([subveiw respondsToSelector:@selector(setEnabled:)]) {
            [subveiw setEnabled:YES];
        }
    }
}

- (void)windowDidResignKey:(id)sender
{
    for (id subveiw in _toolbarView.subviews)
    {
        if ([subveiw respondsToSelector:@selector(setEnabled:)]) {
            [subveiw setEnabled:NO];
        }
    }
}

#pragma mark -
#pragma mark NSTableView delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return self.sensorsAndGroupsCollectionSnapshot.count;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    HWMItem *item = [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:row];

    NSUInteger height = [item isKindOfClass:[HWMSensorsGroup class]] ? 21 : 17;

    if (item.legend && self.monitorEngine.configuration.showSensorLegendsInPopup.boolValue)
        height += 10;

    return height;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

-(id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:row];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    HWMItem *item = [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:row];

    id view = [tableView makeViewWithIdentifier:item.identifier owner:self];

    return view;
}

//- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
//{
//    id item = [_items objectAtIndex:row];
//    
//    if ([item isKindOfClass:[HWMonitorGroup class]]) {
//        return _hasScroller;
//    }
//    
//    return NO;
//}

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    if (self.tableView != tableView) {
        return NO;
    }
    
    id item = [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:[rowIndexes firstIndex]];
    
    if ([item isKindOfClass:[HWMSensor class]]) {
        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorPopupItemDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorPopupItemDataType];

        [NSApp activateIgnoringOtherApps:YES];

        return YES;
    }

    return NO;
}

-(void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
    if (tableView == _tableView && (operation == NSDragOperationDelete || _currentItemDragOperation == NSDragOperationDelete))
    {
        NSPasteboard* pboard = [session draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorPopupItemDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];
        id fromItem = [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:fromRow];

        [(HWMItem*)fromItem setHidden:@YES];

        NSShowAnimationEffect(NSAnimationEffectPoof, screenPoint, NSZeroSize, nil, nil, nil);
    }
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)toRow proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (_tableView != tableView || [info draggingSource] != _tableView) {
        return NO;
    }
    
    [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];
    
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorPopupItemDataType];
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    NSInteger fromRow = [rowIndexes firstIndex];
    id fromItem = [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:fromRow];

    _currentItemDragOperation = NSDragOperationNone;
    
    if ([fromItem isKindOfClass:[HWMSensor class]] && toRow > 0) {
        
        _currentItemDragOperation = NSDragOperationMove;

        if (toRow < self.sensorsAndGroupsCollectionSnapshot.count) {
            if (toRow == fromRow || toRow == fromRow + 1) {
                _currentItemDragOperation = NSDragOperationNone;
            }
            else {
                id toItem = [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:toRow];

                if ([toItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)fromItem group] != [(HWMSensor*)toItem group]) {
                    _currentItemDragOperation = NSDragOperationNone;
                }
            }
        }
        else {
            id toItem = [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:toRow - 1];

            if ([toItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)fromItem group] != [(HWMSensor*)toItem group]) {
                _currentItemDragOperation = NSDragOperationNone;
            }
        }
    }
    
    return _currentItemDragOperation;
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (self.tableView != tableView) {
        return NO;
    }
    
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorPopupItemDataType];
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    NSInteger fromRow = [rowIndexes firstIndex];

    HWMSensor *fromItem = [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:fromRow];
    
    id checkItem = toRow >= self.sensorsAndGroupsCollectionSnapshot.count ? [self.sensorsAndGroupsCollectionSnapshot lastObject] : [self.sensorsAndGroupsCollectionSnapshot objectAtIndex:toRow];
    
    HWMSensor *toItem = ![checkItem isKindOfClass:[HWMSensor class]] 
    || toRow >= self.sensorsAndGroupsCollectionSnapshot.count ? nil : checkItem;

    [fromItem.group moveSensorsObjectAtIndex:[fromItem.group.sensors indexOfObject:fromItem] toIndex: toItem ? [fromItem.group.sensors indexOfObject:toItem] : fromItem.group.sensors.count];

    [self.monitorEngine setNeedsUpdateSensorLists];

    return YES;
}

@end
