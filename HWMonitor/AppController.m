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
#import "HWMGroup.h"
#import "HWMItem.h"

@interface AppController (Private)

@property (readonly) BOOL shouldUpdateOnlyFavoritesSensors;

@end

@implementation AppController

#pragma mark
#pragma mark Properties:

- (BOOL)shouldUpdateOnlyFavoritesSensors
{
    return !([self.window isVisible] || [_popupController.window isVisible] || [_graphsController.window isVisible] || [_graphsController backgroundMonitoring]);
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
            [self initialSetup];
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

- (void)initialSetup
{
    [Localizer localizeView:self.window];

    [_favoritesTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
    [_favoritesTableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];
    [_sensorsTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
    [_sensorsTableView setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];
}

- (void)rebuildSensorsTableView
{
    [_favoritesTableView reloadData];
    [_sensorsTableView reloadData];
}

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

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{

}

-(void)awakeFromNib
{
    }

-(void)applicationWillTerminate:(NSNotification *)notification
{

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

- (IBAction)colorThemeChanged:(id)sender
{
    [self.monitorEngine.configuration setColorTheme:[self.monitorEngine getColorThemeByIndex:[sender selectedRow]]];
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
        return _monitorEngine.favoriteItems.count + 1;
    }

    return _monitorEngine.availableItems.count;
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
        return [[_monitorEngine.availableItems objectAtIndex:row] isKindOfClass:[HWMGroup class]];
    }
    
    return NO;
}

-(id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if (tableView == _favoritesTableView) {
        return nil; // TODO: implement favorites
    }
    else {
        return [_monitorEngine.availableItems objectAtIndex:row];
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
        else {
//            id item = [_favorites objectAtIndex:row - 1];
//            
//            if ([item isKindOfClass:[HWMonitorSensor class]]) {
//                HWMonitorSensor *sensor = (HWMonitorSensor*)item;
//                
//                PrefsSensorCell *itemCell = [tableView makeViewWithIdentifier:@"Sensor" owner:self];;
//                
//                [itemCell.imageView setImage:[[self getIconByGroup:[sensor group]] image]];
//                [itemCell.textField setStringValue:[sensor title]];
//                [itemCell.valueField setStringValue:[sensor stringValue]];
//                
//                return itemCell;
//            }
//            else if ([item isKindOfClass:[HWMonitorIcon class]]) {
//                PrefsSensorCell *iconCell = [tableView makeViewWithIdentifier:@"Icon" owner:self];
//                
//                [[iconCell imageView] setObjectValue:[item image]];
//                [[iconCell textField] setStringValue:GetLocalizedString([item name])];
//                
//                return iconCell;
//            }
            // TODO: implement favorites
        }
    }
    else if (tableView == _sensorsTableView) {

        HWMItem *item = [_monitorEngine.availableItems objectAtIndex:row];
        id view = [tableView makeViewWithIdentifier:item.identifier owner:self];

        return view;
    }

    return nil;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    // TODO: implement favorites
//    if (tableView == _favoritesTableView) {
//        if ([rowIndexes firstIndex] == 0) {
//            return NO;
//        }
//        
//        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
//        
//        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorTableViewDataType, nil] owner:self];
//        [pboard setData:indexData forType:kHWMonitorTableViewDataType];
//        
//        _hasDraggedFavoriteItem = YES;
//    }
//    else if (tableView == _sensorsTableView) {
//        id item = [self getItemAtIndex:[rowIndexes firstIndex]];
//        
//        if ([item isKindOfClass:[NSString class]]) {
//            return NO;
//        }
////        else if ([item isKindOfClass:[HWMonitorItem class]] && [_favorites containsObject:[item sensor]]) {
////            //return NO;
////            _currentItemDragOperation = NSDragOperationPrivate;
////        }
//        
//        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
//        
//        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorTableViewDataType, NSStringPboardType, nil] owner:self];
//        
//        [pboard setData:indexData forType:kHWMonitorTableViewDataType];
//        [pboard setString:[_ordering objectAtIndex:[rowIndexes firstIndex]] forType:NSStringPboardType];
//        
//        _hasDraggedFavoriteItem = NO;
//    }

    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)toRow proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    // TODO: implement favorites
//    //_currentItemDragOperation = NSDragOperationNone;
//    
//    if (tableView == _favoritesTableView) {
//        [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];
//        
//        NSPasteboard* pboard = [info draggingPasteboard];
//        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
//        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
//        NSInteger fromRow = [rowIndexes firstIndex];
//        
//        if ([info draggingSource] == _favoritesTableView) {
//            _currentItemDragOperation = toRow < 1 || toRow == fromRow || toRow == fromRow + 1 ? NSDragOperationNone : NSDragOperationMove;
//        }
//        else if ([info draggingSource] == _sensorsTableView) {
//            id item = [self getItemAtIndex:fromRow];
//            
//            if ([item isKindOfClass:[HWMonitorItem class]]) {
//                _currentItemDragOperation = [_favorites containsObject:[item sensor]] ? NSDragOperationPrivate : toRow > 0  ? NSDragOperationCopy : NSDragOperationNone;
//            }
//            else _currentItemDragOperation = toRow > 0 ? NSDragOperationCopy : NSDragOperationNone;
//        }
//    }
//    else if (tableView == _sensorsTableView) {
//        _currentItemDragOperation = NSDragOperationNone;
//    }

    return _currentItemDragOperation;
}

-(void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
    // TODO: implement favorites
//    if (tableView == _favoritesTableView && (operation == NSDragOperationDelete || _currentItemDragOperation == NSDragOperationDelete))
//    {
//        NSPasteboard* pboard = [session draggingPasteboard];
//        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
//        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
//
//        [_favorites removeObjectAtIndex:[rowIndexes firstIndex] - 1];
//        
//        [self favoritesChanged:tableView];
//        
//        NSShowAnimationEffect(NSAnimationEffectPoof, screenPoint, NSZeroSize, nil, nil, nil);
//    }
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    // TODO: implement favorites
//    if (tableView == _favoritesTableView) {
//        
//        NSPasteboard* pboard = [info draggingPasteboard];
//        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
//        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
//        NSInteger fromRow = [rowIndexes firstIndex];
//        
//        if ([info draggingSource] == _sensorsTableView) {
//            id item = [self getItemAtIndex:fromRow];
//            
//            if ([item isKindOfClass:[HWMonitorItem class]]) {
//                if (![_favorites containsObject:[item sensor]]) {
//                    [_favorites insertObject:[item sensor] atIndex:toRow > 0 ? toRow - 1 : 0];
//                }
//            }
//            else if ([item isKindOfClass:[HWMonitorIcon class]]) {
//                [_favorites insertObject:item  atIndex:toRow > 0 ? toRow - 1 : 0];
//            }
//        }
//        else if ([info draggingSource] == _favoritesTableView) {
//            id item = [_favorites objectAtIndex:fromRow - 1];
//            
//            [_favorites insertObject:item atIndex:toRow > 0 ? toRow - 1 : 0];
//            [_favorites removeObjectAtIndex:fromRow > toRow ? fromRow : fromRow - 1];
//        }
//    
//        [self favoritesChanged:tableView];
//    }

    return YES;
}

@end
