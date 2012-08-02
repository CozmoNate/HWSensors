//
//  HWMonitorEngine.h
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

#import "HWMonitorSensor.h"
#import "ATASmartReporter.h"

@interface HWMonitorEngine : NSObject
{
@private
    io_service_t _service;
    NSATASmartReporter *_smartReporter;
    NSMutableArray *_sensors;
    NSMutableDictionary *_keys;
}

@property (readwrite, retain) NSBundle* bundle;

@property (readonly) NSArray *sensors;
@property (readonly) NSDictionary *keys;

@property (readwrite, assign) BOOL useFahrenheit;
@property (readwrite, assign) BOOL useBSDNames;

+ (HWMonitorEngine*)engineWithBundle:(NSBundle*)bundle;

+ (NSArray*)populateInfoForKey:(NSString *)key;
+ (NSString*)copyTypeFromKeyInfo:(NSArray*)info;
+ (NSData*)copyValueFromKeyInfo:(NSArray*)info;

- (HWMonitorSensor*)addSensorWithKey:(NSString*)key title:(NSString*)title group:(NSUInteger)group;
- (HWMonitorSensor*)addSMARTSensorWithGenericDisk:(ATAGenericDisk*)disk group:(NSUInteger)group;

- (id)init;
- (id)initWithBundle:(NSBundle*)mainBundle;
- (void)dealloc;

- (void)rebuildSensorsList;
- (NSArray*)updateSMARTSensorsValues;
- (NSArray*)updateGenericSensorsValues;
- (NSArray*)updateFavoritesSensorsValues:(NSArray*)favorites;

- (NSArray*)getAllSensorsInGroup:(NSUInteger)group;

@end
