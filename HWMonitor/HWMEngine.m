//
//  HWMEngine.m
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

#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMIcon.h"
#import "HWMSensorsGroup.h"
#import "HWMSmcSensor.h"
#import "HWMSmcFanSensor.h"
#import "HWMSmcFanController.h"
#import "HWMSmcFanControlLevel.h"
#import "HWMAtaSmartSensor.h"
#import "HWMBatterySensor.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMGraph.h"
#import "HWMGraphsGroup.h"
#import "HWMFavorite.h"
#import "HWMSmcFanController.h"
#import "HWMTimer.h"

#include <sys/sysctl.h>

#import "Localizer.h"

#import "HWMonitorDefinitions.h"
#import "FakeSMCDefinitions.h"

#import "SmcHelper.h"

#import "NSImage+HighResolutionLoading.h"

#import <QuartzCore/QuartzCore.h>

//#define DLog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DLog(...)

NSString * const HWMEngineSensorValuesHasBeenUpdatedNotification = @"HWMEngineSensorsHasBenUpdatedNotification";

@interface HWMEngine ()

@property (readonly, strong) HWMTimer * smcAndDevicesSensorsUpdateLoopTimer;
@property (readonly, strong) HWMTimer * ataSmartSensorsUpdateLoopTimer;

@end

@implementation HWMEngine

@synthesize iconsWithSensorsAndGroups = _iconsWithSensorsAndGroups;
@synthesize sensorsAndGroups = _sensorsAndGroups;
@synthesize graphsAndGroups = _graphsAndGroups;
@synthesize favorites = _favorites;

@synthesize isRunningOnMac = _isRunningOnMac;

@synthesize smcAndDevicesSensorsUpdateLoopTimer = _smcAndDevicesSensorsUpdateLoopTimer;
@synthesize ataSmartSensorsUpdateLoopTimer = _ataSmartSensorsUpdateLoopTimer;

#pragma mark
#pragma mark Global methods

+(HWMEngine*)sharedEngine
{
    static dispatch_once_t onceToken;
    static HWMEngine *sharedEngine;

    dispatch_once(&onceToken, ^{
        sharedEngine = [[HWMEngine alloc] init];
    });

    return sharedEngine;
}

#pragma mark
#pragma mark Properties

-(NSManagedObjectModel *)managedObjectModel
{
    if (!_managedObjectModel) {
        _managedObjectModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:[NSURL fileURLWithPath:[self.bundle pathForResource:@"HWMEngine" ofType:@"momd"]]];
    }

    return _managedObjectModel;
}

-(NSPersistentStoreCoordinator *)persistentStoreCoordinator
{
    if (!_persistentStoreCoordinator && self.managedObjectModel) {

        _persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:self.managedObjectModel];

        NSError *error = nil;

        NSString *path = [[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject] stringByAppendingPathComponent:@"HWMonitor/"];
        NSURL *url= [NSURL fileURLWithPath:[path stringByAppendingPathComponent: @"Configuration.xml"]];

        if (![[NSFileManager defaultManager] fileExistsAtPath:path])
            [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:&error];

        if (![_persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:url options:@{ NSMigratePersistentStoresAutomaticallyOption : @YES, NSInferMappingModelAutomaticallyOption : @YES } error:&error]) {

            NSLog(@"incompatible managed model, deleting database %@...", url.path);

            // try to delete incompatible store
            if (![[NSFileManager defaultManager] removeItemAtURL:url error:&error]) {
                NSLog(@"deleting incompatible database error %@", error);
            }

            if (![_persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:url options:nil error:&error]) {
                NSLog(@"adding new persistent store error %@", error);
            }
        }
    }

    return _persistentStoreCoordinator;
}

-(NSManagedObjectContext*)managedObjectContext
{
    if (!_managedObjectContext) {
        _managedObjectContext = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSMainQueueConcurrencyType];
        _managedObjectContext.persistentStoreCoordinator = self.persistentStoreCoordinator;
    }

    return _managedObjectContext;
}

-(HWMConfiguration *)configuration
{
    if (!_configuration) {
        // Create or load configuration entity
        NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Configuration"];

        NSError *error;

        _configuration = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

        if (error) {
            NSLog(@"failed to retrieve configuration %@", error);
        }

        if (!_configuration) {
            _configuration = [NSEntityDescription insertNewObjectForEntityForName:@"Configuration" inManagedObjectContext:self.managedObjectContext];
            
            _configuration.colorThemeIndex = @0;
        }
    }

    return _configuration;
}

-(HWMSensorsUpdateLoopStrategy)updateLoopStrategy
{
    if (self.delegate && [self.delegate respondsToSelector:@selector(updateLoopStrategyForEngine:)]) {
        return [self.delegate updateLoopStrategyForEngine:self];
    }

    return _updateLoopStrategy;
}

-(NSArray *)iconsWithSensorsAndGroups
{
    if (!_iconsWithSensorsAndGroups) {

        @synchronized (self) {

            NSMutableArray *items = [[NSMutableArray alloc] init];

            // Icons
            NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Icon"];

            NSError *error;

            NSArray *icons = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

            if (error) {
                NSLog(@"fetch icons in iconsWithSensorsAndGroups error %@", error);
            }
            else {
                NSSortDescriptor *titleDescriptor = [[NSSortDescriptor alloc] initWithKey:@"title" ascending:YES];
                [items addObjectsFromArray:[icons sortedArrayUsingDescriptors:@[titleDescriptor]]];
            }

            // Sensors and groups

            for (HWMSensorsGroup *group in self.configuration.sensorGroups) {
                NSArray *sensors = [[group.sensors array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"service != 0"]];

                if (sensors.count) {
                    [items addObject:group];
                    [items addObjectsFromArray:sensors];
                }
            }

            //[self willChangeValueForKey:@"iconsWithSensorsAndGroups"];
            _iconsWithSensorsAndGroups = [items copy];
            //[self didChangeValueForKey:@"iconsWithSensorsAndGroups"];

            //_smcAndDevicesSensors = nil;
            //_ataSmartSensors = nil;
        }
    }

    return _iconsWithSensorsAndGroups;
}

-(NSArray *)sensorsAndGroups
{
    if (!_sensorsAndGroups) {

        @synchronized (self) {

            NSMutableArray *items = [[NSMutableArray alloc] init];

            for (HWMSensorsGroup *group in self.configuration.sensorGroups) {
                NSArray *sensors = [[group.sensors array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"service != 0 AND hidden == NO"]];

                if (sensors && sensors.count) {
                    [items addObject:group];
                    [items addObjectsFromArray:sensors];
                }
            }

            //[self willChangeValueForKey:@"sensorsAndGroups"];
            _sensorsAndGroups = [items copy];
            //[self didChangeValueForKey:@"sensorsAndGroups"];
        }
    }

    return _sensorsAndGroups;
}

-(NSArray *)graphsAndGroups
{
    if (!_graphsAndGroups) {
        
        @synchronized (self) {

            NSMutableArray *items = [[NSMutableArray alloc] init];

            for (HWMGraphsGroup *group in self.configuration.graphGroups) {

                NSArray *graphs = [[group.graphs array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"sensor.service != 0"]];

                if (group.graphs && group.graphs.count) {
                    [items addObject:group];
                    [items addObjectsFromArray:graphs];
                }
            }

            //[self willChangeValueForKey:@"graphsAndGroups"];
            _graphsAndGroups = [items copy];
            //[self didChangeValueForKey:@"graphsAndGroups"];
        }
    }

    return _graphsAndGroups;
}

-(NSArray *)favorites
{
    if (!_favorites) {

        @synchronized (self) {

            NSMutableArray *items = [[NSMutableArray alloc] init];

            for (HWMFavorite *favorite in self.configuration.favorites) {
                if ([favorite.item isKindOfClass:[HWMSensor class]] && ![(HWMSensor*)favorite.item service].unsignedLongLongValue) {
                    continue;
                }

                [items addObject:favorite];
            }

            //[self willChangeValueForKey:@"favorites"];
            _favorites = [items copy];
            //[self didChangeValueForKey:@"favorites"];
        }
    }
    
    return _favorites;
}

