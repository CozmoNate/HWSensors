//
//  HWMonitorSensor.h
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "ATASmartReporter.h"

enum HWSensorGroup {
    kHWSensorGroupTemperature           = 1 << 0,
    kHWSensorGroupVoltage               = 1 << 1,
    kHWSensorGroupPWM                   = 1 << 2,
    kHWSensorGroupTachometer            = 1 << 3,
    kHWSensorGroupMultiplier            = 1 << 4,
    kHWSensorGroupFrequency             = 1 << 5,    
    
    kSMARTSensorGroupTemperature        = 1 << 6,   
    kSMARTSensorGroupRemainingLife      = 1 << 7,    
    kSMARTSensorGroupRemainingBlocks    = 1 << 8,
};

#define kHWSensorLevelUnused                0
#define kHWSensorLevelDisabled              1
#define kHWSensorLevelNormal                2
#define kHWSensorLevelModerate              3
#define kHWSensorLevelHigh                  4
#define kHWSensorLevelExceeded              1000

@interface HWMonitorSensor : NSObject

@property (readwrite, retain) id                engine;

@property (readwrite, retain) NSString*         key;
@property (readwrite, retain) NSString*         type;
@property (readwrite, assign) NSUInteger        group;
@property (readwrite, retain) NSString *        caption;
@property (readwrite, retain) NSData*           data;
@property (readwrite, retain) ATAGenericDisk*   disk;

@property (readwrite, assign) NSUInteger        level;
@property (readonly) NSString*                  value;

@property (readwrite, assign) BOOL              favorite;

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
