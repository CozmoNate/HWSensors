//
//  HWMGraphsGroup.h
//  HWMonitor
//
//  Created by Kozlek on 27/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "HWMItem.h"

extern NSString * const HWMGraphsGroupHistoryHasBeenChangedNotification;

@class HWMConfiguration, HWMGraph, HWMIcon, HWMSensorsGroup;

@interface HWMGraphsGroup : HWMItem

@property (nonatomic, retain) HWMConfiguration *configuration;
@property (nonatomic, retain) NSOrderedSet *graphs;
@property (nonatomic, retain) HWMIcon *icon;

@property (nonatomic, strong) NSArray *selectors;

@property (readonly) NSNumber *maxGraphsValue;
@property (readonly) NSNumber *minGraphsValue;

-(void)captureSensorValuesToGraphsHistorySetLimit:(NSUInteger)limit;

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
- (void)moveGraphsObject:(HWMGraph *)value toIndex:(NSUInteger)toIndex;
- (void)addGraphs:(NSOrderedSet *)values;
- (void)removeGraphs:(NSOrderedSet *)values;

@end