-(HWMTimer *)smcAndDevicesSensorsUpdateLoopTimer
{
    if (!_smcAndDevicesSensorsUpdateLoopTimer) {
        _smcAndDevicesSensorsUpdateLoopTimer = [HWMTimer timerWithInterval:self.configuration.smcSensorsUpdateRate.floatValue block:^{
            [self updateSmcAndDeviceSensors];
        }];
    }


    return _smcAndDevicesSensorsUpdateLoopTimer;
}

-(HWMTimer *)ataSmartSensorsUpdateLoopTimer
{
    if (!_ataSmartSensorsUpdateLoopTimer) {
        _ataSmartSensorsUpdateLoopTimer = [HWMTimer timerWithInterval:self.configuration.smartSensorsUpdateRate.floatValue * 60.0f block:^{
            [self updateAtaSmartSensors];
        }];
    }

    return _ataSmartSensorsUpdateLoopTimer;
}

#pragma mark
#pragma mark Overriden Methods

-(instancetype)init;
{
    self = [super init];

    if (self) {
        _bundle = [NSBundle mainBundle];
    }

    return self;
}

-(instancetype)initWithBundle:(NSBundle*)bundle
{
    self = [super init];

    if (self) {
        _bundle = bundle;
    }

    return self;
}

#pragma mark
#pragma mark Private Methods

- (void)assignPlatformProfile
{
    CFDictionaryRef matching = MACH_PORT_NULL;

    _platformName = nil;
    _isRunningOnMac = YES;

    if (MACH_PORT_NULL != (matching = IOServiceMatching("FakeSMCDevice"))) {
        io_iterator_t iterator = IO_OBJECT_NULL;

        if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
            if (IO_OBJECT_NULL != iterator) {

                io_service_t service = MACH_PORT_NULL;

                if (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                    _platformName = @"Hackintosh";
                    _isRunningOnMac = NO;
                    IOObjectRelease(service);
                }

                IOObjectRelease(iterator);
            }
        }
    }

    if (!_platformName) {
        if (MACH_PORT_NULL != (matching = IOServiceMatching("IOPlatformExpertDevice"))) {
            io_iterator_t iterator = IO_OBJECT_NULL;

            if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
                if (IO_OBJECT_NULL != iterator) {

                    io_service_t service = MACH_PORT_NULL;

                    if (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                        _platformName = [[NSString alloc] initWithData:(__bridge_transfer NSData *)IORegistryEntryCreateCFProperty(service, CFSTR("model"), kCFAllocatorDefault, 0) encoding:NSASCIIStringEncoding];

                        IOObjectRelease(service);
                    }

                    IOObjectRelease(iterator);
                }
            }
        }
    }

    NSString *config;

	if ([_platformName hasPrefix:@"MacPro5,1"] == YES || [_platformName hasPrefix:@"MacPro4,1"] == YES)
	{
		int cpuCount;
		size_t count_len;
		sysctlbyname("hw.packages", &cpuCount, &count_len, NULL, 0);

		NSLog(@"found %lu cpus, switching to patched plist", (unsigned long)cpuCount);

		if (cpuCount == 1)
			_platformName = @"MacPro5,1_single";
		else if (cpuCount == 2)
			_platformName = @"MacPro5,1_dual";
	}

	if (_platformName) {
        config = [[NSBundle mainBundle] pathForResource:_platformName ofType:@"plist" inDirectory:@"Profiles"];
    }

    if (!config) {
        config = [[NSBundle mainBundle] pathForResource:@"Default" ofType:@"plist" inDirectory:@"Profiles"];
    }

    if (config) {

        NSLog(@"Loading profile from %@", [config lastPathComponent]);

        NSMutableArray *rawProfile = [[NSMutableArray alloc] initWithContentsOfFile:config];

        if (rawProfile) {

            [rawProfile addObjectsFromArray:[NSArray arrayWithContentsOfURL:[[NSBundle mainBundle] URLForResource:@"FakeSMC" withExtension:@"plist" subdirectory:@"Profiles"]]];

            NSMutableArray *adoptedProfile = [[NSMutableArray alloc] init];

            [rawProfile enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                NSArray *entries = [obj componentsSeparatedByString:@"|"];

                [adoptedProfile addObject:[NSArray arrayWithObjects:[entries objectAtIndex:0], [entries objectAtIndex:entries.count - 1], nil]];
            }];

            _platformProfile = [adoptedProfile copy];
        }
    }

    if (!_platformProfile) {
        NSLog(@"No suitible profile found");
    }
}

-(HWMIcon*)getIconByName:(NSString*)name
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Icon"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];

    NSError *error;

    HWMIcon *icon = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"getIconByName error %@", error);
    }

    return icon;
}

-(HWMIcon*)loadIconNamed:(NSString*)name
{
    return [self loadIconNamed:name asTemplate:YES];
}

-(HWMIcon*)loadIconNamed:(NSString*)name asTemplate:(BOOL)template
{
    HWMIcon *icon = [self getIconByName:name];

    if (!icon) {
        icon = [NSEntityDescription insertNewObjectForEntityForName:@"Icon" inManagedObjectContext:self.managedObjectContext];
    }

    NSImage *image = [NSImage loadImageNamed:name ofType:@"png"];

    if (image) {
        [image setMatchesOnMultipleResolution:YES];
        [image setMatchesOnlyOnBestFittingAxis:YES];
        [image setTemplate:template];
    }

    NSImage *alternate = [NSImage loadImageNamed:[name stringByAppendingString:@"-white"] ofType:@"png"];

    if (alternate) {
        [alternate setMatchesOnMultipleResolution:YES];
        [alternate setMatchesOnlyOnBestFittingAxis:NO];
        [alternate setTemplate:template];
    }

    [icon setName:name];
    [icon setRegular:image];
    [icon setAlternate:alternate];
    [icon setTitle:GetLocalizedString(name)];
    [icon setIdentifier:@"Icon"];

    [icon setEngine:self];

    return icon;
}

-(void)detectSensors
{
    //[[NSOperationQueue mainQueue] addOperationWithBlock:^{

    DLog(@"");

    if (_engineState == kHWMEngineStateActive) {
        [self internalStopEngine];
    }

    NSError *error;

    // SMC SENSORS

    // Add FakeSMCKeyStore keys first
    _fakeSmcConnection = [self insertSmcSensorsWithServiceName:"FakeSMCKeyStore" excludingKeys:nil];

    NSFetchRequest *sensorsFetch = [[NSFetchRequest alloc] initWithEntityName:@"Sensor"];

    // Keys added from FakeSMCKeyStore
    NSArray *excludedKeys = [[[self.managedObjectContext executeFetchRequest:sensorsFetch error:&error] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"service != 0"]] valueForKey:@"name"];

    // Add keys from AppleSMC
    _appleSmcConnection = [self insertSmcSensorsWithServiceName:"AppleSMC" excludingKeys:[NSSet setWithArray:excludedKeys]];

    // Close AppleSMC connection if no keys where obtained from it
    NSArray *appleSmcKeys = [[self.managedObjectContext executeFetchRequest:sensorsFetch error:&error] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"service == %d", _appleSmcConnection]];

    if (appleSmcKeys.count == 0) {
        SMCClose(_appleSmcConnection);
        _appleSmcConnection = 0;
    }

    // Update graphs
    [self insertGraphs];

    // Save context
    [self saveConfiguration];

    [self setNeedsUpdateLists];

    if (_engineState == kHWMEngineStateActive) {
        [self internalStartEngine];
    }

    //}];
}


/**
 *  Start initialized engine. This method doesn't check and change engine state
 */
-(void)internalStartEngine
{
    //if (_engineState == kHWMEngineStateIdle) {
    [HWMAtaSmartSensor startWatchingForBlockStorageDevicesWithEngine:self];
    [HWMBatterySensor startWatchingForBatteryDevicesWithEngine:self];

    [self updateSmcAndDeviceSensors];
    [self updateAtaSmartSensors];

    [self.smcAndDevicesSensorsUpdateLoopTimer resume];
    [self.ataSmartSensorsUpdateLoopTimer resume];
    //}
}

