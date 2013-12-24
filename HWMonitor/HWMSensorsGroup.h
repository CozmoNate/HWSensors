//
//  HWMSensorsGroup.h
//  HWMonitor
//
//  Created by Kozlek on 25/11/13.
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

#define kHWMGroupNone                   0

#define kHWMGroupTemperature            1
#define kHWMGroupVoltage                2
#define kHWMGroupPWM                    3
#define kHWMGroupTachometer             4
#define kHWMGroupMultiplier             5
#define kHWMGroupFrequency              6
#define kHWMGroupCurrent                7
#define kHWMGroupPower                  8

#define kHWMGroupSmartTemperature       9
#define kHWMGroupSmartRemainingLife     10
#define kHWMGroupSmartRemainingBlocks   11

#define kHWMGroupBattery                12


@class HWMIcon, HWMSensor, HWMConfiguration;

@interface HWMSensorsGroup : HWMItem

@property (nonatomic, retain) HWMConfiguration * configuration;
@property (nonatomic, retain) NSNumber * selector;
@property (nonatomic, retain) HWMIcon *icon;
@property (nonatomic, retain) NSOrderedSet *sensors;

-(void)exchangeSensorsObjectAtIndex:(NSUInteger)fromIndex withSensorsObjectAtIndex:(NSUInteger)toIndex;

@end

@interface HWMSensorsGroup (CoreDataGeneratedAccessors)

- (void)insertObject:(HWMSensor *)value inSensorsAtIndex:(NSUInteger)idx;
- (void)removeObjectFromSensorsAtIndex:(NSUInteger)idx;
- (void)insertSensors:(NSArray *)value atIndexes:(NSIndexSet *)indexes;
- (void)removeSensorsAtIndexes:(NSIndexSet *)indexes;
- (void)replaceObjectInSensorsAtIndex:(NSUInteger)idx withObject:(HWMSensor *)value;
- (void)replaceSensorsAtIndexes:(NSIndexSet *)indexes withSensors:(NSArray *)values;
- (void)addSensorsObject:(HWMSensor *)value;
- (void)removeSensorsObject:(HWMSensor *)value;
- (void)addSensors:(NSOrderedSet *)values;
- (void)removeSensors:(NSOrderedSet *)values;
@end
