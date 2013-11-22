//
//  HWMEngine.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

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

@interface HWMEngine : NSObject
{
    NSArray *_platformProfile;
    NSMutableArray *_availableItems;
    NSMutableArray *_favoriteItems;
    NSMutableArray *_arrangedItems;
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
@property (readonly) IBOutlet NSArray * favoriteItems;
@property (readonly) IBOutlet NSArray * arrangedItems;

+(HWMEngine*)engineWithBundle:(NSBundle*)bundle;

-(void)rebuildSensorsList;
-(void)startEngine;
-(void)stopEngine;
-(void)saveContext;
-(void)updateSmcAndDevicesSensors;
-(void)updateAtaSmartSensors;
-(void)setNeedsUpdateLists;

-(HWMColorTheme*)getColorThemeByName:(NSString*)name;
-(HWMColorTheme*)getColorThemeByIndex:(NSUInteger)index;

@end
