//
//  HWMonitorArrayController.m
//  HWSensors
//
//  Created by kozlek on 16.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorSensorsController.h"
#import "HWMonitorDefinitions.h"

@implementation HWMonitorSensorsController

@synthesize scrollView = _scrollView;
@synthesize tableView = _tableView;

-(void)setFirstFavoriteItem:(NSString*)favoriteName firstAvailableItem:(NSString*)availableName;
{
    if ([[self arrangedObjects] count] > 0)
        [self removeAllItems];
    
    _firstFavoriteItem = [NSDictionary dictionaryWithObjectsAndKeys:favoriteName, kHWMonitorKeyName, /*[NSImage imageNamed:NSImageNameIconViewTemplate], kHWMonitorKeyIcon,*/ [[NSObject alloc] init], kHWMonitorKeySeparator, nil];
    _firstAvailableItem = [NSDictionary dictionaryWithObjectsAndKeys:availableName, kHWMonitorKeyName, /*[NSImage imageNamed:NSImageNameListViewTemplate], kHWMonitorKeyIcon,*/ [[NSObject alloc] init], kHWMonitorKeySeparator, nil];
    
    [self insertObject:_firstFavoriteItem atArrangedObjectIndex:0];
    [self insertObject:_firstAvailableItem atArrangedObjectIndex:1];
    
    [_tableView setShowsStickyRowHeader:YES];
}

-(NSMutableDictionary*)addItem
{
    NSMutableDictionary *item = [NSMutableDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithLong:_globalItemsCount], kHWMonitorKeyIndex, nil];
    
    [self addObject:item];
    
    _globalItemsCount++;
    
    return item;
}

-(NSMutableDictionary*)addItem:(NSString*)name icon:(NSImage*)icon key:(NSString*)key
{
    NSMutableDictionary *item = [NSMutableDictionary dictionaryWithObjectsAndKeys:name, kHWMonitorKeyName, icon, kHWMonitorKeyIcon, key, kHWMonitorKeyKey, [NSNumber numberWithLong:_globalItemsCount], kHWMonitorKeyIndex, nil];
    
    [self addObject:item];
    
    _globalItemsCount++;
    
    return item;
}

-(void)removeAllItems
{
    NSRange range = NSMakeRange(0, [[self arrangedObjects] count]);
    [self removeObjectsAtArrangedObjectIndexes:[NSIndexSet indexSetWithIndexesInRange:range]];
}

-(void)setFavoritesItemsFromArray:(NSArray*)favorites
{
    _favorites = [NSArray arrayWithArray:favorites];
    
    for (NSString *key in favorites) {
        
        NSUInteger index;
        
        for (index = 1; index < [[self arrangedObjects] count]; index++) {
            NSDictionary *item = [[self arrangedObjects] objectAtIndex:index];
            
            if ([[item valueForKey:kHWMonitorKeyKey] isEqualToString:key]) {
                NSUInteger to = [[self arrangedObjects] indexOfObject:_firstAvailableItem];
                [self insertObject:item atArrangedObjectIndex:to];
                [self removeObjectAtArrangedObjectIndex:index > to ? index + 1 : index];
                break;
            }
        }
    }
}

-(NSArray*)getFavoritesItems
{
    NSMutableArray *list = [[NSMutableArray alloc] init];
 
    NSUInteger index;
    
    for (index = 1; index < [[self arrangedObjects] count]; index++) {
        
        NSDictionary *item = [[self arrangedObjects] objectAtIndex:index];
        
        if (_firstAvailableItem != item)
            [list addObject:item];
        else
            break;
    }
    
    return list;
}

-(NSArray *)getAllItems
{
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    NSUInteger index;
    
    for (index = 1; index < [[self arrangedObjects] count]; index++) {
        
        NSDictionary *item = [[self arrangedObjects] objectAtIndex:index];
        
        if (/*_firstAvailableItem != item && */![item valueForKey:kHWMonitorKeySeparator])
            [list addObject:item];
    }
    
    return list;
}

