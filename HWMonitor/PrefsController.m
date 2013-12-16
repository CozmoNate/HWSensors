//
//  AppDelegate.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
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

#import "PrefsController.h"
#import "HWMonitorDefinitions.h"

#import "PopupGroupCell.h"
#import "PrefsCell.h"

#import "Localizer.h"

#import "HWMConfiguration.h"
#import "HWMSensorsGroup.h"
#import "HWMItem.h"
#import "HWMIcon.h"
#import "HWMSensor.h"
#import "HWMBatterySensor.h"

@implementation PrefsController

#pragma mark
#pragma mark Properties:

-(NSMutableArray *)themes
{
    if (!_themes) {
        
        _themes = [[NSMutableArray alloc] init];
        
        [self.monitorEngine.configuration.colorThemes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSString *path = [NSString stringWithFormat:@"theme_%@", [obj name]];
            NSImage *preview = [NSImage imageNamed:[path lowercaseString]];
            
            [_themes addObject:@{@"name"    : GetLocalizedString([obj name]),
                                 @"preview" : preview}];
        }];
    }
    
    return _themes;
}

-(NSMutableIndexSet *)themeSelectionIndexes
{
    return [[NSMutableIndexSet alloc] initWithIndex:self.monitorEngine.configuration.colorThemeIndex.unsignedIntegerValue];
}

-(void)setThemeSelectionIndexes:(NSMutableIndexSet *)themeSelectionIndexes
{
    NSUInteger index = [themeSelectionIndexes firstIndex];
    
    if (index > [self.monitorEngine.configuration.colorThemes count]) {
        index = 0;
    }
    
    [self.monitorEngine.configuration setColorThemeIndex:[NSNumber numberWithUnsignedInteger:index]];
}

#pragma mark
#pragma mark Overrides:

- (id)init
{
    self = [super initWithWindowNibName:@"PrefsController"];
    
    if (self != nil)
    {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            
            [Localizer localizeView:self.window];
            [Localizer localizeView:_generalPrefsView];
            [Localizer localizeView:_menubarPrefsView];
            [Localizer localizeView:_popupPrefsView];
            [Localizer localizeView:_graphsPrefsView];
            
            [_favoritesTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorPrefsItemDataType]];
            [_favoritesTableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];
            [_sensorsTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorPrefsItemDataType]];
            [_sensorsTableView setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];
            
            _previousViewTag = -1;
            [self switchView:0];
            [self.window.toolbar setSelectedItemIdentifier:@"General"];
            
            [[self.window standardWindowButton:NSWindowZoomButton] setEnabled:NO];
            
            [self addObserver:self forKeyPath:@"monitorEngine.configuration.useFahrenheit" options:NSKeyValueObservingOptionNew context:nil];
            [self addObserver:self forKeyPath:@"monitorEngine.favoriteItems" options:NSKeyValueObservingOptionNew context:nil];
            [self addObserver:self forKeyPath:@"monitorEngine.iconsWithSensorsAndGroups" options:NSKeyValueObservingOptionNew context:nil];
        }];
    }
    
    return self;
}

-(void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [super showWindow:sender];
    
    [self.monitorEngine updateSmcAndDevicesSensors];
}


#pragma mark
#pragma mark Methods:

-(void)checkForUpdates:(id)sender
{
    if ([sender isKindOfClass:[NSButton class]]) {
        NSButton *button = (NSButton*)sender;
        
        if ([button state]) {
            _sharedUpdater.automaticallyChecksForUpdates = YES;
            [_sharedUpdater checkForUpdatesInBackground];
        }
    }
    else {
        [_sharedUpdater checkForUpdates:sender];
    }
}

-(void)rebuildSensorsList:(id)sender
{
    [_monitorEngine rebuildSensorsList];
}

#pragma mark
#pragma mark Events:

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"monitorEngine.configuration.useFahrenheit"]) {
        [_monitorEngine setNeedsRecalculateSensorValues];
    }
    else if ([keyPath isEqual:@"monitorEngine.favoriteItems"]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [_favoritesTableView reloadData];
        });
    }
    else if ([keyPath isEqual:@"monitorEngine.iconsWithSensorsAndGroups"]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [_sensorsTableView reloadData];
        });
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //[_colorThemeMatrix selectCellAtRow:_monitorEngine.configuration.colorThemeIndex.integerValue column:0];
    [_monitorEngine startEngine];
}