/**
 *  Stop active engine; stop timers, close connections and device watchers. Doesn't check and change engine state
 */
-(void)internalStopEngine
{
    //if (_engineState == kHWMEngineStateActive) {
    [self.smcAndDevicesSensorsUpdateLoopTimer suspend];
    [self.ataSmartSensorsUpdateLoopTimer suspend];

    [HWMAtaSmartSensor stopWatchingForBlockStorageDevices];
    [HWMBatterySensor stopWatchingForBatteryDevices];
    //}
}

-(void)internalCaptureSensorValuesToGraphs
{
    if (!self.delegate || (self.delegate && [self.delegate respondsToSelector:@selector(engine:shouldCaptureSensorValuesToGaphsHistoryWithLimit:)])) {

        NSUInteger limit;

        if ([self.delegate engine:self shouldCaptureSensorValuesToGaphsHistoryWithLimit:&limit] || self.configuration.updateSensorsInBackground.boolValue) {
            for (HWMGraphsGroup *graphsGroup in self.configuration.graphGroups) {
                [graphsGroup captureSensorValuesToGraphsHistorySetLimit:limit];
            }
        }
    }
}

#pragma mark
#pragma mark Public Methods

- (void)open
{
    DLog(@"");

    if (_engineState > kHWMEngineStateClosed) {
        return;
    }

    [self assignPlatformProfile];

    // Create color themes
    [self insertColorThemes];

    // Load icons
    [self loadIconNamed:@"red-thermometer" asTemplate:NO];

    [self loadIconNamed:kHWMonitorIconHWMonitor asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconThermometer asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconScale asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconDevice asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconTemperatures asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconHddTemperatures asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconSsdLife asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconMultipliers asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconFrequencies asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconTachometers asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconVoltages asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconBattery asTemplate:NO];

    NSError *error;

    // Cleanup icons
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Icon"];

    NSArray *icons = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

    [icons enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        HWMIcon *icon = (HWMIcon*)obj;

        if (!icon.regular && !icon.alternate) {
            [self.managedObjectContext deleteObject:obj];
        }
    }];

    // Update groups
    [self insertGroups];

    // Detect sensors
    [self detectSensors];

    _engineState = kHWMEngineStatePaused;

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(workspaceDidMountOrUnmount:) name:NSWorkspaceDidMountNotification object:nil];
	[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(workspaceDidMountOrUnmount:) name:NSWorkspaceDidUnmountNotification object:nil];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(workspaceDidMountOrUnmount:) name:NSWorkspaceDidRenameVolumeNotification object:nil];

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(workspaceWillSleep:) name:NSWorkspaceWillSleepNotification object:nil];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(workspaceDidWake:) name:NSWorkspaceDidWakeNotification object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];

    [self addObserver:self forKeyPath:@keypath(self, configuration.useFahrenheit) options:0 context:nil];
    [self addObserver:self forKeyPath:@keypath(self, configuration.smcSensorsUpdateRate) options:0 context:nil];
    [self addObserver:self forKeyPath:@keypath(self, configuration.smartSensorsUpdateRate) options:0 context:nil];
}

-(void)start
{
    DLog(@"");
    if (_engineState == kHWMEngineStatePaused) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            _engineState = kHWMEngineStateActive;
            [self internalStartEngine];
        }];
    }
}

-(void)stop
{
    DLog(@"");
    if (_engineState == kHWMEngineStateActive) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            _engineState = kHWMEngineStatePaused;
            [self internalStopEngine];
        }];
    }
}

-(void)close
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        DLog(@"");

        if (_engineState < kHWMEngineStatePaused) {
            return;
        }

        [self saveConfiguration];

        [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        [self removeObserver:self forKeyPath:@keypath(self, configuration.useFahrenheit)];
        [self removeObserver:self forKeyPath:@keypath(self, configuration.smcSensorsUpdateRate)];
        [self removeObserver:self forKeyPath:@keypath(self, configuration.smartSensorsUpdateRate)];

        for (HWMSensor *sensor in _smcAndDevicesSensors) {
            if (sensor.service && sensor.service.unsignedLongLongValue) {
                IOObjectRelease((io_object_t)sensor.service.unsignedLongLongValue);
                sensor.service = @0;
            }
        }

        if (_appleSmcConnection) {
            SMCClose(_appleSmcConnection);
            _appleSmcConnection = 0;
        }

        if (_fakeSmcConnection) {
            SMCClose(_fakeSmcConnection);
            _fakeSmcConnection = 0;
        }

        _configuration = nil;
        _managedObjectContext = nil;
        _managedObjectModel = nil;
        _persistentStoreCoordinator = nil;
    }];
}

-(void)saveConfiguration
{
    DLog(@"");

    if (_engineState < kHWMEngineStatePaused) {
        return;
    }

    NSError *error;

    if (![self.managedObjectContext save:&error]) {
        NSLog(@"failed to save context %@", error);
    }
}

-(void)forceDetectSensors
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        DLog(@"");

        if (_engineState < kHWMEngineStatePaused) {
            return;
        }

        [self detectSensors];
    }];
}

-(void)updateSmcAndDeviceSensors
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        //DLog(@"");

        if (_engineState < kHWMEngineStatePaused) {
            return;
        }

        if (!_smcAndDevicesSensors) {

            __block NSMutableArray *sensors = [NSMutableArray array];

            for (HWMSensorsGroup *group in _groups) {
                for (HWMSensor *sensor in group.sensors) {
                    if (![sensor isKindOfClass:[HWMAtaSmartSensor class]] && [sensor isActive]) {
                        [sensors addObject:sensor];
                    }
                }
            }

            _smcAndDevicesSensors = [sensors copy];
        }

        NSUInteger updatedCount = 0;
        NSTimeInterval nineTenths = self.configuration.smcSensorsUpdateRate.floatValue * 0.9f;

        for (HWMSensor *sensor in _smcAndDevicesSensors) {

            if (sensor.timeIntervalSinceLastUpdate < nineTenths) {
                continue;
            }

            BOOL doUpdate = sensor.forced.boolValue || sensor.consumers.count || sensor.controller || self.configuration.updateSensorsInBackground.boolValue;

            if (!doUpdate) {
                switch (self.updateLoopStrategy) {
                    case kHWMSensorsUpdateLoopForced:
                        doUpdate = YES;
                        break;

                    case kHWMSensorsUpdateLoopOnlyFavorites:
                        doUpdate = sensor.favorites.count;
                        break;

                    case kHWMSensorsUpdateLoopRegular:
                    default:
                        doUpdate = !sensor.hidden.boolValue || sensor.favorites.count;
                        break;
                }
            }

            if (doUpdate) {
                [sensor doUpdateValue];
                updatedCount++;
            }
        }

        if (updatedCount) {
            [self internalCaptureSensorValuesToGraphs];
            [[NSNotificationCenter defaultCenter] postNotificationName:HWMEngineSensorValuesHasBeenUpdatedNotification object:self];
        }
    }];
}

