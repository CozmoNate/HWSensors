//
//  HWMEngine.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import <Foundation/Foundation.h>

extern NSString * const HWMEngineSensorValuesHasBeenUpdatedNotification;

typedef NS_ENUM(NSUInteger, HWMEngineState) {
    kHWMEngineStateClosed       = 0,
    kHWMEngineStatePaused       = 2,
    kHWMEngineStateActive       = 3,
};

typedef NS_ENUM(NSUInteger, HWMSensorsUpdateLoopStrategy) {
    kHWMSensorsUpdateLoopRegular = 0,
    kHWMSensorsUpdateLoopForced,
    kHWMSensorsUpdateLoopOnlyFavorites,
    kHWMSensorsUpdateLoopStrategiesCount
};

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

    io_connect_t _appleSmcConnection;
    io_connect_t _fakeSmcConnection;

    NSMutableArray *_groups;

    NSArray *_smcAndDevicesSensors;
    NSArray *_ataSmartSensors;
}

@property (nonatomic, strong) NSBundle * bundle;

@property (assign) IBOutlet id <HWMEngineDelegate> delegate;

@property (nonatomic, strong) IBOutlet NSManagedObjectModel * managedObjectModel;
@property (nonatomic, strong) IBOutlet NSPersistentStoreCoordinator * persistentStoreCoordinator;
@property (nonatomic, strong) IBOutlet NSManagedObjectContext * managedObjectContext;
@property (nonatomic, strong) IBOutlet HWMConfiguration * configuration;

@property (nonatomic, assign) HWMSensorsUpdateLoopStrategy updateLoopStrategy;

@property (readonly) HWMEngineState engineState;
@property (readonly) BOOL isRunningOnMac;

@property (readonly) IBOutlet NSArray * iconsWithSensorsAndGroups;
@property (readonly) IBOutlet NSArray * sensorsAndGroups;
@property (readonly) IBOutlet NSArray * graphsAndGroups;
@property (readonly) IBOutlet NSArray * favorites;

+(HWMEngine*)sharedEngine;

-(void)open;
-(void)saveConfiguration;
-(void)updateSmcAndDeviceSensors;
-(void)updateAtaSmartSensors;
-(void)setNeedsRecalculateSensorValues;
-(void)setNeedsUpdateLists;
-(void)setNeedsUpdateSensorLists;
-(void)setNeedsUpdateGraphsList;
-(void)forceDetectSensors;
-(void)start;
-(void)stop;
-(void)close;

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
