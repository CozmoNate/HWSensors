//
//  HWMEngine.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

extern NSString * const HWMEngineSensorValuesHasBeenUpdatedNotification;

typedef enum {
    kHWMEngineNotInitialized = 0,
    kHWMEngineStateIdle,
    kHWMEngineStateActive,
} HWMEngineState;

typedef enum {
    kHWMSensorsUpdateLoopRegular = 0,
    kHWMSensorsUpdateLoopForced,
    kHWMSensorsUpdateLoopOnlyFavorites,
    
    kHWMSensorsUpdateLoopStrategiesCount
    
} HWMSensorsUpdateLoopStrategy;

@class HWMColorTheme;
@class HWMConfiguration;
@class HWMSensorsGroup;
@class HWMItem;
@class HWMEngine;
@class HWMBatterySensor;

@protocol HWMEngineDelegate <NSObject>
@optional

- (HWMSensorsUpdateLoopStrategy)updateLoopStrategyForEngine:(HWMEngine*)engine;

- (BOOL)engine:(HWMEngine*)engine shouldCaptureSensorValuesToGaphsHistoryWithLimit:(NSUInteger *)limit;

@end

@interface HWMEngine : NSObject
{
@private
    NSString *_platformName;
    NSArray *_platformProfile;

    io_connect_t _smcConnection;
    io_connect_t _fakeSmcConnection;

    NSMutableArray *_groups;

    NSArray *_smcAndDevicesSensors;
    NSTimer *_smcAndDevicesSensorsUpdateLoopTimer;

    NSArray *_ataSmartSensors;
    NSTimer *_ataSmartSensorsUpdateLoopTimer;
}

@property (nonatomic, strong) NSBundle * bundle;

@property (assign) IBOutlet id <HWMEngineDelegate> delegate;

@property (nonatomic, strong) IBOutlet NSManagedObjectModel * managedObjectModel;
@property (nonatomic, strong) IBOutlet NSPersistentStoreCoordinator * persistentStoreCoordinator;
@property (nonatomic, strong) IBOutlet NSManagedObjectContext * managedObjectContext;
@property (nonatomic, strong) IBOutlet HWMConfiguration * configuration;

@property (nonatomic, assign) HWMEngineState engineState;
@property (nonatomic, assign) HWMSensorsUpdateLoopStrategy updateLoopStrategy;

@property (readonly) BOOL isRunningOnMac;

@property (readonly) IBOutlet NSArray * iconsWithSensorsAndGroups;
@property (readonly) IBOutlet NSArray * sensorsAndGroups;
@property (readonly) IBOutlet NSArray * graphsAndGroups;
@property (readonly) IBOutlet NSArray * favorites;

+(HWMEngine*)engineWithBundle:(NSBundle*)bundle;

-(void)rebuildSensorsList;
-(void)startEngine;
-(void)stopEngine;
-(void)saveContext;
-(void)updateSmcAndDeviceSensors;
-(void)updateAtaSmartSensors;
-(void)setNeedsRecalculateSensorValues;
-(void)setNeedsUpdateLists;
-(void)setNeedsUpdateSensorLists;
-(void)setNeedsUpdateGraphsList;

-(void)insertItemIntoFavorites:(HWMItem*)item atIndex:(NSUInteger)index;
-(void)moveFavoritesItemAtIndex:(NSUInteger)fromIndex toIndex:(NSUInteger)toIndex;
-(void)removeItemFromFavoritesAtIndex:(NSUInteger)index;

- (void)systemDidAddBlockStorageDevices:(NSArray*)devices;
- (void)systemDidRemoveBlockStorageDevices:(NSArray*)devices;

-(void)systemDidAddBatteryDevices:(NSArray*)devices;
-(void)systemDidRemoveBatteryDevices:(NSArray*)devices;

-(HWMColorTheme*)getColorThemeByName:(NSString*)name;
-(HWMColorTheme*)getColorThemeByIndex:(NSUInteger)index;

@end
