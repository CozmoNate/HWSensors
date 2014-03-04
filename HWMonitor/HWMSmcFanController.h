//
//  HWMSmcFanController.h
//  HWMonitor
//
//  Created by Kozlek on 03/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "HWMSensorController.h"

@class HWMSmcFanControlLevel;

@interface HWMSmcFanController : HWMSensorController
{
    HWMSmcFanControlLevel * _previousLevel;
    HWMSmcFanControlLevel * _currentLevel;
}

@property (nonatomic, retain) NSNumber * max;
@property (nonatomic, retain) NSNumber * min;
@property (nonatomic, retain) NSOrderedSet *levels;

-(HWMSmcFanControlLevel*)addOutputLevel:(NSNumber*)output forInputLevel:(NSNumber*)input;

@end

@interface HWMSmcFanController (CoreDataGeneratedAccessors)
- (void)insertObject:(HWMSmcFanControlLevel *)value inLevelsAtIndex:(NSUInteger)idx;
- (void)removeObjectFromLevelsAtIndex:(NSUInteger)idx;
- (void)insertLevels:(NSArray *)value atIndexes:(NSIndexSet *)indexes;
- (void)removeLevelsAtIndexes:(NSIndexSet *)indexes;
- (void)replaceObjectInLevelsAtIndex:(NSUInteger)idx withObject:(HWMSmcFanControlLevel *)value;
- (void)replaceLevelsAtIndexes:(NSIndexSet *)indexes withLevels:(NSArray *)values;
- (void)addLevelsObject:(HWMSmcFanControlLevel *)value;
- (void)removeLevelsObject:(HWMSmcFanControlLevel *)value;
- (void)addLevels:(NSOrderedSet *)values;
- (void)removeLevels:(NSOrderedSet *)values;
@end
