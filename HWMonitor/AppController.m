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

#import "AppController.h"
#import "HWMonitorDefinitions.h"

#import "PopupGroupCell.h"
#import "PrefsSensorCell.h"

#import "Localizer.h"

#import "HWMConfiguration.h"
#import "HWMEngine.h"
#import "HWMSensorsGroup.h"
#import "HWMItem.h"
#import "HWMIcon.h"
#import "HWMSensor.h"

@interface AppController (Private)

@property (readonly) BOOL shouldUpdateOnlyFavoritesSensors;

@end

@implementation AppController

#pragma mark
#pragma mark Properties:

- (BOOL)shouldUpdateOnlyFavoritesSensors
{
    return !([self.window isVisible] || [_popupController.window isVisible] || [_graphsController.window isVisible] || _monitorEngine.configuration.updateSensorsInBackground.boolValue);
}

- (BOOL)shouldForceUpdateAllSensors
{
    return [self.window isVisible] || [_graphsController.window isVisible];
}

#pragma mark
#pragma mark Overrides:

- (id)init
{
    self = [super initWithWindowNibName:@"AppController"];

    if (self != nil)
    {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [Localizer localizeView:self.window];

            [_favoritesTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
            [_favoritesTableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];
            [_sensorsTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
            [_sensorsTableView setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];

            [self addObserver:self forKeyPath:@"monitorEngine.configuration.useFahrenheit" options:NSKeyValueObservingOptionNew context:nil];
            [self addObserver:self forKeyPath:@"monitorEngine.configuration.favorites" options:NSKeyValueObservingOptionNew context:nil];
            [self addObserver:self forKeyPath:@"monitorEngine.iconsWithSensorsAndGroups" options:NSKeyValueObservingOptionNew context:nil];
        }];
    }

    return self;
}

-(void)showWindow:(id)sender
{
    [self.monitorEngine setUpdateLoopStrategy:kHWMSensorsUpdateLoopForced];
    [self.monitorEngine updateSmcAndDevicesSensors];
    [self.monitorEngine setUpdateLoopStrategy:kHWMSensorsUpdateLoopRegular];

    [NSApp activateIgnoringOtherApps:YES];
    [super showWindow:sender];
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

#pragma mark
#pragma mark Events:

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"monitorEngine.configuration.useFahrenheit"]) {
        [_monitorEngine setNeedsRecalculateSensorValues];
    }
    else if ([keyPath isEqual:@"monitorEngine.configuration.favorites"]) {
        [_favoritesTableView reloadData];
    }
    else if ([keyPath isEqual:@"monitorEngine.iconsWithSensorsAndGroups"]) {
        [_sensorsTableView reloadData];
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
    [self removeObserver:self forKeyPath:@"monitorEngine.configuration.favorites"];
    [self removeObserver:self forKeyPath:@"monitorEngine.iconsWithSensorsAndGroups"];
}

-(IBAction)favoritesChanged:(id)sender
{
//    [_favoritesTableView reloadData];
//    
//    [_popupController.statusItemView setFavorites:_favorites];
//    
//    NSMutableArray *list = [[NSMutableArray alloc] init];
//    
//    for (id item in _favorites) {
//        NSString *name = nil;
//        
//        if ([item isKindOfClass:[HWMonitorIcon class]] || [item isKindOfClass:[HWMonitorSensor class]]) {
//            name = [item name];
//        }
//        else continue;
//        
//        if ([[_engine keys] objectForKey:name] || [_icons objectForKey:name]) {
//            [list addObject:name];
//        }
//    }
//    
//    [[NSUserDefaults standardUserDefaults] setObject:list forKey:kHWMonitorFavoritesList];
}

- (IBAction)sensorHiddenFlagChanged:(id)sender
{
    [self.monitorEngine setNeedsUpdateLists];
}

#pragma mark
#pragma mark PopupControllerDelegate:

- (void) popupWillOpen:(id)sender
{
    if ([self shouldForceUpdateAllSensors] && self.monitorEngine.updateLoopStrategy != kHWMSensorsUpdateLoopForced) {
        [self.monitorEngine setUpdateLoopStrategy:kHWMSensorsUpdateLoopForced];
        [self.monitorEngine updateSmcAndDevicesSensors];
    }
    else if (![self shouldForceUpdateAllSensors]) {
        [self.monitorEngine setUpdateLoopStrategy:kHWMSensorsUpdateLoopForced];
        [self.monitorEngine updateSmcAndDevicesSensors];
        [self.monitorEngine setUpdateLoopStrategy:kHWMSensorsUpdateLoopRegular];
    }
}

- (void) popupDidClose:(id)sender
{
    if ([self shouldUpdateOnlyFavoritesSensors]) {
        [self.monitorEngine setUpdateLoopStrategy:kHWMSensorsUpdateLoopOnlyFavorites];
    }
}

