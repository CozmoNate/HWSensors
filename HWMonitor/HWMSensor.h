//
//  HWMSensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
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

#define kHWMSensorLevelDisabled     -1
#define kHWMSensorLevelNormal       0
#define kHWMSensorLevelModerate     1
#define kHWMSensorLevelHigh         2
#define kHWMSensorLevelExceeded     1000

@class HWMGraph, HWMSensorController, HWMSensorsGroup;

@interface HWMSensor : HWMItem
{
    NSUInteger _alarmLevel;
    NSString *_formattedValue;
    NSString *_strippedValue;
    NSDate * _lastUpdated;
}

@property (nonatomic, retain) NSNumber * forced;
@property (nonatomic, retain) NSNumber * selector;
@property (nonatomic, retain) NSNumber * service;
@property (nonatomic, retain) NSString * type;
@property (nonatomic, retain) NSNumber * value;
@property (nonatomic, retain) HWMGraph *graph;
@property (nonatomic, retain) HWMSensorsGroup *group;
@property (nonatomic, retain) HWMSensorController *controller;
@property (nonatomic, retain) NSSet *consumers;

@property (readonly) NSUInteger alarmLevel;
@property (readonly) NSString * formattedValue;
@property (readonly) NSString * strippedValue;

@property (readonly) NSTimeInterval timeIntervalSinceLastUpdate;

- (BOOL)isActive;

- (void)doUpdateValue;

- (NSNumber*)internalUpdateValue;
- (NSUInteger)internalUpdateAlarmLevel;
- (void)internalSendAlarmNotification;

@end

@interface HWMSensor (CoreDataGeneratedAccessors)

- (void)addConsumersObject:(HWMSensorController *)value;
- (void)removeConsumersObject:(HWMSensorController *)value;
- (void)addConsumers:(NSSet *)values;
- (void)removeConsumers:(NSSet *)values;

@end
