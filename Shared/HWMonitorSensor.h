//
//  NSHardwareMonitorSensor.h
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "ATASmartReporter.h"

enum {
    kHWTemperatureGroup =        1,
    kHWVoltageGroup =            2,
    kHWTachometerGroup =         3,
    kHWFrequencySensor =          4,
    kHWMultiplierGroup =         5,
    kHWSMARTTemperatureGroup =   6,
    kHWSMARTRemainingLifeGroup = 7,
};

@interface HWMonitorSensor : NSObject

@property (readwrite, retain) NSString *key;
@property (readwrite, retain) NSString *type;
@property (readwrite, assign) NSUInteger group;
@property (readwrite, retain) NSString *caption;
@property (readwrite, retain) NSData *value;
@property (readwrite, retain) ATAGenericDisk *disk;

@property (readwrite, retain) NSMenuItem *menuItem;
@property (readwrite, assign) BOOL favorite;

+ (int)getIndexOfHexChar:(char)c;
+ (float)decodeSMCFloatOfType:(const char*)type fraction:(UInt16) encoded;

+ (HWMonitorSensor*)sensor;

- (float)decodeValue;
- (NSString*)formatValue;

@end
