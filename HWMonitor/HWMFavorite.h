//
//  HWMFavorite.h
//  HWMonitor
//
//  Created by Kozlek on 23/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMItem;

@interface HWMFavorite : NSManagedObject

@property (nonatomic, retain) NSNumber * order;
@property (nonatomic, retain) HWMItem *item;

@end
