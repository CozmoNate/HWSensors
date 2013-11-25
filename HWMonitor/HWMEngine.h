//
//  HWMEngine.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

extern NSString * const HWMEngineSensorsHasBenUpdatedNotification;

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

typedef enum {
    kHWMEngineStateIdle = 0,
    kHWMEngineStateActive,
} HWMEngineState;

typedef enum {
    kHWMSensorsUpdateLoopRegular = 0,
    kHWMSensorsUpdateLoopForced,
    kHWMSensorsUpdateLoopOnlyFavorites
} HWMSensorsUpdateLoopStrategy;

@class HWMColorTheme;
@class HWMConfiguration;
@class HWMItem;

@interface HWMEngine : NSObject
{
    NSRecursiveLock *_syncLock;

    NSArray *_platformProfile;
    NSArray *_availableItems;
    NSArray *_arrangedItems;
    NSArray *_graphs;
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

@property (readonly) IBOutlet NSArray * availableItems;
@property (readonly) IBOutlet NSArray * arrangedItems;

+(HWMEngine*)engineWithBundle:(NSBundle*)bundle;

-(void)rebuildSensorsList;
-(void)startEngine;
-(void)stopEngine;
-(void)saveContext;
-(void)updateSmcAndDevicesSensors;
-(void)updateAtaSmartSensors;
-(void)setNeedsRecalculateSensorValues;
-(void)setNeedsUpdateLists;

-(void)insertItemToFavorites:(HWMItem*)item atIndex:(NSUInteger)index;
-(void)removeItemFromFavoritesAtIndex:(NSUInteger)index;

-(HWMColorTheme*)getColorThemeByName:(NSString*)name;
-(HWMColorTheme*)getColorThemeByIndex:(NSUInteger)index;

@end