-(void)awakeFromNib
{
    
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    [_monitorEngine stopEngine];
    
    [self removeObserver:self forKeyPath:@"monitorEngine.configuration.useFahrenheit"];
    [self removeObserver:self forKeyPath:@"monitorEngine.favoriteItems"];
    [self removeObserver:self forKeyPath:@"monitorEngine.iconsWithSensorsAndGroups"];
}

- (IBAction)sensorHiddenFlagChanged:(id)sender
{
    [self.monitorEngine setNeedsUpdateLists];
}

#pragma mark
#pragma mark Toolbar 

// Got from http://ccoding.co.de/CCoding/Gifts/NSToolbarViews.zip

- (NSRect)newFrameForNewContentView:(NSView*)view {
    
    NSWindow *window = [self window];
    NSRect newFrameRect = [window frameRectForContentRect:[view frame]];
    NSRect oldFrameRect = [window frame];
    NSSize newSize = newFrameRect.size;
    NSSize oldSize = oldFrameRect.size;
    
    NSRect frame = [window frame];
    frame.size = newSize;
    frame.origin.y -= (newSize.height - oldSize.height);
    
    return frame;
}

- (NSView *)viewForTag:(NSInteger)tag {
    
    NSView *view = nil;
    
    switch (tag) {
        case 0:
            view = _generalPrefsView;
            break;
        case 1:
            view = _menubarPrefsView;
            break;
        case 2:
            view = _popupPrefsView;
            break;
        case 3: 
            view = _graphsPrefsView; 
            break;
            
        default:
            view = nil;
            break;
    }
    
    return  view;
}

- (IBAction)switchView:(id)sender {
    
    NSInteger tag = [sender tag];
    
    if (_previousViewTag == tag) {
        return;
    }
    
    __block NSView *view = [self viewForTag:tag];
    NSView *previousView = [self viewForTag:_previousViewTag];
    
    _previousViewTag = tag;
    
    //[view layoutSubtreeIfNeeded];
    
    NSRect newFrame = [self newFrameForNewContentView:view];
    
    if (previousView) {
        
        newFrame.origin.x += ([[self window] frame].size.width - newFrame.size.width) / 2.0f;
        
        [previousView setAlphaValue:0.0];
        
        if (previousView.superview) {
            [previousView removeFromSuperview];
        }
    }
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [context setDuration:[[NSApp currentEvent] modifierFlags] & NSShiftKeyMask ? 1.0 : 0.2];
        [[[self window] animator] setFrame:newFrame display:YES];
    } completionHandler:^{
        if (view == _menubarPrefsView /*|| 
                                       view == _popupPrefsView*/) {
                                           //[[self.window standardWindowButton:NSWindowZoomButton] setEnabled:YES];
                                           [self.window setMinSize:NSZeroSize];
                                           [self.window setMaxSize:NSMakeSize(MAXFLOAT, MAXFLOAT)];
                                       }
        else {
            //[[self.window standardWindowButton:NSWindowZoomButton] setEnabled:NO];
            [self.window setMinSize:newFrame.size];
            [self.window setMaxSize:newFrame.size];
        }
        
        [[[self window] contentView] addSubview:view];
        [NSAnimationContext beginGrouping];
        [[view animator] setAlphaValue:1.0];
        [NSAnimationContext endGrouping];
    }];    
}
#pragma mark
#pragma mark HWMEngineDelegate:

- (HWMSensorsUpdateLoopStrategy)updateLoopStrategyForEngine:(HWMEngine*)engine
{
    if (self.window.isVisible || _graphsController.window.isVisible) {
        return kHWMSensorsUpdateLoopForced;
    }
    else if (_popupController.window.isVisible) {
        return kHWMSensorsUpdateLoopRegular;
    }
    
    return kHWMSensorsUpdateLoopOnlyFavorites;
}

