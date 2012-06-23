//
//  HWMonitorArrayController.m
//  HWSensors
//
//  Created by kozlek on 16.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorArrayController.h"

@implementation HWMonitorArrayController

@synthesize tableView = _tableView;

#define kHWMonitorTableViewDataType @"kHWMonitorTableViewDataType"

-(id)init
{
    return self;
}

-(void)setFirstFavoriteItem:(NSString*)favoriteName firstAvailableItem:(NSString*)availableName;
{
    if ([[self arrangedObjects] count] > 0)
        [self removeAllItems];
    
    _firstFavoriteItem = [NSDictionary dictionaryWithObjectsAndKeys:favoriteName, @"Name", /*[NSImage imageNamed:NSImageNameIconViewTemplate], @"Icon",*/ [[NSObject alloc] init], @"IsSeparator", nil];
    _firstAvailableItem = [NSDictionary dictionaryWithObjectsAndKeys:availableName, @"Name", /*[NSImage imageNamed:NSImageNameListViewTemplate], @"Icon",*/ [[NSObject alloc] init], @"IsSeparator", nil];
    
    [self insertObject:_firstFavoriteItem atArrangedObjectIndex:0];
    [self insertObject:_firstAvailableItem atArrangedObjectIndex:1];
}

-(NSDictionary*)addFavoriteItem:(NSString*)name icon:(NSImage*)icon key:(NSString*)key
{
    NSMutableDictionary *item = [NSMutableDictionary dictionaryWithObjectsAndKeys:name, @"Name", icon, @"Icon", key, @"Key", nil];
    
    [self insertObject:item atArrangedObjectIndex:[[self arrangedObjects] indexOfObject:_firstAvailableItem]];
    
    return item;
}

-(NSDictionary*)addAvailableItem:(NSString*)name icon:(NSImage*)icon key:(NSString*)key
{
    NSMutableDictionary *item = [NSMutableDictionary dictionaryWithObjectsAndKeys:name, @"Name", icon, @"Icon", key, @"Key", nil];
    
    [self addObject:item];
    
    return item;
}

-(void)removeAllItems
{
    NSRange range = NSMakeRange(0, [[self arrangedObjects] count]);
    [self removeObjectsAtArrangedObjectIndexes:[NSIndexSet indexSetWithIndexesInRange:range]];
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
	[_tableView setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];
}

-(NSDictionary*)addItemWithName:(NSString*)name icon:(NSImage*)icon key:(NSString*)key
{
    NSMutableDictionary *item = [NSMutableDictionary dictionaryWithObjectsAndKeys:name, @"Name", icon, @"Icon", key, @"Key", nil];
    
    [self addObject:item];
    
    return item;
}

- (void) awakeFromNib {
	[self setupController];
}

// ===========================================
// NSTableView data managing
// ===========================================

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return nil == [[[self arrangedObjects] objectAtIndex:row] valueForKey:@"IsSeparator"];
}

/*- (NSIndexSet *)tableView:(NSTableView *)tableView selectionIndexesForProposedSelection:(NSIndexSet *)proposedSelectionIndexes
{
    if ([proposedSelectionIndexes count] > 0 && [[[self arrangedObjects] objectAtIndex:[proposedSelectionIndexes firstIndex]] valueForKey:@"Icon"]) {
        return [NSIndexSet indexSetWithIndex:[proposedSelectionIndexes firstIndex]];
    }
    
    return [NSIndexSet indexSet];
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
    
    [pboard declareTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType] owner:self];
    [pboard setData:indexData forType:kHWMonitorTableViewDataType];
    
    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    return row > 0 ? NSDragOperationEvery : NSDragOperationNone;
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation;
{
    if (operation == NSDragOperationNone) {
        
        NSPasteboard* pboard = [session draggingPasteboard];
        NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
        
        NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
        
        int from = [rowIndexes firstIndex];
        int row = [[self arrangedObjects] indexOfObject:_firstAvailableItem] + 1;
        
        NSDictionary *item = [[self arrangedObjects] objectAtIndex:from];
        
        [self insertObject:item atArrangedObjectIndex:row];
        [self removeObjectAtArrangedObjectIndex:from > row ? from + 1 : from];
        
        [NSApp sendAction:[_tableView action] to:[_tableView target]  from:_tableView];
    }
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation;
{
    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:kHWMonitorTableViewDataType];
    
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];
    
    int from = [rowIndexes firstIndex];
    
    NSDictionary *item = [[self arrangedObjects] objectAtIndex:from];
    
    [self insertObject:item atArrangedObjectIndex:row];
    [self removeObjectAtArrangedObjectIndex:from > row ? from + 1 : from];
    
    [NSApp sendAction:[_tableView action] to:[_tableView target]  from:_tableView];
    
    return YES;
}

@end
