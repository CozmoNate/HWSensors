//
//  HWMItem.h
//  HWMonitor
//
//  Created by Kozlek on 16/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMEngine;
@class HWMFavorite;

@interface HWMItem : NSManagedObject

@property (nonatomic, retain) NSString * name;
@property (nonatomic, retain) NSNumber * order;
@property (nonatomic, retain) NSString * title;
@property (nonatomic, retain) NSString * legend;
@property (nonatomic, retain) NSString * identifier;
@property (nonatomic, retain) NSNumber * hidden;

@property (nonatomic, strong) HWMEngine * engine;
@property (nonatomic, retain) HWMFavorite * favorite;

@end
