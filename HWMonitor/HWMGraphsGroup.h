//
//  HWMGraphsGroup.h
//  HWMonitor
//
//  Created by Kozlek on 27/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "HWMItem.h"

@class HWMConfiguration, HWMGraph, HWMIcon, HWMSensorsGroup;

@interface HWMGraphsGroup : HWMItem

@property (nonatomic, retain) HWMConfiguration *configuration;
@property (nonatomic, retain) NSOrderedSet *graphs;
@property (nonatomic, retain) HWMIcon *icon;

@property (assign) NSUInteger selectors;

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