-(void)updateAtaSmartSensors
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{

        DLog(@"");

        if (_engineState < kHWMEngineStatePaused) {
            return;
        }

        if (!_ataSmartSensors) {

            NSMutableArray *sensors = [NSMutableArray array];

            for (HWMSensorsGroup *group in _groups) {
                for (HWMSensor *sensor in group.sensors) {
                    if ([sensor isKindOfClass:[HWMAtaSmartSensor class]] && [sensor isActive]) {
                        [sensors addObject:sensor];
                    }
                }
            }

            _ataSmartSensors = [sensors copy];
        }

        if (_ataSmartSensors) {

            NSUInteger updatedCount = 0;
            NSTimeInterval nineTenths = self.configuration.smartSensorsUpdateRate.floatValue * 60 * 0.9f;

            for (HWMAtaSmartSensor *sensor in _ataSmartSensors) {

                if (sensor.timeIntervalSinceLastUpdate < nineTenths) {
                    continue;
                }

                BOOL doUpdate = sensor.forced.boolValue || sensor.consumers.count || sensor.controller || self.configuration.updateSensorsInBackground.boolValue;

                if (!doUpdate) {
                    switch (self.updateLoopStrategy) {
                        case kHWMSensorsUpdateLoopForced:
                            doUpdate = YES;
                            break;

                        case kHWMSensorsUpdateLoopOnlyFavorites:
                            doUpdate = sensor.favorites.count;
                            break;

                        case kHWMSensorsUpdateLoopRegular:
                        default:
                            doUpdate = !sensor.hidden.boolValue || sensor.favorites.count;
                            break;
                    }
                }

                if (doUpdate) {
                    [sensor doUpdateValue];
                    updatedCount++;
                }
            }

            if (updatedCount) {
                [HWMATASmartInterfaceWrapper destroyAllWrappers];
                [self internalCaptureSensorValuesToGraphs];
                [[NSNotificationCenter defaultCenter] postNotificationName:HWMEngineSensorValuesHasBeenUpdatedNotification object:self];
            }
        }
    });
}

-(void)setNeedsUpdateLists
{
    DLog(@"");

    [self setNeedsUpdateSensorLists];
    [self setNeedsUpdateGraphsList];
}

-(void)setNeedsUpdateSensorLists
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        DLog(@"");

        if (_engineState < kHWMEngineStatePaused) {
            return;
        }

        _smcAndDevicesSensors = nil;
        _ataSmartSensors = nil;

        [self willChangeValueForKey:@keypath(self, iconsWithSensorsAndGroups)];
        _iconsWithSensorsAndGroups = nil;
        [self didChangeValueForKey:@keypath(self, iconsWithSensorsAndGroups)];

        [self willChangeValueForKey:@keypath(self, sensorsAndGroups)];
        _sensorsAndGroups = nil;
        [self didChangeValueForKey:@keypath(self, sensorsAndGroups)];

        [self willChangeValueForKey:@keypath(self, favorites)];
        _favorites = nil;
        [self didChangeValueForKey:@keypath(self, favorites)];
    }];
}

-(void)setNeedsUpdateGraphsList
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        DLog(@"");

        if (_engineState < kHWMEngineStatePaused) {
            return;
        }

        [self willChangeValueForKey:@keypath(self, graphsAndGroups)];
        _graphsAndGroups = nil;
        [self didChangeValueForKey:@keypath(self, graphsAndGroups)];
    }];
}

-(void)setNeedsRecalculateSensorValues
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        DLog(@"");

        if (_engineState < kHWMEngineStatePaused) {
            return;
        }

        NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Sensor"];

        NSError *error;

        NSArray *sensors = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

        if (error) {
            NSLog(@"fetch sensors in setNeedsRecalculateSensorValues error %@", error);
        }

        if (sensors) {
            [sensors enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                HWMSensor *sensor = obj;

                [obj willChangeValueForKey:@keypath(sensor, value)];
                [obj didChangeValueForKey:@keypath(sensor, value)];
                [obj willChangeValueForKey:@keypath(sensor, formattedValue)];
                [obj didChangeValueForKey:@keypath(sensor, formattedValue)];
                [obj willChangeValueForKey:@keypath(sensor, strippedValue)];
                [obj didChangeValueForKey:@keypath(sensor, strippedValue)];
            }];
        }

        [[NSNotificationCenter defaultCenter] postNotificationName:HWMEngineSensorValuesHasBeenUpdatedNotification object:self];
    }];
}

#pragma mark
#pragma mark Favorites

-(void)insertItemIntoFavorites:(HWMItem*)item atIndex:(NSUInteger)index
{
    if (item && item.managedObjectContext == _managedObjectContext) {

            if ([item isKindOfClass:[HWMSensor class]] && item.favorites.count) {
                return;
            }

            HWMFavorite * favorite = [NSEntityDescription insertNewObjectForEntityForName:@"Favorite" inManagedObjectContext:self.managedObjectContext];

            [favorite setItem:item];

            [[self.configuration mutableOrderedSetValueForKey:@keypath(self, favorites)] insertObject:favorite atIndex:index];

            [self willChangeValueForKey:@keypath(self, favorites)];
            _favorites = nil;
            [self didChangeValueForKey:@keypath(self, favorites)];

    }
}

-(void)moveFavoritesItemAtIndex:(NSUInteger)fromIndex toIndex:(NSUInteger)toIndex
{
    [[self.configuration mutableOrderedSetValueForKey:@keypath(self, favorites)] moveObjectsAtIndexes:[NSIndexSet indexSetWithIndex:fromIndex] toIndex:toIndex > fromIndex ? toIndex - 1 : toIndex < self.configuration.favorites.count ? toIndex : self.configuration.favorites.count];

    [self willChangeValueForKey:@keypath(self, favorites)];
    _favorites = nil;
    [self didChangeValueForKey:@keypath(self, favorites)];
}

-(void)removeItemFromFavoritesAtIndex:(NSUInteger)index
{
        if (!self.configuration.favorites.count)
            return;

        HWMFavorite *favorite = [self.configuration.favorites objectAtIndex:index];

        [[self.configuration mutableOrderedSetValueForKey:@"favorites"] removeObjectAtIndex:index];

        [self.managedObjectContext deleteObject:favorite];

        [self willChangeValueForKey:@"favorites"];
        _favorites = nil;
        [self didChangeValueForKey:@"favorites"];
}

#pragma mark
#pragma mark Events

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@keypath(self, configuration.useFahrenheit)]) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self setNeedsRecalculateSensorValues];
        }];
    }
    else if ([keyPath isEqual:@keypath(self, configuration.smcSensorsUpdateRate)]) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self.smcAndDevicesSensorsUpdateLoopTimer setInterval:self.configuration.smcSensorsUpdateRate.floatValue];
        }];
    }
    else if ([keyPath isEqual:@keypath(self, configuration.smartSensorsUpdateRate)]) {
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [self.ataSmartSensorsUpdateLoopTimer setInterval:self.configuration.smartSensorsUpdateRate.floatValue * 60.0f];
        }];
    }
}

-(void)workspaceDidMountOrUnmount:(id)sender
{
//    // Update SMART sensors
//    [self insertAtaSmartSensors];

    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"AtaSmartSensor"];

    NSError *error;

    NSArray *sensors = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

    if (!error && sensors && sensors.count) {

        [HWMAtaSmartSensor updatePartitionsList];

        for (HWMAtaSmartSensor *sensor in sensors) {
            [sensor updateVolumeNames];
        }

        // Force update some sensors info
        [self.configuration willChangeValueForKey:@keypath(self.configuration, driveNameSelector)];
        [self.configuration didChangeValueForKey:@keypath(self.configuration, driveNameSelector)];
        //[_configuration willChangeValueForKey:@keypath(_configuration, showSensorLegendsInPopup)];
        //[_configuration didChangeValueForKey:@keypath(_configuration, showSensorLegendsInPopup)];

        [self setNeedsUpdateLists];
    }
}

-(void)workspaceWillSleep:(id)sender
{
    [self saveConfiguration];

    if (_engineState == kHWMEngineStateActive) {
        [self internalStopEngine];
    }
}

-(void)workspaceDidWake:(id)sender
{
    [self forceDetectSensors];
}

- (void)systemDidAddBlockStorageDevices:(NSArray*)devices
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        HWMSensorsGroup *smartTemperatures = [self getGroupBySelector:kHWMGroupSmartTemperature];
        HWMSensorsGroup *smartRemainingLife = [self getGroupBySelector:kHWMGroupSmartRemainingLife];

        for (NSDictionary *properties in devices) {
            [self insertAtaSmartSensorFromDictionary:properties group:smartTemperatures];
            [self insertAtaSmartSensorFromDictionary:properties group:smartRemainingLife];
        }

        [HWMATASmartInterfaceWrapper destroyAllWrappers];

        // Update graphs
        [self insertGraphs];

        [self setNeedsUpdateLists];
    }];
}

