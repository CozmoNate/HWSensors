//
//  HWMonitorArrayController.h
//  HWSensors
//
//  Created by kozlek on 16.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface HWMonitorArrayController : NSArrayController
{
    NSDictionary *_firstFavoriteItem;
    NSDictionary *_firstAvailableItem;
}

@property (assign) IBOutlet NSTableView *tableView;

-(void)setFirstFavoriteItem:(NSString*)favoriteName firstAvailableItem:(NSString*)availableName;

-(NSMutableDictionary*)addFavoriteItem;
-(NSDictionary*)addFavoriteItem:(NSString*)name icon:(NSImage*)icon enabled:(NSNumber*)enabled key:(NSString*)key;
-(NSDictionary*)addFavoriteItem:(NSString*)name icon:(NSImage*)icon key:(NSString*)key;

-(NSMutableDictionary*)addAvailableItem;
-(NSDictionary*)addAvailableItem:(NSString*)name icon:(NSImage*)icon enabled:(NSNumber*)enabled key:(NSString*)key;
-(NSDictionary*)addAvailableItem:(NSString*)name icon:(NSImage*)icon key:(NSString*)key;

-(void)removeAllItems;

-(NSArray*)getFavoritesItems;
-(NSArray*)getAllItems;
-(BOOL)favoritesContainKey:(NSString*)key;

-(void)setupController;

@end