#pragma mark
#pragma mark  NSTableViewDelegate:

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (tableView == _favoritesTableView) {
        return _monitorEngine.configuration.favorites.count + 1;
    }
    else if (tableView == _sensorsTableView) {
        return _monitorEngine.iconsWithSensorsAndGroups.count + 1;
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
        return [_monitorEngine.configuration.favorites objectAtIndex:row - 1];
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
    if (tableView == _favoritesTableView) {
        if (row == 0) {
            PopupGroupCell *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];

            [[groupCell textField] setStringValue:GetLocalizedString(@"Menubar items")];

            return groupCell;
        }

        HWMItem *item = [_monitorEngine.configuration.favorites objectAtIndex:row - 1];
        return [tableView makeViewWithIdentifier:item.identifier owner:self];
    }
    else if (tableView == _sensorsTableView) {
        if (row == 0) {
            PopupGroupCell *groupCell = [tableView makeViewWithIdentifier:@"Group" owner:self];

            [[groupCell textField] setStringValue:GetLocalizedString(@"Icons")];

            return groupCell;
        }

        HWMItem *item = [_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:row - 1];
        return [tableView makeViewWithIdentifier:item.identifier owner:self];
    }

    return nil;
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
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorTableViewDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorTableViewDataType];
        
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
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorTableViewDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorTableViewDataType];

        _hasDraggedFavoriteItem = NO;
    }

    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)toRow proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    //_currentItemDragOperation = NSDragOperationNone;
    
    if (tableView == _favoritesTableView) {

        [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];
        
        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];
        
        if ([info draggingSource] == _favoritesTableView) {
            _currentItemDragOperation = toRow < 1 || toRow == fromRow || toRow == fromRow + 1 ? NSDragOperationNone : NSDragOperationMove;
        }
        else if ([info draggingSource] == _sensorsTableView) {
            id item = [_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:fromRow - 1];
            
            if ([item isKindOfClass:[HWMSensor class]]) {
                _currentItemDragOperation = [(HWMItem*)item favorite].boolValue ? NSDragOperationPrivate : toRow > 0  ? NSDragOperationCopy : NSDragOperationNone;
            }
            else _currentItemDragOperation = toRow > 0 ? NSDragOperationCopy : NSDragOperationNone;
        }
    }
    else if (tableView == _sensorsTableView && [info draggingSource] == _sensorsTableView) {

            [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];

            NSPasteboard* pboard = [info draggingPasteboard];
            NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
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
        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];

        [_monitorEngine removeItemFromFavoritesAtIndex:[rowIndexes firstIndex] - 1];

        NSShowAnimationEffect(NSAnimationEffectPoof, screenPoint, NSZeroSize, nil, nil, nil);
    }
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (tableView == _favoritesTableView) {

        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger fromRow = [rowIndexes firstIndex];

        if ([info draggingSource] == _favoritesTableView) {
            toRow = toRow > fromRow ? toRow - 1 : toRow;

            [tableView moveRowAtIndex:fromRow toIndex:toRow];

            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0f * NSEC_PER_SEC)), dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void){
                [_monitorEngine moveFavoritesItemAtIndex:fromRow - 1 toIndex:toRow > 0 ? toRow - 1 : 0];
            });
        }
        else  if ([info draggingSource] == _sensorsTableView) {
            HWMItem *item = [_monitorEngine.iconsWithSensorsAndGroups objectAtIndex:fromRow - 1];

//            [tableView insertRowsAtIndexes:[NSIndexSet indexSetWithIndex:toRow] withAnimation:NSTableViewAnimationEffectGap];
//
//            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0f * NSEC_PER_SEC)), dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void){
                [_monitorEngine insertItemToFavorites:item atIndex:toRow > 0 ? toRow - 1 : 0];
            //            });
        }
    }
    else if (tableView == _sensorsTableView && [info draggingSource] == _sensorsTableView) {

        NSInteger destinationRow = toRow - 1;

        NSPasteboard* pboard = [info draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        NSInteger sourceRow = [rowIndexes firstIndex] - 1;

        HWMSensor *sourceItem = [self.monitorEngine.iconsWithSensorsAndGroups objectAtIndex:sourceRow];
        HWMSensor *destinationItem = [self.monitorEngine.iconsWithSensorsAndGroups objectAtIndex:destinationRow];

        destinationRow = destinationRow > sourceRow ? destinationRow - 1 : destinationRow;

        [tableView moveRowAtIndex:sourceRow + 1 toIndex:destinationRow + 1];
        [sourceItem.group moveSensorsObject:sourceItem toIndex:[sourceItem.group.sensors indexOfObject:destinationItem]];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0f * NSEC_PER_SEC)), dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void){
            [_monitorEngine setNeedsUpdateLists];
        });
    }

    return YES;
}

@end