- (void)systemDidRemoveBlockStorageDevices:(NSArray*)devices
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        HWMSensorsGroup *smartTemperatures = [self getGroupBySelector:kHWMGroupSmartTemperature];
        HWMSensorsGroup *smartRemainingLife = [self getGroupBySelector:kHWMGroupSmartRemainingLife];

        NSMutableArray *sensors = [NSMutableArray arrayWithArray:[[smartTemperatures.sensors array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"service IN %@", devices]]];

        [sensors addObjectsFromArray:[[smartRemainingLife.sensors array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"service IN %@", devices]]];

        if (sensors) {
            for (HWMAtaSmartSensor *sensor in sensors) {

                NSLog(@"removed ATA block storage device %@ (%@)", sensor.name, sensor.service);

                [sensor setService:@0];
            }
        }

        // Update graphs
        [self insertGraphs];
        
        [self setNeedsUpdateLists];
    }];
}

- (void)systemDidAddBatteryDevices:(NSArray*)devices
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        HWMSensorsGroup *group = [self getGroupBySelector:kHWMGroupBattery];

        if (group) {

            for (NSDictionary *properties in devices) {
                [self insertBatterySensorFromDictionary:properties group:group];
            }

            [self setNeedsUpdateLists];
        }
    }];
}

- (void)systemDidRemoveBatteryDevices:(NSArray*)devices
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        HWMSensorsGroup *group = [self getGroupBySelector:kHWMGroupBattery];

        if (group) {

            NSArray *sensors = [[group.sensors array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"service IN %@", devices]];

            if (sensors) {
                for (HWMBatterySensor *sensor in sensors) {
                    
                    NSLog(@"removed battery device %@ (%@)", sensor.name, sensor.service);

                    [sensor setService:@0];
                }
            }


            [self setNeedsUpdateLists];
        }
    }];
}

- (void)applicationWillTerminate:(id)sender
{
    [self removeObserver:self forKeyPath:@keypath(self, configuration.useFahrenheit)];
    [self removeObserver:self forKeyPath:@keypath(self, configuration.smcSensorsUpdateRate)];
    [self removeObserver:self forKeyPath:@keypath(self, configuration.smartSensorsUpdateRate)];

    if (self.engineState == kHWMEngineStateActive) {
        [self internalStopEngine];
    }

    [self saveConfiguration];

    [self close];
}

#pragma mark
#pragma mark Color Themes

-(HWMColorTheme*)getColorThemeByName:(NSString*)name
{
    NSArray *themes = [[self.configuration.colorThemes array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];

    return themes && themes.count ? [themes objectAtIndex:0] : nil;
}

-(HWMColorTheme*)getColorThemeByIndex:(NSUInteger)index
{
    return [self.configuration.colorThemes objectAtIndex:index];
    /*NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"ColorTheme"];

     [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"order == %d", index]];

     NSError *error;

     HWMColorTheme *colorTheme = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

     if (error) {
     NSLog(@"getColorThemeByIndex error %@", error);
     }

     return colorTheme;*/
}

-(HWMColorTheme*)insertColorThemeWithName:(NSString*)name
                            groupEndColor:(NSColor*)groupEndColor
                          groupStartColor:(NSColor*)groupStartColor
                          groupTitleColor:(NSColor*)groupTitleColor
                        itemSubTitleColor:(NSColor*)itemSubTitleColor
                           itemTitleColor:(NSColor*)itemTitleColor
                      itemValueTitleColor:(NSColor*)itemValueTitleColor
                      listBackgroundColor:(NSColor*)listBackgroundColor
                          listStrokeColor:(NSColor*)listStrokeColor
                          toolbarEndColor:(NSColor*)toolbarEndColor
                       toolbarShadowColor:(NSColor*)toolbarShadowColor
                        toolbarStartColor:(NSColor*)toolbarStartColor
                       toolbarStrokeColor:(NSColor*)toolbarStrokeColor
                        toolbarTitleColor:(NSColor*)toolbarTitleColor
                             useDarkIcons:(BOOL)useDarkIcons
{
    HWMColorTheme *colorTheme = [self getColorThemeByName:name];

    if (!colorTheme) {
        colorTheme = [NSEntityDescription insertNewObjectForEntityForName:@"ColorTheme" inManagedObjectContext:self.managedObjectContext];

        [colorTheme setName:name];
        [colorTheme setConfiguration:self.configuration];
    }

    [colorTheme setGroupEndColor:groupEndColor];
    [colorTheme setGroupStartColor:groupStartColor];
    [colorTheme setGroupTitleColor:groupTitleColor];
    [colorTheme setItemSubTitleColor:itemSubTitleColor];
    [colorTheme setItemTitleColor:itemTitleColor];
    [colorTheme setItemValueTitleColor:itemValueTitleColor];
    [colorTheme setListBackgroundColor:listBackgroundColor];
    [colorTheme setListStrokeColor:listStrokeColor];
    [colorTheme setToolbarEndColor:toolbarEndColor];
    [colorTheme setToolbarShadowColor:toolbarShadowColor];
    [colorTheme setToolbarStartColor:toolbarStartColor];
    [colorTheme setToolbarStrokeColor:toolbarStrokeColor];
    [colorTheme setToolbarTitleColor:toolbarTitleColor];
    [colorTheme setUseBrightIcons:[NSNumber numberWithBool:useDarkIcons]];

    return colorTheme;
}

-(void)insertColorThemes
{
    CGFloat defaultOpacity = 0.8;
    CGFloat darkVibrantOpacity = 0.5;

    [self insertColorThemeWithName:@"Default"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.85 alpha:0.35]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.95 alpha:0.35]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.2 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.45 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.0 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:1.0 alpha:defaultOpacity]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.15]
                   toolbarEndColor:[NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:defaultOpacity]
                toolbarShadowColor:[[NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:defaultOpacity] highlightWithLevel:0.4]
                 toolbarStartColor:[[NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:defaultOpacity] highlightWithLevel:0.6]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.25]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:NO];

    [self insertColorThemeWithName:@"Gray"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.85 alpha:0.35]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.95 alpha:0.35]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.2 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.45 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.0 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:1.0 alpha:defaultOpacity]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.15]
                   toolbarEndColor:[NSColor colorWithCalibratedWhite:0.23 alpha:defaultOpacity]
                toolbarShadowColor:[[NSColor colorWithCalibratedWhite:0.23 alpha:defaultOpacity] highlightWithLevel:0.30]
                 toolbarStartColor:[[NSColor colorWithCalibratedWhite:0.23 alpha:defaultOpacity] highlightWithLevel:0.55]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.25]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:NO];

    [self insertColorThemeWithName:@"Dark"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.1 alpha:0.15]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.2 alpha:0.15]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.85 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.65 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.85 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.95 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:0.15 alpha:darkVibrantOpacity]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.15]
                   toolbarEndColor:[NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:darkVibrantOpacity]
                toolbarShadowColor:[[NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:darkVibrantOpacity] highlightWithLevel:0.30]
                 toolbarStartColor:[[NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:darkVibrantOpacity] highlightWithLevel:0.55]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.15]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:YES];
}

#pragma mark
#pragma mark Groups

-(HWMSensorsGroup*)getGroupBySelector:(NSUInteger)selector
{
    NSArray *groups = [[self.configuration.sensorGroups array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"selector == %d", selector]];
    return groups && groups.count ? [groups objectAtIndex:0] : nil;
}

-(HWMSensorsGroup*)insertGroupWithSelector:(NSUInteger)selector name:(NSString*)name icon:(HWMIcon*)icon
{
    HWMSensorsGroup *group = [self getGroupBySelector:selector];

    if (!group) {
        group = [NSEntityDescription insertNewObjectForEntityForName:@"SensorsGroup" inManagedObjectContext:self.managedObjectContext];

        [group setConfiguration:self.configuration];
    }

    [group setName:name];
    [group setTitle:GetLocalizedString(group.name)];
    [group setIcon:icon];
    [group setSelector:[NSNumber numberWithInteger:selector]];
    [group setIdentifier:@"Group"];

    [group setEngine:self];

    if (!_groups) {
        _groups = [NSMutableArray array];
    }

    [_groups addObject:group];

    return group;
}

