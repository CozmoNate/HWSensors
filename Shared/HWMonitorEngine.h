//
//  HWMonitorEngine.h
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "HWMonitorSensor.h"
#import "ATASmartReporter.h"

@interface HWMonitorEngine : NSObject
{
@private
    io_service_t service;
    NSBundle* bundle;
    NSATASmartReporter *smartReporter;
    NSMutableArray *sensors;
    NSMutableDictionary *keys;
}

@property (readonly) NSArray *sensors;
@property (readonly) NSDictionary *keys;

@property (readwrite, assign) BOOL useFahrenheit;
@property (readwrite, assign) BOOL hideDisabledSensors;
@property (readwrite, assign) BOOL showBSDNames;

+ (HWMonitorEngine*)engine;

+ (NSArray*)populateInfoForKey:(NSString *)key;
+ (NSString*)getTypeFromKeyInfo:(NSArray*)info;
+ (NSData*)getValueFromKeyInfo:(NSArray*)info;

- (HWMonitorSensor*)addSensorWithKey:(NSString*)key caption:(NSString*)caption group:(NSUInteger)group;
- (HWMonitorSensor*)addSMARTSensorWithGenericDisk:(ATAGenericDisk*)disk group:(NSUInteger)group;

- (id)init;
- (id)initWithBundle:(NSBundle*)mainBundle;
- (void)dealloc;

- (void)rebuildSensorsList;
- (void)updateSMARTSensorsValues;
- (void)updateGenericSensorsValuesButOnlyFavorits:(BOOL)updateOnlyFavorites;

- (NSArray*)getAllSensorsInGroup:(NSUInteger)group;

@end
