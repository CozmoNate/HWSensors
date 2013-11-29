//
//  HWMSensorsGroup.h
//  HWMonitor
//
//  Created by Kozlek on 25/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

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

#define kHWMGroupBatteryInternal        13
#define kHWMGroupBatteryMouse           14
#define kHWMGroupBatteryKeyboard        15
#define kHWMGroupBatteryTrackpad        16

@class HWMIcon, HWMSensor, HWMConfiguration;

@interface HWMSensorsGroup : HWMItem

@property (nonatomic, retain) NSNumber * selector;
@property (nonatomic, retain) HWMIcon *icon;
@property (nonatomic, retain) NSOrderedSet *sensors;

-(void)moveSensorsObject:(HWMSensor *)value toIndex:(NSUInteger)toIndex;

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
