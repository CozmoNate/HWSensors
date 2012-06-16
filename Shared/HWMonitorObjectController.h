//
//  HWMonitorObjectController.h
//  HWSensors
//
//  Created by Kozlek on 15.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class HWMonitor;

@interface HWMonitorObjectController : NSObjectController

@property (readwrite, retain) NSMutableArray *items;

@property (assign) IBOutlet NSTableView *tableView;
@property (assign) IBOutlet NSTableColumn *iconColumn;
@property (assign) IBOutlet NSTableColumn *nameColumn;

@property (readwrite, assign) BOOL allowDropOperations;

-(IBAction)removeSelectedItems:(id)sender;

-(void)addItem:(NSDictionary*)item;

@end
