//
//  HWMItem.h
//  HWMonitor
//
//  Created by Kozlek on 07.12.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMEngine, HWMFavorite, RACSignal;

@interface HWMItem : NSManagedObject

@property (nonatomic, retain) NSNumber * hidden;
@property (nonatomic, retain) NSString * identifier;
@property (nonatomic, retain) NSString * legend;
@property (nonatomic, retain) NSString * name;
@property (nonatomic, retain) NSString * title;
@property (nonatomic, retain) NSSet *favorites;

@property (nonatomic, strong) HWMEngine * engine;

@property (readonly) RACSignal *hasBeenDeletedSignal;

-(void)initialize;

@end

@interface HWMItem (CoreDataGeneratedAccessors)

- (void)addFavoriteObject:(HWMFavorite *)value;
- (void)removeFavoriteObject:(HWMFavorite *)value;
- (void)addFavorite:(NSSet *)values;
- (void)removeFavorite:(NSSet *)values;

@end