-(BOOL)engine:(HWMEngine *)engine shouldCaptureSensorValuesToGaphsHistoryWithLimit:(NSUInteger *)limit
{
    *limit = (NSUInteger)_graphsController.graphsCollectionView.frame.size.width / _monitorEngine.configuration.graphsScaleValue.floatValue;
    
    return _graphsController.window.isVisible || _monitorEngine.configuration.updateSensorsInBackground.boolValue;
}

#pragma mark
#pragma mark PopupControllerDelegate:

- (void) popupDidOpen:(id)sender
{
    [self.monitorEngine updateSmcAndDevicesSensors];
}

#pragma mark
#pragma mark  NSTableViewDelegate:

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (tableView == _favoritesTableView) {
        return _monitorEngine.favoriteItems.count + 1;
    }
    else if (tableView == _sensorsTableView) {
        return _monitorEngine.iconsWithSensorsAndGroups.count + 1;
    }
    
    return 0;
}

-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    if (tableView == _favoritesTableView && row > 0 && [[_monitorEngine.favoriteItems objectAtIndex:row - 1] isKindOfClass:[HWMBatterySensor class]]) {
        return  29;
    }
    else if (tableView == _sensorsTableView && row > 0 && [[_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:row - 1] isKindOfClass:[HWMBatterySensor class]]) {
        return  29;
    }
    
    return 19;
}

-(BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
    if (tableView == _favoritesTableView) {
        if (row == 0) {
            return YES;
        }
        else {
            return NO;
        }
    }
    else if (tableView == _sensorsTableView) {
        if (row == 0) {
            return YES;
        }
        else {
            return [[_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:row - 1] isKindOfClass:[HWMSensorsGroup class]];
        }
    }
    
    return NO;
}

-(id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if (tableView == _favoritesTableView) {
        if (row == 0) {
            return nil;
        }
        return [_monitorEngine.favoriteItems objectAtIndex:row - 1];
    }
    else {
        if (row == 0) {
            return nil;
        }
        return [_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:row - 1];
    }
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSView *view = nil;
    
    if (tableView == _favoritesTableView) {
        if (row == 0) {
            PopupGroupCell *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];
            
            [[groupCell textField] setStringValue:GetLocalizedString(@"Menubar items")];
            
            view = groupCell;
        }
        else {
            HWMItem *item = [[_monitorEngine favoriteItems] objectAtIndex:row - 1];
            view = [tableView makeViewWithIdentifier:item.identifier owner:self];
        }
    }
    else if (tableView == _sensorsTableView) {
        if (row == 0) {
            PopupGroupCell *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];
            
            [[groupCell textField] setStringValue:GetLocalizedString(@"Icons")];
            
            view = groupCell;
        }
        else {
            HWMItem *item = [_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:row - 1];
            view = [tableView makeViewWithIdentifier:item.identifier owner:self];
        }
    }
    
    [Localizer localizeView:view];
    
    return view;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    if (tableView == _favoritesTableView) {
        
        if ([rowIndexes firstIndex] == 0) {
            return NO;
        }
        
        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorPrefsItemDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorPrefsItemDataType];
        
        _hasDraggedFavoriteItem = YES;
    }
    else if (tableView == _sensorsTableView) {
        
        if ([rowIndexes firstIndex] == 0) {
            return NO;
        }
        
        id item = [_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:[rowIndexes firstIndex] - 1];
        
        if ([item isKindOfClass:[HWMSensorsGroup class]]) {
            return NO;
        }
        
        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorPrefsItemDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorPrefsItemDataType];
        
        _hasDraggedFavoriteItem = NO;
    }
    
    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)toRow proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    _currentItemDragOperation = NSDragOperationNone;
    
    if (tableView == _favoritesTableView) {
        
        [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];
        
        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorPrefsItemDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];
        
        if ([info draggingSource] == _favoritesTableView) {
            _currentItemDragOperation = toRow < 1 || toRow == fromRow || toRow == fromRow + 1 ? NSDragOperationNone : NSDragOperationMove;
        }
        else if ([info draggingSource] == _sensorsTableView) {
            id item = [_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:fromRow - 1];
            
            if ([item isKindOfClass:[HWMSensor class]]) {
                _currentItemDragOperation = [[(HWMSensor*)item favorites] count] ? NSDragOperationPrivate : toRow > 0  ? NSDragOperationCopy : NSDragOperationNone;
            }
            else _currentItemDragOperation = toRow > 0 ? NSDragOperationCopy : NSDragOperationNone;
        }
    }
    else if (tableView == _sensorsTableView && [info draggingSource] == _sensorsTableView) {
        
        [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];
        
        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorPrefsItemDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex] - 1;
        toRow -= 1;
        id sourceItem = [self.monitorEngine.iconsWithSensorsAndGroups objectAtIndex:fromRow];
        
        _currentItemDragOperation = NSDragOperationNone;
        
        if ([sourceItem isKindOfClass:[HWMSensor class]] && toRow > 0) {
            
            _currentItemDragOperation = NSDragOperationMove;
            
            if (toRow < self.monitorEngine.iconsWithSensorsAndGroups.count) {
                
                if (toRow == fromRow || toRow == fromRow + 1) {
                    _currentItemDragOperation = NSDragOperationNone;
                }
                else {
                    id destinationItem = [self.monitorEngine.iconsWithSensorsAndGroups objectAtIndex:toRow];
                    
                    if ([destinationItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)sourceItem group] != [(HWMSensor*)destinationItem group]) {
                        _currentItemDragOperation = NSDragOperationNone;
                    }
                    else {
                        destinationItem = [self.monitorEngine.iconsWithSensorsAndGroups objectAtIndex:toRow - 1];
                        
                        if ([destinationItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)sourceItem group] != [(HWMSensor*)destinationItem group]) {
                            _currentItemDragOperation = NSDragOperationNone;
                        }
                    }
                }
            }
            else {
                id destinationItem = [self.monitorEngine.iconsWithSensorsAndGroups objectAtIndex:toRow - 1];
                
                if ([destinationItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)sourceItem group] != [(HWMSensor*)destinationItem group]) {
                    _currentItemDragOperation = NSDragOperationNone;
                }
            }
        }
    }
    
    return _currentItemDragOperation;
}

