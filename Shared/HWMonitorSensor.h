//
//  NSHardwareMonitorSensor.h
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "ATASmartReporter.h"


#define kHWSensorGroupTemperature       1
#define kHWSensorGroupVoltage           2
#define kHWSensorGroupTachometer        3
#define kHWSensorGroupFrequency         4
#define kHWSensorGroupMultiplier        5
#define kSMARTSensorGroupTemperature    6
#define kSMARTSensorGroupRemainingLife  7

#define kHWSensorLevelUnused            0
#define kHWSensorLevelDisabled          1
#define kHWSensorLevelNormal            2
#define kHWSensorLevelModerate          3
#define kHWSensorLevelHigh              4
#define kHWSensorLevelExceeded          1000

@interface HWMonitorSensor : NSObject

@property (readwrite, retain) NSString *key;
@property (readwrite, retain) NSString *type;
@property (readwrite, assign) NSUInteger group;
@property (readwrite, retain) NSString *caption;
@property (readwrite, retain) NSData *value;
@property (readwrite, retain) ATAGenericDisk *disk;

@property (readwrite, assign) BOOL favorite;
@property (readwrite, assign) NSUInteger level;
@property (readonly) BOOL levelHasBeenChanged;

@property (readwrite, retain) NSMenuItem *menuItem;

+ (int)getIndexOfHexChar:(char)c;
+ (HWMonitorSensor*)sensor;

- (float)decodeValue;
- (NSString*)formattedValue;

@end
