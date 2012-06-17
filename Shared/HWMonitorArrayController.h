//
//  HWMonitorArrayController.h
//  HWSensors
//
//  Created by Kozlek on 16.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface HWMonitorArrayController : NSArrayController
{
    NSDictionary *_firstFavoriteItem;
    NSDictionary *_firstAvailableItem;
}

@property (assign) IBOutlet NSTableView *tableView;

-(void)setFirstFavoriteItem:(NSString*)favoriteName firstAvailableItem:(NSString*)availableName;

-(NSDictionary*)addFavoriteItem:(NSString*)name icon:(NSImage*)icon key:(NSString*)key;
-(NSDictionary*)addAvailableItem:(NSString*)name icon:(NSImage*)icon key:(NSString*)key;

-(void)removeAllItems;

-(NSArray*)getFavoritesItems;

-(void)setupController;

@end
