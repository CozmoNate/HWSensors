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
#import "HWMEngineHelper.h"

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
    
    [self.monitorEngine updateSmcAndDeviceSensors];
}

-(NSDictionary *)registrationDictionaryForGrowl
{
    NSDictionary *allNotifications = @{
                                       NotifierSensorLevelExceededNotification:        NotifierSensorLevelExceededHumanReadableDescription,
                                       NotifierSensorLevelHighNotification: NotifierSensorLevelHighHumanReadableDescription,
                                       NotifierSensorLevelModerateNotification:NotifierSensorLevelModerateHumanReadableDescription
                                       };

    NSMutableDictionary *defaultNotifications = [allNotifications mutableCopy];

    [defaultNotifications removeObjectForKey:NotifierSensorLevelModerateNotification];

    return @{GROWL_APP_NAME : @"HWMonitor",
             GROWL_NOTIFICATIONS_ALL: allNotifications.allKeys,
             GROWL_NOTIFICATIONS_DEFAULT: defaultNotifications.allKeys,
             GROWL_NOTIFICATIONS_HUMAN_READABLE_NAMES: allNotifications,
             };
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

-(void)reloadFavoritesTableView:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        NSMutableArray *newFavorites = [[self.monitorEngine.configuration.favorites array] mutableCopy];

        if (_favoritesCollectionSnapshot) {
            [newFavorites insertObject:[_favoritesCollectionSnapshot objectAtIndex:0] atIndex:0];
        }
        else {
            NSDictionary *item = @{@"title" : [GetLocalizedString(@"Menubar items") uppercaseString],
                                   @"identifier" : @"Group"};
            [newFavorites insertObject:@{@"item" : item} atIndex:0];
        }

        NSIndexSet *inserted, *removed, *from, *to;

        [HWMEngineHelper compareItemsList:_favoritesCollectionSnapshot toItemsList:newFavorites additions:&inserted deletions:&removed movedFrom:&from movedTo:&to];

        _favoritesCollectionSnapshot = newFavorites;

        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
            [_favoritesTableView removeRowsAtIndexes:removed withAnimation:NSTableViewAnimationSlideUp];
            [_favoritesTableView insertRowsAtIndexes:inserted withAnimation:NSTableViewAnimationSlideDown];

            while (from.count) {
                [_favoritesTableView moveRowAtIndex:from.firstIndex toIndex:to.firstIndex];
                [(NSMutableIndexSet*)from removeIndex:from.firstIndex];
                [(NSMutableIndexSet*)to removeIndex:to.firstIndex];
            }

        } completionHandler:^{
            //[_favoritesTableView reloadData];
        }];
    }];
}

-(void)reloadIconsAndSensorsTableView:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        NSMutableArray *newSensorsAndGroups = [self.monitorEngine.iconsWithSensorsAndGroups mutableCopy];

        if (_iconsAndSensorsCollectionSnapshot) {
            [newSensorsAndGroups insertObject:[_iconsAndSensorsCollectionSnapshot objectAtIndex:0]
                                      atIndex:0];
        }
        else {
            [newSensorsAndGroups insertObject:@{@"title" :
                                                    [GetLocalizedString(@"Icons") uppercaseString],
                                                @"identifier" :
                                                    @"Group"}
                                      atIndex:0];
        }

        NSIndexSet *inserted, *removed, *from, *to;

        [HWMEngineHelper compareItemsList:_iconsAndSensorsCollectionSnapshot toItemsList:newSensorsAndGroups additions:&inserted deletions:&removed movedFrom:&from movedTo:&to];

        _iconsAndSensorsCollectionSnapshot = newSensorsAndGroups;

        [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {

            [_sensorsTableView removeRowsAtIndexes:removed withAnimation:NSTableViewAnimationSlideUp];
            [_sensorsTableView insertRowsAtIndexes:inserted withAnimation:NSTableViewAnimationSlideDown];

            while (from.count) {
                [_sensorsTableView moveRowAtIndex:from.firstIndex toIndex:to.firstIndex];
                [(NSMutableIndexSet*)from removeIndex:from.firstIndex];
                [(NSMutableIndexSet*)to removeIndex:to.firstIndex];
            }

        } completionHandler:^{
            //[_sensorsTableView reloadData];
        }];
    }];
}

#pragma mark
#pragma mark Events:

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"monitorEngine.favoriteItems"]) {
        [self reloadFavoritesTableView:self];
    }
    else if ([keyPath isEqual:@"monitorEngine.iconsWithSensorsAndGroups"]) {
        [self reloadIconsAndSensorsTableView:self];
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [GrowlApplicationBridge setGrowlDelegate:self];

    [_monitorEngine startEngine];

    _forceUpdateSensors = YES;
    [_monitorEngine updateSmcAndDeviceSensors];
    [_monitorEngine updateAtaSmartSensors];
    _forceUpdateSensors = NO;

    [self performSelector:@selector(reloadFavoritesTableView:) withObject:self afterDelay:0.0];
    [self performSelector:@selector(reloadIconsAndSensorsTableView:) withObject:self afterDelay:0.0];
}