-(BOOL)favoritesContainKey:(NSString*)key
{
    NSUInteger index;
    
    for (index = 1; index < [[self arrangedObjects] count]; index++) {
        
        NSDictionary *item = [[self arrangedObjects] objectAtIndex:index];
        
        if ([(NSString*)[item valueForKey:@"Key"] isEqualToString:key]) {
            return YES;
        }
    }
    
    return NO;
}

-(void)setupController
{
    [_tableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
    [_tableView setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:NO];
    
    _globalItemsCount = 0;
    
    _favorites = [[NSArray alloc] init];
}

- (void) awakeFromNib
{
	[self setupController];
}

// ===========================================
// NSTableView data managing
// ===========================================

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return nil == [[[self arrangedObjects] objectAtIndex:row] valueForKey:kHWMonitorKeySeparator];
}

- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    if ([cell isKindOfClass:[NSButtonCell class]])
        [cell setTransparent:![cell isEnabled]];
    /*if ([cell isKindOfClass:[NSButtonCell class]] && [[[self arrangedObjects] objectAtIndex:row] valueForKey:@"Visible"]) {
        [cell setEnabled:YES];
    }
    else {
        [cell setEnabled:NO];
    }*/
}

- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row
{
	return nil != [[[self arrangedObjects] objectAtIndex:row] valueForKey:kHWMonitorKeySeparator];
}

/*- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    
}*/

// ===========================================
// NSTableView drag & drop support
// ===========================================


- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    //[_tableView selectRowIndexes:rowIndexes byExtendingSelection:YES];
    
    if ([rowIndexes count] == 0 || [[[self arrangedObjects] objectAtIndex:[rowIndexes firstIndex]] valueForKey:@"IsSeparator"]) 
        return NO;
    
    NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
    
    [pboard declareTypes:[NSArray arrayWithObjects:kHWMonitorTableViewDataType, NSStringPboardType, nil] owner:self];
    [pboard setData:indexData forType:kHWMonitorTableViewDataType];
    
    NSDictionary *item = [[self arrangedObjects] objectAtIndex:[rowIndexes firstIndex]];
    
    [pboard setString:[item valueForKey:kHWMonitorKeyKey] forType:NSStringPboardType];
    
    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
    
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    
    NSInteger itemsRow = [[self arrangedObjects] indexOfObject:_firstAvailableItem];
    NSInteger fromRow = [rowIndexes firstIndex];
    
    return (fromRow > itemsRow && row <= itemsRow && row > 0) || (fromRow < itemsRow && row > 0)  ? NSDragOperationMove : NSDragOperationNone;
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)toRow dropOperation:(NSTableViewDropOperation)dropOperation;
{
    //[_scrollView setDocumentCursor:[NSCursor currentSystemCursor]];
    
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
    
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    
    NSInteger itemsRow = [[self arrangedObjects] indexOfObject:_firstAvailableItem];
    NSInteger fromRow = [rowIndexes firstIndex];
    
    NSDictionary *movingItem = [[self arrangedObjects] objectAtIndex:fromRow];
    NSUInteger movingItemIndex = [[movingItem valueForKey:kHWMonitorKeyIndex] longValue];
    
    if (toRow > itemsRow) {
        
        NSUInteger index;
        
        for (index = itemsRow + 1; index < [[self arrangedObjects] count]; index++) {
            
            NSDictionary *item = [[self arrangedObjects] objectAtIndex:index];
            
            if ([[item valueForKey:kHWMonitorKeyIndex] longValue] > movingItemIndex) {
                [self insertObject:movingItem atArrangedObjectIndex:index];
                [self removeObjectAtArrangedObjectIndex:fromRow > index ? fromRow + 1 : fromRow];
                break;
            }
        }
        
        if (index >= [[self arrangedObjects] count]) {
            [self insertObject:movingItem atArrangedObjectIndex:index];
            [self removeObjectAtArrangedObjectIndex:fromRow];
        }
    }
    else {
        [self insertObject:movingItem atArrangedObjectIndex:toRow];
        [self removeObjectAtArrangedObjectIndex:fromRow > toRow ? fromRow + 1 : fromRow];
    }
    
    [NSApp sendAction:[_tableView action] to:[_tableView target] from:_tableView];
    
    return YES;
}

@end
