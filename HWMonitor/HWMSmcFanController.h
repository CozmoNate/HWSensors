//
//  HWMSmcFanController.h
//  HWMonitor
//
//  Created by Kozlek on 03/03/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
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
#import "HWMSensorController.h"

@class HWMSmcFanControlLevel;

@interface HWMSmcFanController : HWMSensorController
{
     HWMSmcFanControlLevel * _currentLevel;
}

@property (nonatomic, retain) NSNumber * max;
@property (nonatomic, retain) NSNumber * min;
@property (nonatomic, retain) NSOrderedSet *levels;

@property (nonatomic, strong) NSNumber * rangedMax;
@property (nonatomic, strong) NSNumber * rangedMin;

-(HWMSmcFanControlLevel*)addOutputLevel:(NSNumber*)output forInputLevel:(NSNumber*)input;
-(void)calculateOutputRange;

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
