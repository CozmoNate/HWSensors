//
//  HWMFavorite.h
//  HWMonitor
//
//  Created by Kozlek on 07.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMConfiguration, HWMItem;

@interface HWMFavorite : NSManagedObject

@property (nonatomic, retain) NSNumber *large;

@property (nonatomic, retain) HWMConfiguration *configuration;
@property (nonatomic, retain) HWMItem *item;

@end
