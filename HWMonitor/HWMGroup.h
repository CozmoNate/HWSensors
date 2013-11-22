//
//  HWMGroup.h
//  HWMonitor
//
//  Created by Kozlek on 16/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

#import "HWMItem.h"

typedef enum {
    kHWMGroupNone = 0,

    kHWMGroupTemperature,
    kHWMGroupVoltage,
    kHWMGroupPWM,
    kHWMGroupTachometer,
    kHWMGroupMultiplier,
    kHWMGroupFrequency,
    kHWMGroupCurrent,
    kHWMGroupPower,

    kHWMGroupBattery,

    kHWMGroupSmartTemperature,
    kHWMGroupSmartRemainingLife,
    kHWMGroupSmartRemainingBlocks,

    kHWMGroupSelectorsCount

} HWMGroupSelector;

@class HWMIcon, HWMItem, HWMSensor;

@interface HWMGroup : HWMItem

@property (nonatomic, retain) NSNumber * selector;

@property (nonatomic, retain) HWMIcon *icon;
@property (nonatomic, retain) NSSet *sensors;

@end

@interface HWMGroup (CoreDataGeneratedAccessors)

- (void)addSensorsObject:(HWMSensor *)value;
- (void)removeSensorsObject:(HWMSensor *)value;
- (void)addSensors:(NSSet *)values;
- (void)removeSensors:(NSSet *)values;

@end
