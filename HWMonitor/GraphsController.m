//
//  GraphsController.m
//  HWMonitor
//
//  Created by kozlek on 24.02.13.
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

#import "GraphsController.h"

#import "GraphsView.h"
#import "GraphsSensorCell.h"
//#import "WindowFilter.h"

#import "HWMonitorDefinitions.h"

#import "JLNFadingScrollView.h"

#import "Localizer.h"

#import "HWMEngine.h"
#import "HWMGraph.h"
#import "HWMGraphsGroup.h"
#import "HWMConfiguration.h"

#import "NSTableView+HWMEngineHelper.h"

//#define GetLocalizedString(key) \
//[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

@implementation GraphsController

@synthesize selectedItem = _selectedItem;

#pragma mark
#pragma mark Properties

-(HWMonitorItem *)selectedItem
{
    if (_graphsTableView.selectedRow >= 0 && _graphsTableView.selectedRow < _graphsAndGroupsCollectionSnapshot.count) {
        id item = [_graphsAndGroupsCollectionSnapshot objectAtIndex:_graphsTableView.selectedRow];

        if (item != _selectedItem) {
            [self willChangeValueForKey:@"selectedItem"];
            _selectedItem = item;
            [self didChangeValueForKey:@"selectedItem"];
        }
    }
    else {
        [self willChangeValueForKey:@"selectedItem"];
        _selectedItem = nil;
        [self didChangeValueForKey:@"selectedItem"];
    }
    
    return _selectedItem;
}

#pragma mark
#pragma mark Methods

-(id)init
{
    self = [super initWithWindowNibName:@"GraphsController"];
    
    if (self) {

        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [Localizer localizeView:self.window];
            [self.window setLevel:_monitorEngine.configuration.graphsWindowAlwaysTopmost.boolValue ? NSFloatingWindowLevel : NSNormalWindowLevel];

            [self rebuildViews];

//            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sensorValuesHasBeenUpdated) name:HWMEngineSensorValuesHasBeenUpdatedNotification object:_monitorEngine];
            
            [_graphsTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorGraphsItemDataType]];
            [_graphsTableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];

            [self addObserver:self forKeyPath:@"monitorEngine.graphsAndGroups" options:NSKeyValueObservingOptionNew context:nil];
            [self addObserver:self forKeyPath:@"monitorEngine.configuration.graphsWindowAlwaysTopmost" options:NSKeyValueObservingOptionNew context:nil];
            [self addObserver:self forKeyPath:@"monitorEngine.configuration.useGraphSmoothing" options:NSKeyValueObservingOptionNew context:nil];
            [self addObserver:self forKeyPath:@"monitorEngine.configuration.graphsScaleValue" options:NSKeyValueObservingOptionNew context:nil];
        }];

    }
    
    return self;
}

-(void)dealloc
{
    [self removeObserver:self forKeyPath:@"monitorEngine.graphsAndGroups"];
    [self removeObserver:self forKeyPath:@"monitorEngine.configuration.graphsWindowAlwaysTopmost"];
    [self removeObserver:self forKeyPath:@"monitorEngine.configuration.useGraphSmoothing"];
    [self removeObserver:self forKeyPath:@"monitorEngine.configuration.graphsScaleValue"];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

-(void)awakeFromNib
{

}

-(void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [super showWindow:sender];
    
    [self.monitorEngine updateSmcAndDeviceSensors];
    [self.monitorEngine updateAtaSmartSensors];
}

-(void)reloadGraphsTableView:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        
        NSArray *oldGraphsAndGroups = [_graphsAndGroupsCollectionSnapshot copy];
        _graphsAndGroupsCollectionSnapshot = [self.monitorEngine.graphsAndGroups copy];

        [_graphsTableView updateWithObjectValues:_graphsAndGroupsCollectionSnapshot previousObjectValues:oldGraphsAndGroups];
    }];
}

-(void)rebuildViews
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        if (!_graphViews) {
            _graphViews = [[NSMutableArray alloc] init];
        }
        else {
            [_graphViews removeAllObjects];
        }

        for (HWMGraphsGroup *group in _monitorEngine.configuration.graphGroups) {
            if (group.graphs && group.graphs.count) {
                GraphsView *graphView = [[GraphsView alloc] init];

                [graphView setGraphsController:self];
                [graphView setGraphsGroup:group];

                [_graphViews addObject:graphView];
            }
        }

        [_graphsCollectionView setContent:_graphViews];

        [_graphsCollectionView setMinItemSize:NSMakeSize(0, 80)];
        [_graphsCollectionView setMaxItemSize:NSMakeSize(0, 0)];

        [[_graphsCollectionView content] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            [[_graphsCollectionView itemAtIndex:idx] setView:obj];
        }];
    }];
}

