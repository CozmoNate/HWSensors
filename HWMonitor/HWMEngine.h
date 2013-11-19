//
//  HWMEngine.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum {
    kHWMEngineIdle = 0,
    kHWMEngineActive,
} HWMEngineState;

typedef enum {
    kHWMSensorsUpdateLoopRegular = 0,
    kHWMSensorsUpdateLoopForced,
    kHWMSensorsUpdateLoopOnlyFavorites
} HWMSensorsUpdateLoopStrategy;


@class ColorTheme;
@class HWMConfiguration;

@interface HWMEngine : NSObject
{
    NSArray *_platformProfile;
    NSMutableArray *_arrangedItems;
    NSMutableArray *_favoriteItems;
    NSArray *_smcAndDevicesSensors;
    NSArray *_ataSmartSensors;
    io_connect_t _smcConnection;
    io_connect_t _fakeSmcConnection;
    NSTimer *_smcAndDevicesSensorsUpdateLoopTimer;
    NSTimer *_ataSmartSensorsUpdateLoopTimer;
}

@property (nonatomic, strong) NSBundle * bundle;

@property (nonatomic, strong) IBOutlet NSManagedObjectModel * managedObjectModel;
@property (nonatomic, strong) IBOutlet NSPersistentStoreCoordinator * persistentStoreCoordinator;
@property (nonatomic, strong) IBOutlet NSManagedObjectContext * managedObjectContext;

@property (nonatomic, assign) HWMEngineState engineState;
@property (nonatomic, assign) HWMSensorsUpdateLoopStrategy updateLoopStrategy;

@property (nonatomic, strong) IBOutlet HWMConfiguration * configuration;
@property (readonly) IBOutlet NSArray * arrangedItems;
@property (readonly) IBOutlet NSArray * favoriteItems;

+(HWMEngine*)engineWithBundle:(NSBundle*)bundle;

-(void)rebuildSensorsList;
-(void)startEngine;
-(void)stopEngine;
-(void)saveContext;
-(void)updateSmcAndDevicesSensors;
-(void)updateAtaSmartSensors;

@end