-(void)insertGroups
{
    [self insertGroupWithSelector:kHWMGroupTemperature name:@"TEMPERATURES" icon:[self getIconByName:kHWMonitorIconTemperatures]];
    [self insertGroupWithSelector:kHWMGroupSmartTemperature name:@"DRIVE TEMPERATURES" icon:[self getIconByName:kHWMonitorIconHddTemperatures]];
    [self insertGroupWithSelector:kHWMGroupSmartRemainingLife name:@"SSD REMAINING LIFE" icon:[self getIconByName:kHWMonitorIconSsdLife]];
    [self insertGroupWithSelector:kHWMGroupFrequency name:@"FREQUENCIES" icon:[self getIconByName:kHWMonitorIconFrequencies]];
    [self insertGroupWithSelector:kHWMGroupTachometer name:@"FANS & PUMPS" icon:[self getIconByName:kHWMonitorIconTachometers]];
    [self insertGroupWithSelector:kHWMGroupVoltage name:@"VOLTAGES" icon:[self getIconByName:kHWMonitorIconVoltages]];
    [self insertGroupWithSelector:kHWMGroupCurrent name:@"CURRENTS" icon:[self getIconByName:kHWMonitorIconVoltages]];
    [self insertGroupWithSelector:kHWMGroupPower name:@"POWER CONSUMPTION" icon:[self getIconByName:kHWMonitorIconVoltages]];
    [self insertGroupWithSelector:kHWMGroupBattery name:@"BATTERIES" icon:[self getIconByName:kHWMonitorIconBattery]];
}

#pragma mark
#pragma mark Sensors

-(id)getSensorByName:(NSString*)name fromGroup:(HWMSensorsGroup*)group
{
    NSArray *sensors = [[group.sensors array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];

    return sensors && sensors.count ? [sensors objectAtIndex:0] : nil;
}

#pragma mark
#pragma mark SMC Sensors

- (NSArray*)getSmcKeysFromConnection:(io_connect_t)connection excludedList:(NSArray*)excluded
{
    if (!connection)
        return nil;

    NSMutableArray *array = [[NSMutableArray alloc] init];

    UInt32 count = [SmcHelper readNumericKey:@"#KEY" connection:connection].unsignedIntValue;

    for (UInt32 index = 0; index < count; index++) {
        SMCKeyData_t  inputStructure;
        SMCKeyData_t  outputStructure;
        SMCVal_t val;
        
        memset(&inputStructure, 0, sizeof(SMCKeyData_t));
        memset(&outputStructure, 0, sizeof(SMCKeyData_t));
        memset(&val, 0, sizeof(SMCVal_t));

        inputStructure.data8 = SMC_CMD_READ_INDEX;
        inputStructure.data32 = index;

        if (kIOReturnSuccess == SMCCall(connection, KERNEL_INDEX_SMC, &inputStructure, &outputStructure)) {
            NSString *key = [NSString stringWithFormat:@"%c%c%c%c",
                             (unsigned int) outputStructure.key >> 24,
                             (unsigned int) outputStructure.key >> 16,
                             (unsigned int) outputStructure.key >> 8,
                             (unsigned int) outputStructure.key];


            if (!excluded || NSNotFound == [excluded indexOfObject:key]) {
                [array addObject:key];
            }
        }
    }

    return array;
}

-(HWMSmcSensor*)insertSmcSensorWithConnection:(io_connect_t)connection name:(NSString*)name type:(NSString*)type title:(NSString*)title selector:(NSUInteger)selector group:(HWMSensorsGroup*)group
{
    HWMSmcSensor *sensor = [self getSensorByName:name fromGroup:group];

    if (!sensor) {
        sensor = [NSEntityDescription insertNewObjectForEntityForName:@"SmcSensor" inManagedObjectContext:self.managedObjectContext];

        [sensor setName:name];
        [sensor setGroup:group];
    }

    [sensor setService:[NSNumber numberWithLongLong:connection]];

    [sensor doUpdateValue];

    if (sensor.value) {
        [sensor setType:type];
        [sensor setTitle:title];
        [sensor setSelector:[NSNumber numberWithUnsignedInteger:selector]];
        [sensor setIdentifier:@"Sensor"];

        [sensor setEngine:self];
    }
    else {
        //[self.managedObjectContext deleteObject:sensor];
        [sensor setService:@0];
        return nil;
    }

    return sensor;
}

-(void)insertSmcSensorsWithKeys:(NSSet*)keys connection:(io_connect_t)connection selector:(NSUInteger)selector
{
    if (!connection || !selector)
        return;

    NSString *prefix = nil;
    HWMSensorsGroup *group = nil;

    switch (selector) {
        case kHWMGroupTemperature:
            prefix = @"T";
            group = [self getGroupBySelector:kHWMGroupTemperature];
            break;

        case kHWMGroupMultiplier:
            prefix = @"M";
            group = [self getGroupBySelector:kHWMGroupFrequency];
            break;

        case kHWMGroupFrequency:
            prefix = @"C";
            group = [self getGroupBySelector:kHWMGroupFrequency];
            break;

        case kHWMGroupVoltage:
            prefix = @"V";
            group = [self getGroupBySelector:kHWMGroupVoltage];
            break;

        case kHWMGroupCurrent:
            prefix = @"I";
            group = [self getGroupBySelector:kHWMGroupCurrent];
            break;

        case kHWMGroupPower:
            prefix = @"P";
            group = [self getGroupBySelector:kHWMGroupPower];
            break;

        default:
            return;
    }

    [_platformProfile enumerateObjectsUsingBlock:^(id item, NSUInteger index, BOOL *stop) {

        NSString *key = [item objectAtIndex:0];

        if ([key hasPrefix:prefix]) {
            NSString *title = [item objectAtIndex:1];

            NSRange formater = [key rangeOfString:@":"];

            if (formater.length) {
                // key and title should be formatted by index
                unsigned int start = 0;
                [[NSScanner scannerWithString:[key substringWithRange:NSMakeRange(formater.location + 1, 1)]] scanHexInt:&start];
                unsigned int count = 0;
                [[NSScanner scannerWithString:[key substringWithRange:NSMakeRange(formater.location + 2, 1)]] scanHexInt:&count];
                unsigned int shift = 0;
                [[NSScanner scannerWithString:[key substringWithRange:NSMakeRange(formater.location + 3, 1)]] scanHexInt:&shift];

                NSString *keyFormat = [NSString stringWithFormat:@"%@%%X%@", [key substringToIndex:formater.location], [key substringFromIndex:formater.location + formater.length + 3]];

                for (NSUInteger offset = 0; offset < count; offset++) {

                    NSString *formattedKey = [NSString stringWithFormat:keyFormat, start + offset];

                    if ([keys containsObject:formattedKey]) {

                        SMCVal_t info;

                        if (kIOReturnSuccess == SMCReadKey(connection, [formattedKey cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                            [self insertSmcSensorWithConnection:connection name:formattedKey type:[NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding] title:[NSString stringWithFormat:GetLocalizedString(title), shift + offset] selector:selector group:group];

                        }
                    }
                }
            }
            else if ([keys containsObject:key]) {

                SMCVal_t info;

                if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {
                    [self insertSmcSensorWithConnection:connection name:key type:[NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding] title:GetLocalizedString(title) selector:selector group:group];

                }
            }
        }

    }];
}

-(void)insertSmcSensorsWithKeys:(NSSet*)keys connection:(io_connect_t)connection
{
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupTemperature];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupMultiplier];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupFrequency];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupVoltage];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupCurrent];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupPower];
}