#pragma mark
#pragma mark Events

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (_monitorEngine) {
        if ([keyPath isEqual:@"monitorEngine.graphsAndGroups"]) {
            [self reloadGraphsTableView:self];
            [self rebuildViews];
        }
        else if ([keyPath isEqual:@"monitorEngine.configuration.graphsWindowAlwaysTopmost"]) {
            [self.window setLevel:_monitorEngine.configuration.graphsWindowAlwaysTopmost.boolValue ? NSFloatingWindowLevel : NSNormalWindowLevel];
        }
        else if ([keyPath isEqual:@"monitorEngine.configuration.useGraphSmoothing"]) {
            [self setNeedDisplayGraphs:self];
        }
        else if ([keyPath isEqual:@"monitorEngine.configuration.graphsScaleValue"]) {
            [self setNeedDisplayGraphs:self];
        }
    }
}

//-(void)sensorValuesHasBeenUpdated
//{
//    if ([self.window isVisible] || _monitorEngine.configuration.updateSensorsInBackground.boolValue) {
//        for (GraphsView *view in _graphViews) {
//            [view captureDataToHistoryNow];
//        }
//    }
//}

-(IBAction)setNeedDisplayGraphs:(id)sender
{
    for (id graphView in _graphViews) {
        [graphView setNeedsDisplay:YES];
    }
}

#pragma mark
#pragma mark NSTableView delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return _graphsAndGroupsCollectionSnapshot.count;
}

-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return [[_graphsAndGroupsCollectionSnapshot objectAtIndex:row] isKindOfClass:[HWMGraph class]] ? 19 : 21;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return ![[_graphsAndGroupsCollectionSnapshot objectAtIndex:row] isKindOfClass:[HWMGraphsGroup class]];
}

//-(BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
//{
//    return [[_items objectAtIndex:row] isKindOfClass:[NSString class]];
//}

-(id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return [_graphsAndGroupsCollectionSnapshot objectAtIndex:row];
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    id item = [_graphsAndGroupsCollectionSnapshot objectAtIndex:row];
    id view = [tableView makeViewWithIdentifier:[item identifier] owner:self];
    return view;
}

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    if (_graphsTableView != tableView) {
        return NO;
    }
    
    id item = [_graphsAndGroupsCollectionSnapshot objectAtIndex:[rowIndexes firstIndex]];
    
    if ([item isKindOfClass:[HWMGraph class]]) {
        NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
        
        [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorGraphsItemDataType, nil] owner:self];
        [pboard setData:indexData forType:kHWMonitorGraphsItemDataType];
        
        [NSApp activateIgnoringOtherApps:YES];
        
        return YES;
    }
    
    return NO;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)toRow proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (_graphsTableView != tableView || [info draggingSource] != _graphsTableView) {
        return NO;
    }
    
    [tableView setDropRow:toRow dropOperation:NSTableViewDropAbove];
    
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorGraphsItemDataType];
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    NSInteger fromRow = [rowIndexes firstIndex];
    id fromItem = [_graphsAndGroupsCollectionSnapshot objectAtIndex:fromRow];
    
    _currentItemDragOperation = NSDragOperationNone;
    
    if ([fromItem isKindOfClass:[HWMGraph class]] && toRow > 0) {
        
        _currentItemDragOperation = NSDragOperationMove;
        
        if (toRow < _graphsAndGroupsCollectionSnapshot.count) {
            
            if (toRow == fromRow || toRow == fromRow + 1) {
                _currentItemDragOperation = NSDragOperationNone;
            }
            else {
                id toItem = [_graphsAndGroupsCollectionSnapshot objectAtIndex:toRow];
                
                if ([toItem isKindOfClass:[HWMGraph class]] && [(HWMGraph*)fromItem group] != [(HWMGraph*)toItem group]) {
                    _currentItemDragOperation = NSDragOperationNone;
                }
            }
        }
        else {
            id toItem = [_graphsAndGroupsCollectionSnapshot objectAtIndex:toRow - 1];
            
            if ([toItem isKindOfClass:[HWMGraph class]] && [(HWMGraph*)fromItem group] != [(HWMGraph*)toItem group]) {
                _currentItemDragOperation = NSDragOperationNone;
            }
        }
    }
    
    return _currentItemDragOperation;
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    if (_graphsTableView != tableView) {
        return NO;
    }
    
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorGraphsItemDataType];
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    NSInteger fromRow = [rowIndexes firstIndex];
    
    HWMGraph *fromItem = [_graphsAndGroupsCollectionSnapshot objectAtIndex:fromRow];
    
    id checkItem = toRow >= _graphsAndGroupsCollectionSnapshot.count ? [_graphsAndGroupsCollectionSnapshot lastObject] : [_graphsAndGroupsCollectionSnapshot objectAtIndex:toRow];
    
    HWMGraph *toItem = ![checkItem isKindOfClass:[HWMGraph class]] || toRow >= _graphsAndGroupsCollectionSnapshot.count ? nil : checkItem;
    
    [fromItem.group moveGraphsObjectAtIndex:[fromItem.group.graphs indexOfObject:fromItem]
                                        toIndex:toItem ? [fromItem.group.graphs indexOfObject:toItem] : fromItem.group.graphs.count];

    [self.monitorEngine setNeedsUpdateGraphsList];
    
    return YES;
}

@end
