//
//  HWMonitorArrayController.h
//  HWSensors
//
//  Created by kozlek on 16.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface HWMonitorSensorsList : NSArrayController
{
    NSDictionary *_firstFavoriteItem;
    NSDictionary *_firstAvailableItem;
    NSUInteger _globalItemsCount;
    NSArray *_favorites;
}

@property (assign) IBOutlet NSScrollView *scrollView;
@property (assign) IBOutlet NSTableView *tableView;

-(void)setupController;

-(void)setFirstFavoriteItem:(NSString*)favoriteName firstAvailableItem:(NSString*)availableName;

-(NSMutableDictionary*)addItem;
-(NSMutableDictionary*)addItem:(NSString*)name icon:(NSImage*)icon key:(NSString*)key;

-(void)setFavoritesItemsFromArray:(NSArray*)favorites;
-(NSArray*)getFavoritesItems;
-(BOOL)favoritesContainKey:(NSString*)key;

-(NSArray*)getAllItems;
-(void)removeAllItems;

@end
