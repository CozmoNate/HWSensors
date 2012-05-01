//
//  NSHardwareMonitorSensor.h
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "ATASmartReporter.h"


#define kHWSensorGroupTemperature           1
#define kHWSensorGroupVoltage               2
#define kHWSensorGroupTachometer            3
#define kHWSensorGroupFrequency             4
#define kSMARTSensorGroupTemperature        5
#define kSMARTSensorGroupRemainingLife      6
#define kSMARTSensorGroupRemainingBlocks    7

#define kHWSensorLevelUnused                0
#define kHWSensorLevelDisabled              1
#define kHWSensorLevelNormal                2
#define kHWSensorLevelModerate              3
#define kHWSensorLevelHigh                  4
#define kHWSensorLevelExceeded              1000

#define kHWSensorFlagFavorite               (1 << 0)
#define kHWSensorFlagExtended               (1 << 1)

@interface HWMonitorSensor : NSObject

@property (readwrite, retain) NSString*         key;
@property (readwrite, retain) NSString*         type;
@property (readwrite, assign) NSUInteger        group;
@property (readwrite, retain) NSString *        caption;
@property (readwrite, retain) NSData*           data;
@property (readwrite, retain) ATAGenericDisk*   disk;

@property (readwrite, assign) NSUInteger        level;
@property (readonly) NSString*                  value;

@property (readwrite, assign) BOOL              favorite;
@property (readwrite, assign) BOOL              extendedFormat;
@property (readonly) BOOL                       levelHasBeenChanged;
@property (readonly) BOOL                       valueHasBeenChanged;

@property (readwrite, retain) NSMenuItem*       menuItem;

+ (int)getIndexOfHexChar:(char)c;
+ (HWMonitorSensor*)sensor;

- (void)setType:(NSString *)newType;
- (NSString *)type;
- (void)setData:(NSData *)newData;
- (NSData *)data;

- (float)decodeValue;
- (NSString*)value;

@end