-(void)awakeFromNib
{
    
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    [_monitorEngine stopEngine];
    
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
    if (_forceUpdateSensors || self.window.isVisible || _graphsController.window.isVisible) {
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
    [self.monitorEngine updateSmcAndDeviceSensors];
}

#pragma mark
#pragma mark  NSTableViewDelegate:

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (tableView == _favoritesTableView) {
        return _favoritesCollectionSnapshot.count;
    }
    else if (tableView == _sensorsTableView) {
        return _iconsAndSensorsCollectionSnapshot.count;
    }
    
    return 0;
}

-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 19;
}

-(BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
    if (tableView == _favoritesTableView) {
        return row == 0 ? YES : NO;
    }
    else if (tableView == _sensorsTableView) {
        return row == 0 || [[_iconsAndSensorsCollectionSnapshot objectAtIndex:row] isKindOfClass:[HWMSensorsGroup class]];
    }
    
    return NO;
}

-(id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if (tableView == _favoritesTableView) {
        return [[_favoritesCollectionSnapshot objectAtIndex:row] valueForKey:@"item"];
    }
    else if (tableView == _sensorsTableView) {
        return [_iconsAndSensorsCollectionSnapshot objectAtIndex:row];
    }

    return nil;
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSView *view = nil;
    
    if (tableView == _favoritesTableView) {
        id item = [[_favoritesCollectionSnapshot objectAtIndex:row] valueForKey:@"item"];
        view = [tableView makeViewWithIdentifier:[item valueForKey:@"identifier"] owner:self];
    }
    else if (tableView == _sensorsTableView) {
            id item = [_iconsAndSensorsCollectionSnapshot objectAtIndex:row];
        view = [tableView makeViewWithIdentifier:[item valueForKey:@"identifier"] owner:self];
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
        
        id item = [_iconsAndSensorsCollectionSnapshot objectAtIndex:[rowIndexes firstIndex]];
        
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
            id item = [_iconsAndSensorsCollectionSnapshot objectAtIndex:fromRow];
            
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
        NSInteger fromRow = [rowIndexes firstIndex];

        id fromItem = [_iconsAndSensorsCollectionSnapshot objectAtIndex:fromRow];
        
        _currentItemDragOperation = NSDragOperationNone;
        
        if ([fromItem isKindOfClass:[HWMSensor class]] && toRow > 0) {
            
            _currentItemDragOperation = NSDragOperationMove;
            
            if (toRow < _iconsAndSensorsCollectionSnapshot.count) {
                
                if (toRow == fromRow || toRow == fromRow + 1) {
                    _currentItemDragOperation = NSDragOperationNone;
                }
                else {
                    id toItem = [_iconsAndSensorsCollectionSnapshot objectAtIndex:toRow];
                    
                    if ([toItem isKindOfClass:[HWMIcon class]] || 
                        ([toItem isKindOfClass:[HWMSensorsGroup class]] && fromRow > toRow) ||
                        ([toItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)fromItem group] != [(HWMSensor*)toItem group])) {
                        _currentItemDragOperation = NSDragOperationNone;
                    }
                }
            }
            else {
                id toItem = [_iconsAndSensorsCollectionSnapshot lastObject];
                
                if ([toItem isKindOfClass:[HWMIcon class]] || 
                    ([toItem isKindOfClass:[HWMSensorsGroup class]] && fromRow > toRow) || 
                    ([toItem isKindOfClass:[HWMSensor class]] && [(HWMSensor*)fromItem group] != [(HWMSensor*)toItem group])) {
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
        __block NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];

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

        NSInteger listFromRow = fromRow - 1;
        NSInteger listToRow = toRow - 1;

        if ([info draggingSource] == _favoritesTableView) {
            [_monitorEngine moveFavoritesItemAtIndex:listFromRow toIndex:listToRow];
        }
        else  if ([info draggingSource] == _sensorsTableView) {
            HWMItem *item = [_iconsAndSensorsCollectionSnapshot objectAtIndex:fromRow];
            [_monitorEngine insertItemIntoFavorites:item atIndex:listToRow];
        }
    }
    else if (tableView == _sensorsTableView && [info draggingSource] == _sensorsTableView) {

        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorPrefsItemDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];

        HWMSensor *fromItem = [_iconsAndSensorsCollectionSnapshot objectAtIndex:fromRow];
        
        id checkItem = toRow >= _iconsAndSensorsCollectionSnapshot.count ? nil : [_iconsAndSensorsCollectionSnapshot objectAtIndex:toRow];
        
        HWMSensor *toItem = ![checkItem isKindOfClass:[HWMSensor class]] 
        || toRow >= _iconsAndSensorsCollectionSnapshot.count ? nil : checkItem;

        [fromItem.group moveSensorsObjectAtIndex:[fromItem.group.sensors indexOfObject:fromItem] toIndex:toItem ? [fromItem.group.sensors indexOfObject:toItem] : fromItem.group.sensors.count];
        [_monitorEngine setNeedsUpdateSensorLists];
    }
    
    return YES;
}

@end
