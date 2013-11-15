//
//  HWMBatterySensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "HWMSensor.h"

#define kHWMBatterySensorTypeNone       0
#define kHWMBatterySensorTypeInternal   1
#define kHWMBatterySensorTypeMouse      2
#define kHWMBatterySensorTypeKeyboard   3
#define kHWMBatterySensorTypeTrackpad   4

@interface HWMBatterySensor : HWMSensor

@property (nonatomic, retain) NSNumber * selector;
@property (nonatomic, retain) NSNumber * service;

+(NSArray*)discoverDevices;

-(void)doUpdateValue;

@end
