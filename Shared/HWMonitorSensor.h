//
//  HWMonitorSensor.h
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


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

enum HWSensorLevel {
    kHWSensorLevelUnused                = 0,
    kHWSensorLevelDisabled              = 1,
    kHWSensorLevelNormal                = 2,
    kHWSensorLevelModerate              = 3,
    kHWSensorLevelHigh                  = 4,
    kHWSensorLevelExceeded              = 1000,
};

@class HWMonitorEngine;

@interface HWMonitorSensor : NSObject

@property (readwrite, retain) HWMonitorEngine*  engine;

@property (readwrite, retain) NSString* name;
@property (readwrite, retain) NSString* type;
@property (readwrite, assign) NSUInteger group;
@property (readwrite, retain) NSString* title;
@property (readwrite, retain) NSData* data;
@property (readwrite, retain) ATAGenericDisk* disk;

@property (readwrite, assign) enum HWSensorLevel level;
@property (readonly) NSNumber* rawValue;
@property (readonly) NSString* formattedValue;

@property (readonly) BOOL levelHasBeenChanged;
@property (readonly) BOOL valueHasBeenChanged;

@property (readwrite, retain) id representedObject;

+ (int)getIndexOfHexChar:(char)c;
+ (HWMonitorSensor*)sensor;

- (void)setType:(NSString *)newType;
- (NSString *)type;
- (void)setData:(NSData *)newData;
- (NSData *)data;

- (float)decodeValue;
- (NSString*)formattedValue;

@end
