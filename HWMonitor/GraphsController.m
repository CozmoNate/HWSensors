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
#import "HWMSensor.h"
#import "HWMGraph.h"
#import "HWMGraphsGroup.h"
#import "HWMConfiguration.h"

#import "NSTableView+HWMEngineHelper.h"
#import "NSWindow+BackgroundBlur.h"

#import <ReactiveCocoa/ReactiveCocoa.h>

//#define GetLocalizedString(key) \
//[[NSBundle mainBundle] localizedStringForKey:(key) value:@"" table:nil]

@implementation GraphsController

@synthesize selectedItem = _selectedItem;
@synthesize graphsAndGroupsCollectionSnapshot = _graphsAndGroupsCollectionSnapshot;

#pragma mark
#pragma mark Properties

-(HWMEngine *)monitorEngine
{
    return [HWMEngine sharedEngine];
}

-(HWMonitorItem *)selectedItem
{
    if (_graphsTableView.selectedRow >= 0 && _graphsTableView.selectedRow < self.graphsAndGroupsCollectionSnapshot.count) {
        id item = [self.graphsAndGroupsCollectionSnapshot objectAtIndex:_graphsTableView.selectedRow];

        if (item != _selectedItem) {
            [self willChangeValueForKey:@keypath(self, selectedItem)];
            _selectedItem = item;
            [self didChangeValueForKey:@keypath(self, selectedItem)];
        }
    }
    else {
        [self willChangeValueForKey:@keypath(self, selectedItem)];
        _selectedItem = nil;
        [self didChangeValueForKey:@keypath(self, selectedItem)];
    }
    
    return _selectedItem;
}

-(NSArray *)graphsAndGroupsCollectionSnapshot
{
    if (!_graphsAndGroupsCollectionSnapshot) {
        _graphsAndGroupsCollectionSnapshot = [self.monitorEngine.graphsAndGroups copy];
    }

    return _graphsAndGroupsCollectionSnapshot;
}

#pragma mark
#pragma mark Methods

-(id)init
{
    self = [super initWithWindowNibName:NSStringFromClass([GraphsController class])];
    
    if (self) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [[RACObserve(self, monitorEngine) filter:^BOOL(id value) {
                return value != nil;
            }] subscribeNext:^(HWMEngine *engine) {
                [RACObserve(engine, graphsAndGroups)
                 subscribeNext:^(id x) {
                     [self reloadGraphsTableView:self];
                     [self rebuildViews];
                 }];

                [RACObserve(engine.configuration, graphsWindowAlwaysTopmost)
                 subscribeNext:^(id x) {
                     [self.window setLevel:self.monitorEngine.configuration.graphsWindowAlwaysTopmost.boolValue ? NSFloatingWindowLevel : NSNormalWindowLevel];
                 }];

                [RACObserve(engine.configuration, showSensorLegendsInGraphs)
                 subscribeNext:^(id x) {
                     [self reloadGraphsTableView:self];
                 }];

                [[RACSignal combineLatest:@[RACObserve(engine.configuration, useFahrenheit),
                                            RACObserve(engine.configuration, useGraphSmoothing),
                                            RACObserve(engine.configuration, graphsScaleValue)]]
                 subscribeNext:^(id x) {
                     [self setNeedDisplayGraphs:self];
                 }];
            }];
        }];
    }
    
    return self;
}

-(void)windowDidLoad
{
    [super windowDidLoad];

    [Localizer localizeView:self.window];

    [_graphsTableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorGraphsItemDataType]];
    [_graphsTableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];
}

-(void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    
    [self.window makeKeyAndOrderFront:self];
    [self.window setBackgroundBlurRadius:4];
}

-(void)reloadGraphsTableView:(id)sender
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        
        NSArray *oldGraphsAndGroups = [_graphsAndGroupsCollectionSnapshot copy];

        _graphsAndGroupsCollectionSnapshot = nil;

        [_graphsTableView updateWithObjectValues:self.graphsAndGroupsCollectionSnapshot
                            previousObjectValues:oldGraphsAndGroups];
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

        for (HWMGraphsGroup *group in self.monitorEngine.configuration.graphGroups) {
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
    return self.graphsAndGroupsCollectionSnapshot.count;
}

-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    NSObject *item = [self.graphsAndGroupsCollectionSnapshot objectAtIndex:row];

    if ([item isKindOfClass:[HWMGraph class]]) {
        HWMGraph *graph = (HWMGraph*)item;

        return graph.sensor.legend && self.monitorEngine.configuration.showSensorLegendsInGraphs.boolValue ? 32 : 19;
    }

    // Group
    return 21;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return ![[self.graphsAndGroupsCollectionSnapshot objectAtIndex:row] isKindOfClass:[HWMGraphsGroup class]];
}

//-(BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
//{
//    return [[_items objectAtIndex:row] isKindOfClass:[NSString class]];
//}

-(id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return [self.graphsAndGroupsCollectionSnapshot objectAtIndex:row];
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    id item = [self.graphsAndGroupsCollectionSnapshot objectAtIndex:row];
    id view = [tableView makeViewWithIdentifier:[item identifier] owner:self];
    return view;
}

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    if (_graphsTableView != tableView) {
        return NO;
    }
    
    id item = [self.graphsAndGroupsCollectionSnapshot objectAtIndex:[rowIndexes firstIndex]];
    
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
    id fromItem = [self.graphsAndGroupsCollectionSnapshot objectAtIndex:fromRow];
    
    _currentItemDragOperation = NSDragOperationNone;
    
    if ([fromItem isKindOfClass:[HWMGraph class]] && toRow > 0) {
        
        _currentItemDragOperation = NSDragOperationMove;
        
        if (toRow < self.graphsAndGroupsCollectionSnapshot.count) {
            
            if (toRow == fromRow || toRow == fromRow + 1) {
                _currentItemDragOperation = NSDragOperationNone;
            }
            else {
                id toItem = [self.graphsAndGroupsCollectionSnapshot objectAtIndex:toRow];
                
                if ([toItem isKindOfClass:[HWMGraph class]] && [(HWMGraph*)fromItem group] != [(HWMGraph*)toItem group]) {
                    _currentItemDragOperation = NSDragOperationNone;
                }
            }
        }
        else {
            id toItem = [self.graphsAndGroupsCollectionSnapshot objectAtIndex:toRow - 1];
            
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
    
    HWMGraph *fromItem = [self.graphsAndGroupsCollectionSnapshot objectAtIndex:fromRow];
    
    id checkItem = toRow >= self.graphsAndGroupsCollectionSnapshot.count ? [self.graphsAndGroupsCollectionSnapshot lastObject] : [self.graphsAndGroupsCollectionSnapshot objectAtIndex:toRow];
    
    HWMGraph *toItem = ![checkItem isKindOfClass:[HWMGraph class]] || toRow >= self.graphsAndGroupsCollectionSnapshot.count ? nil : checkItem;
    
    [fromItem.group moveGraphsObjectAtIndex:[fromItem.group.graphs indexOfObject:fromItem]
                                        toIndex:toItem ? [fromItem.group.graphs indexOfObject:toItem] : fromItem.group.graphs.count];

    [self.monitorEngine setNeedsUpdateGraphsList];
    
    return YES;
}

@end