-(io_connect_t)insertSmcSensorsWithServiceName:(const char*)service excludingKeys:(NSSet*)excludedKeys
{
    io_connect_t connection;

    if (kIOReturnSuccess == SMCOpen(service, &connection)) {

        NSMutableSet *keys = [[NSMutableSet alloc] init];

        UInt32 count = [SmcHelper readNumericKey:@"#KEY" connection:connection].unsignedIntValue;

        for (UInt32 index = 0; index < count; index++) {
            SMCKeyData_t  inputStructure;
            SMCKeyData_t  outputStructure;
            SMCVal_t val;

            memset(&inputStructure, 0, sizeof(SMCKeyData_t));
            memset(&outputStructure, 0, sizeof(SMCKeyData_t));
            memset(&val, 0, sizeof(SMCVal_t));

            inputStructure.data8 = SMC_CMD_READ_INDEX;
            inputStructure.data32 = index;

            if (kIOReturnSuccess == SMCCall(connection, KERNEL_INDEX_SMC, &inputStructure, &outputStructure)) {
                [keys addObject:[NSString stringWithFormat:@"%c%c%c%c",
                                 (unsigned int) outputStructure.key >> 24,
                                 (unsigned int) outputStructure.key >> 16,
                                 (unsigned int) outputStructure.key >> 8,
                                 (unsigned int) outputStructure.key]];
            }
        }

        NSMutableSet *strippedKeys = keys.mutableCopy;

        [strippedKeys minusSet:excludedKeys];

        if (keys.count) {
            [self insertSmcSensorsWithKeys:strippedKeys connection:connection];
            [self insertSmcFansWithConnection:connection keys:keys];
            [self insertSmcGpuFansWithConnection:connection keys:keys];
        }
        
        return connection;
    }
    
    return 0;
}

#pragma mark
#pragma mark SMC Fan Sensors

-(HWMSmcFanSensor*)getSmcFanSensorByDescriptor:(NSString*)descriptor fromGroup:(HWMSensorsGroup*)group
{
    NSArray *sensors = [[group.sensors array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"descriptor == %@", descriptor]];

    return sensors && sensors.count ? [sensors objectAtIndex:0] : nil;
}


-(HWMSmcSensor*)insertSmcFanWithConnection:(io_connect_t)connection descriptor:(NSString*)descriptor name:(NSString*)name type:(NSString*)type title:(NSString*)title selector:(NSUInteger)selector group:(HWMSensorsGroup*)group
{
    __block HWMSmcFanSensor *fan = [self getSmcFanSensorByDescriptor:descriptor fromGroup:group];

    BOOL newFan = NO;

    if (!fan) {
        fan = [NSEntityDescription insertNewObjectForEntityForName:@"SmcFanSensor" inManagedObjectContext:self.managedObjectContext];

        [fan setName:name];
        [fan setDescriptor:descriptor];
        [fan setGroup:group];

        newFan = YES;
    }
    else if (fan.service.unsignedLongLongValue != 0) {
        return fan;
    }

    [fan setTitle:title];
    [fan setType:type];
    [fan setSelector:[NSNumber numberWithUnsignedInteger:selector]];
    [fan setIdentifier:@"Fan"];

    [fan setService:[NSNumber numberWithLongLong:connection]];

    [fan setEngine:self];

    [fan doUpdateValue];

    int index = [SmcHelper getIndexFromHexChar:[fan.name characterAtIndex:1]];

    if (index >= 0) {

        [fan setNumber:[NSNumber numberWithInt:index]];

        NSNumber *min = [SmcHelper readNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_MIN, index] connection:connection];

        NSNumber *max = [SmcHelper readNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_MAX, index] connection:connection];

        if (newFan || !fan.controller) {
            if (min && max && [max isGreaterThan:min]) {
                HWMSmcFanController *controller = [NSEntityDescription insertNewObjectForEntityForName:@"SmcFanController" inManagedObjectContext:self.managedObjectContext];

                [controller setMin:min];
                [controller setMax:max];

                [fan setController:controller];

                if (self.isRunningOnMac) {

                    NSNumber *target = [SmcHelper readNumericKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_TARGET, index] connection:connection];

                    [controller addOutputLevel:target forInputLevel:@30];
                }
                else {
                    [controller addOutputLevel:fan.value forInputLevel:@30];
                }
            }
        }

        if (fan.controller) {
            if (!min || !max || [max isLessThan:min]) {
                NSManagedObject *controller = fan.controller;
                [fan setController:nil];
                [self.managedObjectContext deleteObject:controller];
            }
            else {
                [fan.controller calculateCurrentLevel];
            }
        }

        return fan;
    }
    else {
        //[self.managedObjectContext deleteObject:fan];
        [fan setService:@0];
    }

    return nil;
}


- (void)insertSmcFansWithConnection:(io_connect_t)connection keys:(NSSet*)keys
{
    HWMSensorsGroup *group = [self getGroupBySelector:kHWMGroupTachometer];

	SMCVal_t info;

	UInt32Char_t fnum = "FNum";
	int totalFans = 15;	// use old value if we can't find number of fans in SMC

	if (kIOReturnSuccess == SMCReadKey(connection, fnum, &info)) {

		totalFans = _strtoul((const char*)info.bytes, info.dataSize, 10);

		NSLog(@"number of fans: %d", totalFans);
	}

	for (int i=0; i<totalFans; i++) {

        NSString *key = [NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i];

		if ([keys containsObject:key]) {

            if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                NSString *type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];
                NSData *value = [NSData dataWithBytes:info.bytes length:info.dataSize];

                if ([type isEqualToString:@TYPE_CH8]) {

                    NSString * caption = [[NSString alloc] initWithData:value encoding: NSUTF8StringEncoding];

                    if ([caption length] == 0)
                        caption = [[NSString alloc] initWithFormat:@"Fan %X", i + 1];

                    if (![caption hasPrefix:@"GPU "]) {

                        key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                        if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                            type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];

                            [self insertSmcFanWithConnection:connection descriptor:caption name:key type:type title:GetLocalizedString(caption) selector:group.selector.unsignedIntegerValue group:group];
                        }
                    }
                }
                else if ([type isEqualToString:@TYPE_FDS]) {

                    FanTypeDescStruct *fds = (FanTypeDescStruct*)[value bytes];

                    if (fds) {

                        NSString *caption = [[NSString stringWithCString:fds->strFunction encoding:NSASCIIStringEncoding] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

                        if ([caption length] == 0)
                            caption = [[NSString alloc] initWithFormat:@"Fan %X", i + 1];

                        switch (fds->type) {
                            case GPU_FAN_RPM:
                            case GPU_FAN_PWM_CYCLE:
                                // Add it later as GPU sensors
                                break;

                            default:
                                key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                                if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                                    type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];

                                    [self insertSmcFanWithConnection:connection descriptor:caption name:key type:type title:GetLocalizedString(caption) selector:group.selector.unsignedIntegerValue group:group];
                                }

                                break;
                        }
                    }
                }
            }
        }
    }

}

- (void)insertSmcGpuFansWithConnection:(io_connect_t)connection keys:(NSSet*)keys
{
    HWMSensorsGroup *group = [self getGroupBySelector:kHWMGroupTachometer];

    // GPU Fans
    for (int i=0; i < 0xf; i++) {
        
        NSString *key = [NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i];

        if ([keys containsObject:key]) {
            SMCVal_t info;

            if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                NSString *type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];
                NSData *value = [NSData dataWithBytes:info.bytes length:info.dataSize];

                if ([type isEqualToString:@TYPE_CH8]) {

                    NSString * caption = [[NSString alloc] initWithData:value encoding: NSUTF8StringEncoding];

                    if ([caption hasPrefix:@"GPU "]) {

                        UInt8 cardIndex = [[caption substringFromIndex:4] intValue] - 1;

                        NSString *title = cardIndex == 0 ? @"GPU Fan" : [NSString stringWithFormat:@"GPU %X Fan", cardIndex + 1];

                        key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                        if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                            type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];

                            [self insertSmcFanWithConnection:connection descriptor:title name:key type:type title:GetLocalizedString(title) selector:group.selector.unsignedIntegerValue group:group];
                        }
                    }
                }
                else if ([type isEqualToString:@TYPE_FDS]) {

                    FanTypeDescStruct *fds = (FanTypeDescStruct*)[value bytes];

                    switch (fds->type) {
                        case GPU_FAN_RPM: {
                            NSString *caption = fds->ui8Zone == 0 ? @"GPU Fan" : [NSString stringWithFormat:@"GPU %X Fan", fds->ui8Zone + 1];

                            key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                            if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                                type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];

                                [self insertSmcFanWithConnection:connection descriptor:caption name:key type:type title:GetLocalizedString(caption) selector:group.selector.unsignedIntegerValue group:group];
                            }

                            break;
                        }

                        case GPU_FAN_PWM_CYCLE: {

                            NSString *caption = fds->ui8Zone == 0 ? @"GPU PWM" : [NSString stringWithFormat:@"GPU %X PWM", fds->ui8Zone + 1];

                            key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                            if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                                type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];

                                [self insertSmcFanWithConnection:connection descriptor:caption name:key type:type title:GetLocalizedString(caption) selector:kHWMGroupPWM group:group];
                            }

                            break;
                        }
                    }
                }
            }
        }
    }
}

