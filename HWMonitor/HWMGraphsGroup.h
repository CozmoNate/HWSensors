//
//  HWMGraphsGroup.h
//  HWMonitor
//
//  Created by Kozlek on 25/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMGraph, HWMIcon, HWMConfiguration;

@interface HWMGraphsGroup : NSManagedObject

@property (nonatomic, retain) NSNumber * selector;
@property (nonatomic, retain) HWMIcon *icon;
@property (nonatomic, retain) NSOrderedSet *graphs;

@end

@interface HWMGraphsGroup (CoreDataGeneratedAccessors)

- (void)insertObject:(HWMGraph *)value inGraphsAtIndex:(NSUInteger)idx;
- (void)removeObjectFromGraphsAtIndex:(NSUInteger)idx;
- (void)insertGraphs:(NSArray *)value atIndexes:(NSIndexSet *)indexes;
- (void)removeGraphsAtIndexes:(NSIndexSet *)indexes;
- (void)replaceObjectInGraphsAtIndex:(NSUInteger)idx withObject:(HWMGraph *)value;
- (void)replaceGraphsAtIndexes:(NSIndexSet *)indexes withGraphs:(NSArray *)values;
- (void)addGraphsObject:(HWMGraph *)value;
- (void)removeGraphsObject:(HWMGraph *)value;
- (void)addGraphs:(NSOrderedSet *)values;
- (void)removeGraphs:(NSOrderedSet *)values;
@end
