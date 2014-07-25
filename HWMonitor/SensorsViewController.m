//
//  SensorsTableViewController.m
//  HWMonitor
//
//  Created by Kozlek on 23/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "SensorsViewController.h"

#import "HWMonitorDefinitions.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMSensorsGroup.h"
#import "HWMSensor.h"

#import "Localizer.h"
#import "JLNFadingScrollView.h"
#import "NSTableView+HWMEngineHelper.h"

@interface SensorsViewController ()
{
    NSDate *_lastReloaded;
}
@property (readonly) HWMEngine *monitorEngine;

@end

@implementation SensorsViewController

#pragma mark - Properties

@synthesize sensorsAndGroupsCollectionSnapshot = _sensorsAndGroupsCollectionSnapshot;
@synthesize contentHeight = _contentHeight;

//-(NSScrollView *)scrollView
//{
//    return (NSScrollView*)self.view;
//}

-(BOOL)hasDraggedFavoriteItem
{
    return YES;
}

-(HWMEngine *)monitorEngine
{
    return [HWMEngine sharedEngine];
}

-(NSArray *)sensorsAndGroupsCollectionSnapshot
{
    if (!_sensorsAndGroupsCollectionSnapshot) {
        _sensorsAndGroupsCollectionSnapshot = [self.monitorEngine.sensorsAndGroups copy];
    }

    return _sensorsAndGroupsCollectionSnapshot;
}

-(CGFloat)contentHeight
{
    if (!_contentHeight) {
        [self.sensorsAndGroupsCollectionSnapshot enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            _contentHeight += [self tableView:_tableView heightOfRow:idx];
        }];
    }

    return _contentHeight;
}

#pragma mark - Overridden Methods

-(id)init
{
    self = [self initWithNibName:NSStringFromClass([SensorsViewController class]) bundle:[NSBundle mainBundle]];

    if (self) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [(JLNFadingScrollView *)self.scrollView setFadeColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];

            [_tableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorPopupItemDataType]];
            [_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationDelete forLocal:YES];

            [self addObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.colorTheme) options:0 context:nil];
            [self addObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.showSensorLegendsInPopup) options:0 context:nil];
            [self addObserver:self forKeyPath:@keypath(self, monitorEngine.sensorsAndGroups) options:0 context:nil];
        }];
    }

    return self;
}

-(void)dealloc
{
    [self removeObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.colorTheme)];
    [self removeObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.showSensorLegendsInPopup)];
    [self removeObserver:self forKeyPath:@keypath(self, monitorEngine.sensorsAndGroups)];
}

#pragma mark - Methods

-(void)reloadData
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        //dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)((NSEC_PER_SEC / 2 + [_lastReloaded timeIntervalSinceNow]) * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        //NSLog(@"reloading data");

            _lastReloaded = [NSDate date];

            NSArray *oldSensorsAndGroups = [_sensorsAndGroupsCollectionSnapshot copy];

            _sensorsAndGroupsCollectionSnapshot = nil;

            [_tableView updateWithObjectValues:self.sensorsAndGroupsCollectionSnapshot previousObjectValues:oldSensorsAndGroups];

            _contentHeight = 0;

            if (self.delegate && [self.delegate respondsToSelector:@selector(sensorsViewControllerDidReloadData:)]) {
                [self.delegate sensorsViewControllerDidReloadData:self];
            }
            
        //NSLog(@"ended reloading data");
        //});
    }];
}

#pragma mark - Events

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@keypath(self, monitorEngine.configuration.colorTheme)]) {
        [(JLNFadingScrollView *)self.scrollView setFadeColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];
    }
    else if ([keyPath isEqual:@keypath(self, monitorEngine.configuration.showSensorLegendsInPopup)] || [keyPath isEqual:@keypath(self, monitorEngine.sensorsAndGroups)]) {
        [self reloadData];
    }
}

#pragma mark - NSTableViewDelegate

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