#pragma mark
#pragma mark ATA SMART Sensors

-(HWMAtaSmartSensor*)insertAtaSmartSensorFromDictionary:(NSDictionary*)attributes group:(HWMSensorsGroup*)group
{
    NSString *serialNumber = [attributes objectForKey:@"serialNumber"];
    NSString *revision = [attributes objectForKey:@"revision"];

    NSString *name = [NSString stringWithFormat:@"%@%@", serialNumber, revision];

    HWMAtaSmartSensor *sensor = [self getSensorByName:name fromGroup:group];

    if (!sensor) {
        sensor = [NSEntityDescription insertNewObjectForEntityForName:@"AtaSmartSensor" inManagedObjectContext:self.managedObjectContext];

        [sensor setName:name];
        [sensor setGroup:group];
    }

    [sensor setService:[attributes objectForKey:@"service"]];

    [sensor setSelector:group.selector];
    [sensor setBsdName:[attributes objectForKey:@"bsdName"]];
    [sensor setProductName:[attributes objectForKey:@"productName"]];
    [sensor setVolumeNames:[attributes objectForKey:@"volumesNames"]];
    [sensor setRevision:revision];
    [sensor setSerialNumber:serialNumber];
    [sensor setRotational:[attributes objectForKey:@"rotational"]];

    [sensor setLegend:sensor.volumeNames];
    [sensor setIdentifier:@"Drive"];

    [sensor setEngine:self];

    [sensor doUpdateValue];

    if (!sensor.value && !sensor.hidden.boolValue) {
        [sensor setService:@0];
        return nil;
    }

    return sensor;
}

#pragma mark
#pragma mark Battery Sensors

-(HWMBatterySensor*)getBatterySensorByName:(NSString*)name fromGroup:(HWMSensorsGroup*)group
{
    NSArray *sensors = [[group.sensors array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];

    return sensors && sensors.count ? [sensors objectAtIndex:0] : nil;
}

-(HWMBatterySensor*)insertBatterySensorFromDictionary:(NSDictionary*)attributes group:(HWMSensorsGroup*)group
{
    NSString *serialNumber = [attributes objectForKey:@"serialNumber"];
    NSString *productName = [attributes objectForKey:@"productName"];
    NSNumber *deviceType = [attributes objectForKey:@"deviceType"];
    NSNumber *service = [attributes objectForKey:@"service"];

    HWMBatterySensor *sensor = [self getBatterySensorByName:serialNumber fromGroup:group];

    if (!sensor) {
        sensor = [NSEntityDescription insertNewObjectForEntityForName:@"BatterySensor" inManagedObjectContext:self.managedObjectContext];

        [sensor setName:serialNumber];
        [sensor setGroup:group];
    }

    [sensor setService:service];
    [sensor setSelector:group.selector];

    [sensor setDeviceType:deviceType.unsignedIntegerValue];

    [sensor doUpdateValue];

#ifdef kHWMonitorDebugBattery
    [sensor setValue:[NSNumber numberWithInt:arc4random() % 100]];
#endif

    if (sensor.value) {
        [sensor setTitle:GetLocalizedString(productName)];
        [sensor setIdentifier:@"Battery"];

        [sensor setEngine:self];

        NSLog(@"added battery device %@ (%@)", sensor.name, sensor.service);
    }
    else {
        //[self.managedObjectContext deleteObject:sensor];
        [sensor setService:@0];
        return nil;
    }

    return sensor;
}

#pragma mark
#pragma mark Graphs

static NSUInteger gHWMGraphColorIndex = 0;

-(void)insertGraphsFromSensorsArray:(NSArray*)sensors group:(HWMGraphsGroup*)group
{
    for (HWMSensor *sensor in sensors) {
        
        if (!sensor.graph) {
            [sensor setGraph:[NSEntityDescription insertNewObjectForEntityForName:@"Graph" inManagedObjectContext:self.managedObjectContext]];
            
            [sensor.graph setGroup:group];
        }
        
        if (++gHWMGraphColorIndex >= [[HWMGraph graphColors] count]) {
            gHWMGraphColorIndex = 0;
        }
        
        [sensor.graph setColor:[[HWMGraph graphColors] objectAtIndex:gHWMGraphColorIndex]];
        [sensor.graph setIdentifier:@"Item"];
    }
}

-(HWMGraphsGroup*)getGraphsGroupByName:(NSString*)name
{
    NSArray *graphs = [[self.configuration.graphGroups array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];
    
    return graphs && graphs.count ? [graphs objectAtIndex:0] : nil;
}

-(HWMGraphsGroup*)insertGraphsGroupWithSelectors:(NSArray*)selectors name:(NSString*)name icon:(HWMIcon*)icon sensors:(NSArray*)sensors
{
    HWMGraphsGroup *group = [self getGraphsGroupByName:name];
    
    if (!group) {
        group = [NSEntityDescription insertNewObjectForEntityForName:@"GraphsGroup" inManagedObjectContext:self.managedObjectContext];
        
        [group setConfiguration:self.configuration];
    }
    
    [group setName:name];
    [group setTitle:GetLocalizedString(group.name)];
    [group setIcon:icon];
    [group setSelectors:selectors];
    [group setIdentifier:@"Group"];
    
    [group setEngine:self];
    
    gHWMGraphColorIndex = 3;
    
    // insert sensors
    for (NSNumber * sel in selectors) {
        NSArray *arrangedSensors = [sensors filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"selector == %@", sel]];
        
        [self insertGraphsFromSensorsArray:arrangedSensors group:group];
    }
    
    return group;
}

-(void)insertGraphs
{
    NSMutableArray *sensors = [[NSMutableArray alloc] init];
    
    [self.configuration.sensorGroups enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [sensors addObjectsFromArray:[[(HWMSensorsGroup*)obj sensors] array]];
    }];
    
    [self insertGraphsGroupWithSelectors:@[@kHWMGroupTemperature, @kHWMGroupSmartTemperature] name:@"TEMPERATURES" icon:[self getIconByName:kHWMonitorIconTemperatures] sensors:sensors];
    [self insertGraphsGroupWithSelectors:@[@kHWMGroupFrequency] name:@"FREQUENCIES" icon:[self getIconByName:kHWMonitorIconFrequencies] sensors:sensors];
    [self insertGraphsGroupWithSelectors:@[@kHWMGroupTachometer] name:@"FANS & PUMPS" icon:[self getIconByName:kHWMonitorIconTachometers] sensors:sensors];
    [self insertGraphsGroupWithSelectors:@[@kHWMGroupVoltage] name:@"VOLTAGES" icon:[self getIconByName:kHWMonitorIconVoltages] sensors:sensors];
    [self insertGraphsGroupWithSelectors:@[@kHWMGroupCurrent] name:@"CURRENTS" icon:[self getIconByName:kHWMonitorIconVoltages] sensors:sensors];
    [self insertGraphsGroupWithSelectors:@[@kHWMGroupPower] name:@"POWER CONSUMPTION" icon:[self getIconByName:kHWMonitorIconVoltages] sensors:sensors];
}

@end
