//
//  HWMonitorObjectController.m
//  HWSensors
//
//  Created by Kozlek on 15.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "HWMonitorObjectController.h"

@implementation HWMonitorObjectController

@synthesize items = _items;
@synthesize tableView = _tableView;
@synthesize iconColumn = _iconColumn;
@synthesize nameColumn = _nameColumn;
@synthesize allowDropOperations = _allowDropOperations;

#define kHWMonitorTableViewDataType @"kHWMonitorTableViewDataType"

-(id)init
{
    if (!_items)
        _items = [[NSMutableArray alloc] init];
    
    _allowDropOperations = YES;
    
    return self;
}

- (void) awakeFromNib {
    if (!_items)
        _items = [[NSMutableArray alloc] init];
    
    _allowDropOperations = YES;
    
	[_tableView registerForDraggedTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType]];
	[_tableView setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];
}

-(void)addItem:(NSDictionary*)item
{
    [_items addObject:(id)item];
    
    [_tableView noteNumberOfRowsChanged];
    [_tableView reloadData];
}

-(IBAction)removeSelectedItems:(id)sender
{
    [_items removeObjectsAtIndexes:[_tableView selectedRowIndexes]];
    [_tableView noteNumberOfRowsChanged];
    [_tableView reloadData];
}

// ===========================================
// NSTableView data managing
// ===========================================

- (int)numberOfRowsInTableView:(NSTableView *)aTableView 
{
    if (aTableView == _tableView)
        return [_items count];
	
	return 0;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row 
{	
    if (tableView == _tableView) {
        NSDictionary *item = [_items objectAtIndex:row];
        
        if (tableColumn == _nameColumn){
            return [item objectForKey:@"Name"];
        }
        else if (tableColumn == _iconColumn) {
            return [item objectForKey:@"Icon"];
        }
	} 
	
	return NULL;
}

// ===========================================
// NSTableView drag & drop support
// ===========================================

- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;
{
    //[_tableView selectRowIndexes:rowIndexes byExtendingSelection:YES];

    NSData *indexData = [NSKeyedArchiver archivedDataWithRootObject:[_items objectsAtIndexes:rowIndexes]];
    
    [pboard declareTypes:[NSArray arrayWithObject:kHWMonitorTableViewDataType] owner:self];
    [pboard setData:indexData forType:kHWMonitorTableViewDataType];
    
    return YES;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation;
{
    return NSDragOperationEvery;
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation 
{
    if (_allowDropOperations && operation == NSDragOperationNone) {
        NSPasteboard *pboard = [session draggingPasteboard];
        
        NSArray *pboardItems = [NSKeyedUnarchiver unarchiveObjectWithData:[pboard dataForType:kHWMonitorTableViewDataType]];
        
        NSMutableIndexSet *deletingElements = [[NSMutableIndexSet alloc]init];
        
        for (NSDictionary *pboardItem in pboardItems) {
            NSUInteger i = 0;
            for (i = 0; i < [_items count]; i++) {
                NSDictionary *item = [_items objectAtIndex:i];
                if ([[item objectForKey:@"Key"] compare:[pboardItem objectForKey:@"Key"]] == NSOrderedSame)
                {
                    [deletingElements addIndex:i];
                }
            }
        }
        
        [_items removeObjectsAtIndexes:deletingElements];
        
        [_tableView noteNumberOfRowsChanged];
        [_tableView reloadData];
    }
}

- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation;
{	
    NSPasteboard *pboard = [info draggingPasteboard];
	NSArray *supportedTypes = [NSArray arrayWithObject:kHWMonitorTableViewDataType];
	NSString *availableType = [pboard availableTypeFromArray:supportedTypes];
    
	if ([availableType compare:kHWMonitorTableViewDataType] == NSOrderedSame ) {

        NSArray *pboardItems = [NSKeyedUnarchiver unarchiveObjectWithData:[pboard dataForType:kHWMonitorTableViewDataType]];
        
        if (!_allowDropOperations && tableView == _tableView) {
            return NO;
        }
        
        NSMutableArray *selectedElements = [[NSMutableArray alloc]init];
        for (NSDictionary *pboardItem in pboardItems) {
            for (NSDictionary *item in _items) {
                if ([[item objectForKey:@"Key"] compare:[pboardItem objectForKey:@"Key"]] == NSOrderedSame) 
                    continue;
                
                [selectedElements addObject:item];
            }
        }

        NSMutableArray *newElements = [[NSMutableArray alloc]init];
        NSUInteger i;
		for (i = 0; i < row; i++) {
            NSDictionary *item = [_items objectAtIndex:i];
            
            BOOL shouldContinue = NO;
            
            for (NSDictionary *pboardItem in pboardItems) 
                if ([[item objectForKey:@"Key"] compare:[pboardItem objectForKey:@"Key"]] == NSOrderedSame) {
                    shouldContinue = YES;
                    break;
                }
            
            if (shouldContinue)
                continue;
            
			[newElements addObject:item];
		}
        
        [newElements addObjectsFromArray:pboardItems];
        
        for (i = row; i < [_items count]; i++) {
            NSDictionary *item = [_items objectAtIndex:i];
            
            BOOL shouldContinue = NO;
            
			for (NSDictionary *pboardItem in pboardItems) 
                if ([[item objectForKey:@"Key"] compare:[pboardItem objectForKey:@"Key"]] == NSOrderedSame) {
                    shouldContinue = YES;
                    break;
                }
            
            if (shouldContinue)
                continue;
            
			[newElements addObject:item];
		}
        
        _items = newElements;
		
		[_tableView noteNumberOfRowsChanged];
		[_tableView reloadData];
		
		return YES;
	}
	
	return NO;
}

@end
