//
//  HWMSensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

#import "HWMItem.h"


#define kHWMSensorLevelNormal       0
#define kHWMSensorLevelModerate     1
#define kHWMSensorLevelHigh         2
#define kHWMSensorLevelExceeded     1000


@class HWMSensorsGroup, HWMGraph;

@interface HWMSensor : HWMItem
{
    NSUInteger _alarmLevel;
}

@property (nonatomic, retain) NSNumber * service;
@property (nonatomic, retain) NSNumber * selector;
@property (nonatomic, retain) NSString * type;
@property (nonatomic, retain) NSNumber * value;
@property (nonatomic, retain) HWMSensorsGroup * group;
@property (nonatomic, retain) HWMGraph *graph;

@property (readonly) NSUInteger alarmLevel;
@property (readonly) NSString * formattedValue;

- (void)doUpdateValue;

- (NSNumber*)internalUpdateValue;
- (NSUInteger)internalUpdateAlarmLevel;

@end