-(void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
    if (tableView == _favoritesTableView && (operation == NSDragOperationDelete || _currentItemDragOperation == NSDragOperationDelete))
    {
        NSPasteboard* pboard = [session draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorPrefsItemDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        
        [_monitorEngine removeItemFromFavoritesAtIndex:[rowIndexes firstIndex] - 1];
        
        NSShowAnimationEffect(NSAnimationEffectPoof, screenPoint, NSZeroSize, nil, nil, nil);
    }
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (tableView == _favoritesTableView) {
        
        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorPrefsItemDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];
        
        if ([info draggingSource] == _favoritesTableView) {
            
            toRow = toRow > fromRow ? toRow - 1 : toRow;
            
            [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
                [tableView moveRowAtIndex:fromRow toIndex:toRow];
            } completionHandler:^{
                [_monitorEngine moveFavoritesItemAtIndex:fromRow - 1 toIndex:toRow > 0 ? toRow - 1 : 0];
            }];
        }
        else  if ([info draggingSource] == _sensorsTableView) {
            HWMItem *item = [_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:fromRow - 1];
            [_monitorEngine insertItemIntoFavorites:item atIndex:toRow > 0 ? toRow - 1 : 0];
        }
    }
    else if (tableView == _sensorsTableView && [info draggingSource] == _sensorsTableView) {
        
        NSInteger destinationRow = toRow - 1;
        
        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorPrefsItemDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger sourceRow = [rowIndexes firstIndex] - 1;
        
        HWMSensor *sourceItem = [self.monitorEngine.iconsWithSensorsAndGroups objectAtIndex:sourceRow];
        HWMSensor *destinationItem = [self.monitorEngine.iconsWithSensorsAndGroups objectAtIndex:destinationRow];
        
        destinationRow = destinationRow > sourceRow ? destinationRow - 1 : destinationRow;
        
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [tableView moveRowAtIndex:sourceRow + 1 toIndex:destinationRow + 1];
            [sourceItem.group moveSensorsObject:sourceItem toIndex:[sourceItem.group.sensors indexOfObject:destinationItem]];
        } completionHandler:^{
            [_monitorEngine setNeedsUpdateLists];
        }];
    }
    
    return YES;
}

@end
