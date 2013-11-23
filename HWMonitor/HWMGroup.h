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
    kHWMGroupNone           = 0,

    kHWMGroupTemperature    = 1 << 1,
    kHWMGroupVoltage        = 1 << 2,
    kHWMGroupPWM            = 1 << 3,
    kHWMGroupTachometer     = 1 << 4,
    kHWMGroupMultiplier     = 1 << 5,
    kHWMGroupFrequency      = 1 << 6,
    kHWMGroupCurrent        = 1 << 7,
    kHWMGroupPower          = 1 << 8,

    kHWMGroupBattery        = 1 << 9,

    kHWMGroupSmartTemperature       = 1 << 10,
    kHWMGroupSmartRemainingLife     = 1 << 11,
    kHWMGroupSmartRemainingBlocks   = 1 << 12,

} HWMGroupSelector;

@class HWMIcon, HWMSensor;

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
