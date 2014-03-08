//
//  HWMEngine.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

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

#import "smc.h"
#import "Localizer.h"

#import "HWMonitorDefinitions.h"
#import "FakeSMCDefinitions.h"

#import "SmcHelper.h"

#import "NSImage+HighResolutionLoading.h"

#import <QuartzCore/QuartzCore.h>

NSString * const HWMEngineSensorValuesHasBeenUpdatedNotification = @"HWMEngineSensorsHasBenUpdatedNotification";

static HWMEngine* gDefaultEngine = nil;

@implementation HWMEngine

@synthesize iconsWithSensorsAndGroups = _iconsWithSensorsAndGroups;
@synthesize sensorsAndGroups = _sensorsAndGroups;
@synthesize graphsAndGroups = _graphsAndGroups;
@synthesize favorites = _favorites;

@synthesize isRunningOnMac = _isRunningOnMac;

#pragma mark
#pragma mark Global methods

+(HWMEngine*)defaultEngine
{
    return gDefaultEngine;
}

+(HWMEngine*)engineWithBundle:(NSBundle*)bundle;
{
    return [[HWMEngine alloc] initWithBundle:bundle];
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

-(void)setEngineState:(HWMEngineState)engineState
{
    switch (engineState) {
        case kHWMEngineStateIdle:
            if (_engineState == kHWMEngineStateActive) {
                [self internalStopEngine];
            }
            break;

        case kHWMEngineStateActive:
            if (_engineState == kHWMEngineStateIdle) {
                [self internalStartEngine];
            }
            break;

        default:
            break;
    }

    _engineState = engineState;
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

            for (HWMSensorsGroup *group in _configuration.sensorGroups) {
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

            for (HWMSensorsGroup *group in _configuration.sensorGroups) {
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

            for (HWMGraphsGroup *group in _configuration.graphGroups) {

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

            for (HWMFavorite *favorite in _configuration.favorites) {
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

#pragma mark
#pragma mark Overriden Methods

- (id)init;
{
    self = [super init];

    if (self) {
        _bundle = [NSBundle mainBundle];

        if (!gDefaultEngine) {
            gDefaultEngine = self;
        }
    }

    return self;
}

-(id)initWithBundle:(NSBundle*)bundle
{
    self = [super init];

    if (self) {
        _bundle = bundle;

        if (!gDefaultEngine) {
            gDefaultEngine = self;
        }
    }

    return self;
}

#pragma mark
#pragma mark Private Methods

- (void)initialize
{
    [self assignPlatformProfile];

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

    // Create color themes
    [self insertColorThemes];

    // Load icons
    [self loadIconNamed:kHWMonitorIconHWMonitor asTemplate:NO];
    [self loadIconNamed:kHWMonitorIconThermometer];
    [self loadIconNamed:kHWMonitorIconScale];
    [self loadIconNamed:kHWMonitorIconDevice];
    [self loadIconNamed:kHWMonitorIconTemperatures];
    [self loadIconNamed:kHWMonitorIconHddTemperatures];
    [self loadIconNamed:kHWMonitorIconSsdLife];
    [self loadIconNamed:kHWMonitorIconMultipliers];
    [self loadIconNamed:kHWMonitorIconFrequencies];
    [self loadIconNamed:kHWMonitorIconTachometers];
    [self loadIconNamed:kHWMonitorIconVoltages];
    [self loadIconNamed:kHWMonitorIconBattery];

    // Cleanup icons
    fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Icon"];

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
    [self rebuildSensorsList];

    _engineState = kHWMEngineStateIdle;

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(workspaceDidMountOrUnmount:) name:NSWorkspaceDidMountNotification object:nil];
	[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(workspaceDidMountOrUnmount:) name:NSWorkspaceDidUnmountNotification object:nil];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(workspaceDidMountOrUnmount:) name:NSWorkspaceDidRenameVolumeNotification object:nil];

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(workspaceWillSleep:) name:NSWorkspaceWillSleepNotification object:nil];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(workspaceDidWake:) name:NSWorkspaceDidWakeNotification object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];

    [self addObserver:self forKeyPath:@"configuration.useFahrenheit" options:NSKeyValueObservingOptionNew context:nil];
    [self addObserver:self forKeyPath:@"configuration.smcSensorsUpdateRate" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"configuration.smartSensorsUpdateRate" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"configuration.showVolumeNames" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"configuration.enableFanControl" options:NSKeyValueObservingOptionNew context:nil];
}

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

    if (_platformName) {
        config = [[NSBundle mainBundle] pathForResource:_platformName ofType:@"plist" inDirectory:@"Profiles"];
    }

    if (!config) {
        config = [[NSBundle mainBundle] pathForResource:@"Default" ofType:@"plist" inDirectory:@"Profiles"];
    }

    if (config) {

        NSLog(@"Loading profile from %@", [config lastPathComponent]);

        NSArray *rawProfile = [[NSArray alloc] initWithContentsOfFile:config];

        if (rawProfile) {

            NSMutableArray *adoptedProfile = [[NSMutableArray alloc] init];

            [rawProfile enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                NSArray *entries = [obj componentsSeparatedByString:@"|"];

                [adoptedProfile addObject:[NSArray arrayWithObjects:[entries objectAtIndex:0], [entries objectAtIndex:entries.count - 1], nil]];
            }];

            _platformProfile = [NSArray arrayWithArray:adoptedProfile];
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
        [image setMatchesOnlyOnBestFittingAxis:YES];
        [image setTemplate:template];
    }

    NSImage *alternate = [NSImage loadImageNamed:[name stringByAppendingString:@"-white"] ofType:@"png"];

    if (alternate) {
        [alternate setMatchesOnlyOnBestFittingAxis:YES];
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

- (void)initSmcAndDevicesTimer
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        if (_smcAndDevicesSensorsUpdateLoopTimer && _smcAndDevicesSensorsUpdateLoopTimer.timeInterval == _configuration.smcSensorsUpdateRate.floatValue) {
            return;
        }

        if (_smcAndDevicesSensorsUpdateLoopTimer && _smcAndDevicesSensorsUpdateLoopTimer.isValid) {
            [_smcAndDevicesSensorsUpdateLoopTimer invalidate];
        }

        _smcAndDevicesSensorsUpdateLoopTimer = [NSTimer timerWithTimeInterval:_configuration.smcSensorsUpdateRate.floatValue target:self selector:@selector(updateSmcAndDeviceSensors) userInfo:nil repeats:YES];

        [[NSRunLoop mainRunLoop] addTimer:_smcAndDevicesSensorsUpdateLoopTimer forMode:NSRunLoopCommonModes];
    }];
}

- (void)initAtaSmartTimer
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        if (_ataSmartSensorsUpdateLoopTimer && _ataSmartSensorsUpdateLoopTimer.timeInterval == _configuration.smartSensorsUpdateRate.floatValue * 60.0f) {
            return;
        }

        if (_ataSmartSensorsUpdateLoopTimer && _ataSmartSensorsUpdateLoopTimer.isValid) {
            [_ataSmartSensorsUpdateLoopTimer invalidate];
        }

        _ataSmartSensorsUpdateLoopTimer = [NSTimer timerWithTimeInterval:_configuration.smartSensorsUpdateRate.floatValue * 60.0f target:self selector:@selector(updateAtaSmartSensors) userInfo:nil repeats:YES];

        [[NSRunLoop mainRunLoop] addTimer:_ataSmartSensorsUpdateLoopTimer forMode:NSRunLoopCommonModes];

    }];
}

-(void)internalStartEngine
{
    if (!_smcAndDevicesSensorsUpdateLoopTimer || ![_smcAndDevicesSensorsUpdateLoopTimer isValid]) {
        [self initSmcAndDevicesTimer];
    }

    if (!_ataSmartSensorsUpdateLoopTimer || ![_ataSmartSensorsUpdateLoopTimer isValid]) {
        [self initAtaSmartTimer];
    }
}

-(void)internalStopEngine
{
    if (_smcAndDevicesSensorsUpdateLoopTimer) {
        [_smcAndDevicesSensorsUpdateLoopTimer invalidate];
        _smcAndDevicesSensorsUpdateLoopTimer = 0;
    }

    if (_ataSmartSensorsUpdateLoopTimer) {
        [_ataSmartSensorsUpdateLoopTimer invalidate];
        _ataSmartSensorsUpdateLoopTimer = 0;
    }
}

#pragma mark
#pragma mark Public Methods

-(void)saveContext
{
    NSError *error;

    if (![self.managedObjectContext save:&error])
        NSLog(@"failed to save context %@", error);
}

-(void)updateSmcAndDeviceSensors
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        if (_engineState == kHWMEngineNotInitialized)
            return;

        NSTimeInterval nineTenths = _configuration.smcSensorsUpdateRate.floatValue * 0.9f;

        if (_smcAndDevicesSensorsLastUpdated && fabs(_smcAndDevicesSensorsLastUpdated.timeIntervalSinceNow) < nineTenths) {
            return;
        }
        else {
            _smcAndDevicesSensorsLastUpdated = [NSDate date];
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

        for (HWMSensor *sensor in _smcAndDevicesSensors) {
            BOOL doUpdate = NO;

//            if (sensor.lastUpdated && fabs(sensor.lastUpdated.timeIntervalSinceNow) < nineTenths) {
//                continue;
//            }

            if (_configuration.updateSensorsInBackground.boolValue || sensor.forced.boolValue || sensor.acceptors.count) {
                doUpdate = YES;
            }
            else {
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
            }
        }

        if (self.delegate && [self.delegate respondsToSelector:@selector(engine:shouldCaptureSensorValuesToGaphsHistoryWithLimit:)]) {

            NSUInteger limit;

            if ([self.delegate engine:self shouldCaptureSensorValuesToGaphsHistoryWithLimit:&limit]) {
                for (HWMGraphsGroup *graphsGroup in self.configuration.graphGroups) {
                    [graphsGroup captureSensorValuesToGraphsHistorySetLimit:limit];
                }
            }
        }

        [[NSNotificationCenter defaultCenter] postNotificationName:HWMEngineSensorValuesHasBeenUpdatedNotification object:self];
    }];
}

-(void)updateAtaSmartSensors
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        if (_engineState == kHWMEngineNotInitialized)
            return;

        NSTimeInterval nineTenths = _configuration.smartSensorsUpdateRate.floatValue * 60 * 0.9f;

        if (_ataSmartSensorsLastUpdated && fabs(_ataSmartSensorsLastUpdated.timeIntervalSinceNow) < nineTenths) {
            return;
        }
        else {
            _ataSmartSensorsLastUpdated = [NSDate date];
        }

        if (!_ataSmartSensors) {

            __block NSMutableArray *sensors = [NSMutableArray array];

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
            for (HWMAtaSmartSensor *sensor in _ataSmartSensors) {
                BOOL doUpdate = NO;

//                if (sensor.lastUpdated && fabs(sensor.lastUpdated.timeIntervalSinceNow) < nineTenths) {
//                    continue;
//                }

                if (_configuration.updateSensorsInBackground.boolValue || sensor.forced.boolValue || sensor.acceptors.count) {
                    doUpdate = YES;
                }
                else {
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

                if (doUpdate)
                    [sensor doUpdateValue];
            }

            [HWMSmartPluginInterfaceWrapper destroyAllWrappers];

            if (self.delegate && [self.delegate respondsToSelector:@selector(engine:shouldCaptureSensorValuesToGaphsHistoryWithLimit:)]) {

                NSUInteger limit;

                if ([self.delegate engine:self shouldCaptureSensorValuesToGaphsHistoryWithLimit:&limit]) {
                    for (HWMGraphsGroup *graphsGroup in self.configuration.graphGroups) {
                        [graphsGroup captureSensorValuesToGraphsHistorySetLimit:limit];
                    }
                }
            }

            [[NSNotificationCenter defaultCenter] postNotificationName:HWMEngineSensorValuesHasBeenUpdatedNotification object:self];
        }
    }];
}

-(void)rebuildSensorsList
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{

        if (_engineState == kHWMEngineStateActive) {
            [self internalStopEngine];
        }

        NSError *error;

        // Nulify "service" attribute for all sensors
        NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Sensor"];

        NSArray *objects = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

        for (HWMSmcSensor *sensor in objects) {
            [sensor setService:@0];
        }

        // SMC

        // FakeSMCKeyStore is prioritized key source
        SMCOpen("FakeSMCKeyStore", &_fakeSmcConnection);
        NSArray *fakeSmcKeys = [self getSmcKeysFromConnection:_fakeSmcConnection excludedList:nil];
        if (!fakeSmcKeys || !fakeSmcKeys.count) SMCClose(_fakeSmcConnection);

        SMCOpen("AppleSMC", &_smcConnection);
        NSArray *smcKeys = [self getSmcKeysFromConnection:_smcConnection excludedList:fakeSmcKeys];
        if (!smcKeys || !smcKeys.count) SMCClose(_smcConnection);

        [self insertSmcSensorsWithKeys:fakeSmcKeys connection:_fakeSmcConnection];
        [self insertSmcSensorsWithKeys:smcKeys connection:_smcConnection];

        // SMART
        [HWMAtaSmartSensor startWatchingForBlockStorageDevicesWithEngine:self];

        // FANS
        [self insertSmcFansWithConnection:_fakeSmcConnection keys:fakeSmcKeys];
        [self insertSmcFansWithConnection:_smcConnection keys:smcKeys];

        // Insert additional GPU fans from FakeSMCKeyStore
        [self insertSmcGpuFansWithConnection:_fakeSmcConnection keys:fakeSmcKeys];

        // BATTERIES
        [HWMBatterySensor startWatchingForBatteryDevicesWithEngine:self];

        // Update graphs
        [self insertGraphs];

        // Save context
        if (![self.managedObjectContext save:&error])
            NSLog(@"saving context on rebuildSensorsList error %@", error);

        [self setNeedsUpdateLists];

        if (_engineState == kHWMEngineStateActive) {
            [self internalStartEngine];
        }

    }];
}

-(void)setNeedsUpdateLists
{
    [self setNeedsUpdateSensorLists];
    [self setNeedsUpdateGraphsList];
}

-(void)setNeedsUpdateSensorLists
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self willChangeValueForKey:@"iconsWithSensorsAndGroups"];
        _iconsWithSensorsAndGroups = nil;
        [self didChangeValueForKey:@"iconsWithSensorsAndGroups"];

        [self willChangeValueForKey:@"sensorsAndGroups"];
        _sensorsAndGroups = nil;
        [self didChangeValueForKey:@"sensorsAndGroups"];

        [self willChangeValueForKey:@"favorites"];
        _favorites = nil;
        [self didChangeValueForKey:@"favorites"];

        _smcAndDevicesSensors = nil;
        _ataSmartSensors = nil;
    }];
}

-(void)setNeedsUpdateGraphsList
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self willChangeValueForKey:@"graphsAndGroups"];
        _graphsAndGroups = nil;
        [self didChangeValueForKey:@"graphsAndGroups"];
    }];
}

-(void)setNeedsRecalculateSensorValues
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Sensor"];

        NSError *error;

        NSArray *sensors = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

        if (error) {
            NSLog(@"fetch sensors in setNeedsRecalculateSensorValues error %@", error);
        }

        if (sensors) {
            [sensors enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [obj willChangeValueForKey:@"value"];
                [obj didChangeValueForKey:@"value"];
                [obj willChangeValueForKey:@"formattedValue"];
                [obj didChangeValueForKey:@"formattedValue"];
                [obj willChangeValueForKey:@"strippedValue"];
                [obj didChangeValueForKey:@"strippedValue"];
            }];
        }
    }];
}

-(void)startEngine
{
    [self internalStartEngine];

    self.engineState = kHWMEngineStateActive;
}

-(void)stopEngine
{
    [self internalStopEngine];

    self.engineState = kHWMEngineStateIdle;
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

            [[_configuration mutableOrderedSetValueForKey:@"favorites"] insertObject:favorite atIndex:index];

            [self willChangeValueForKey:@"favorites"];
            _favorites = nil;
            [self didChangeValueForKey:@"favorites"];

    }
}

-(void)moveFavoritesItemAtIndex:(NSUInteger)fromIndex toIndex:(NSUInteger)toIndex
{
    [[_configuration mutableOrderedSetValueForKey:@"favorites"] moveObjectsAtIndexes:[NSIndexSet indexSetWithIndex:fromIndex] toIndex:toIndex > fromIndex ? toIndex - 1 : toIndex < _configuration.favorites.count ? toIndex : _configuration.favorites.count];

    [self willChangeValueForKey:@"favorites"];
    _favorites = nil;
    [self didChangeValueForKey:@"favorites"];
}

-(void)removeItemFromFavoritesAtIndex:(NSUInteger)index
{
        if (!_configuration.favorites.count)
            return;

        HWMFavorite *favorite = [_configuration.favorites objectAtIndex:index];

        [[_configuration mutableOrderedSetValueForKey:@"favorites"] removeObjectAtIndex:index];

        [self.managedObjectContext deleteObject:favorite];

        [self willChangeValueForKey:@"favorites"];
        _favorites = nil;
        [self didChangeValueForKey:@"favorites"];
}

#pragma mark
#pragma mark Events

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"configuration.useFahrenheit"]) {
        [self setNeedsRecalculateSensorValues];
        
    }
    else if ([keyPath isEqual:@"configuration.smcSensorsUpdateRate"]) {
        [self performSelectorInBackground:@selector(initSmcAndDevicesTimer) withObject:nil];

    }
    else if ([keyPath isEqual:@"configuration.smartSensorsUpdateRate"]) {
        [self performSelectorInBackground:@selector(initAtaSmartTimer) withObject:nil];

    }
    else if ([keyPath isEqual:@"configuration.showVolumeNames"]) {
        [self willChangeValueForKey:@"sensorsAndGroups"];
        [self didChangeValueForKey:@"sensorsAndGroups"];

    }
}

-(void)awakeFromNib
{
    [self initialize];
    //[self startEngine];
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
        [_configuration willChangeValueForKey:@"useBsdDriveNames"];
        [_configuration didChangeValueForKey:@"useBsdDriveNames"];
        [_configuration willChangeValueForKey:@"showVolumeNames"];
        [_configuration didChangeValueForKey:@"showVolumeNames"];

        [self setNeedsUpdateLists];
    }
}

-(void)workspaceWillSleep:(id)sender
{
    [self saveContext];

    if (self.engineState == kHWMEngineStateActive) {
        [self internalStopEngine];
    }

    if (_smcConnection) {
        SMCClose(_smcConnection);
        _smcConnection = 0;
    }

    if (_fakeSmcConnection) {
        SMCClose(_fakeSmcConnection);
        _fakeSmcConnection = 0;
    }

    [HWMAtaSmartSensor stopWatchingForBlockStorageDevices];
    [HWMBatterySensor stopWatchingForBatteryDevices];
}

-(void)workspaceDidWake:(id)sender
{
    [self rebuildSensorsList];

    if (self.engineState == kHWMEngineStateActive) {
        [self internalStartEngine];
    }
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

        [HWMSmartPluginInterfaceWrapper destroyAllWrappers];

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
    [self removeObserver:self forKeyPath:@"configuration.useFahrenheit"];
    [self removeObserver:self forKeyPath:@"configuration.smcSensorsUpdateRate"];
    [self removeObserver:self forKeyPath:@"configuration.smartSensorsUpdateRate"];
    [self removeObserver:self forKeyPath:@"configuration.showVolumeNames"];

    [self internalStopEngine];
    [HWMAtaSmartSensor stopWatchingForBlockStorageDevices];
    [HWMBatterySensor stopWatchingForBatteryDevices];

    [self saveContext];

    for (HWMSensor *sensor in _smcAndDevicesSensors) {
        if (sensor.service && sensor.service.unsignedLongLongValue) {
            IOObjectRelease((io_object_t)sensor.service.unsignedLongLongValue);
        }
    }
    if (_smcConnection)
        SMCClose(_smcConnection);

    if (_fakeSmcConnection)
        SMCClose(_fakeSmcConnection);
}

#pragma mark
#pragma mark Color Themes

-(HWMColorTheme*)getColorThemeByName:(NSString*)name
{
    NSArray *themes = [[_configuration.colorThemes array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];

    return themes && themes.count ? [themes objectAtIndex:0] : nil;
}

-(HWMColorTheme*)getColorThemeByIndex:(NSUInteger)index
{
    return [_configuration.colorThemes objectAtIndex:index];
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
        [colorTheme setConfiguration:_configuration];
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
    [colorTheme setUseDarkIcons:[NSNumber numberWithBool:useDarkIcons]];

    return colorTheme;
}

-(void)insertColorThemes
{
    [self insertColorThemeWithName:@"Default"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.85 alpha:0.5]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.95 alpha:0.5]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.2 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.45 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.0 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.95]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.15 alpha:0.35]
                   toolbarEndColor:[NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:0.95]
                toolbarShadowColor:[[NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:0.95] highlightWithLevel:0.63]
                 toolbarStartColor:[[NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:0.95] highlightWithLevel:0.6]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.35]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:NO];

    [self insertColorThemeWithName:@"Gray"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.85 alpha:0.5]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.95 alpha:0.5]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.2 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.45 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.25 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.0 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.95]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.15 alpha:0.35]
                   toolbarEndColor:[NSColor colorWithCalibratedWhite:0.23 alpha:0.95]
                toolbarShadowColor:[[NSColor colorWithCalibratedWhite:0.23 alpha:0.95] highlightWithLevel:0.58]
                 toolbarStartColor:[[NSColor colorWithCalibratedWhite:0.23 alpha:0.95] highlightWithLevel:0.55]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.7]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:NO];

    [self insertColorThemeWithName:@"Dark"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.14 alpha:0.5]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.2 alpha:0.5]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.85 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.65 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.85 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.95 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:0.15 alpha:0.95]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.55]
                   toolbarEndColor:[NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:0.98]
                toolbarShadowColor:[[NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:0.98] highlightWithLevel:0.58]
                 toolbarStartColor:[[NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:0.98] highlightWithLevel:0.55]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.35]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:YES];
}

#pragma mark
#pragma mark Groups

-(HWMSensorsGroup*)getGroupBySelector:(NSUInteger)selector
{
    NSArray *groups = [[_configuration.sensorGroups array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"selector == %d", selector]];
    return groups && groups.count ? [groups objectAtIndex:0] : nil;
}

-(HWMSensorsGroup*)insertGroupWithSelector:(NSUInteger)selector name:(NSString*)name icon:(HWMIcon*)icon
{
    HWMSensorsGroup *group = [self getGroupBySelector:selector];

    if (!group) {
        group = [NSEntityDescription insertNewObjectForEntityForName:@"SensorsGroup" inManagedObjectContext:self.managedObjectContext];

        [group setConfiguration:_configuration];
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

    SMCVal_t val;

    SMCReadKey(connection, "#KEY", &val);

    UInt32 count = [SmcHelper decodeNumericValueFromBuffer:val.bytes length:val.dataSize type:val.dataType].unsignedIntValue;

    NSMutableArray *array = [[NSMutableArray alloc] init];

    for (UInt32 index = 0; index < count; index++) {
        SMCKeyData_t  inputStructure;
        SMCKeyData_t  outputStructure;

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

-(void)insertSmcSensorsWithKeys:(NSArray*)keys connection:(io_connect_t)connection selector:(NSUInteger)selector
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

                for (NSUInteger index = 0; index < count; index++) {

                    NSString *formattedKey = [NSString stringWithFormat:keyFormat, start + index];

                    if ([keys indexOfObject:formattedKey] != NSNotFound /*&& [excludedKeys indexOfObject:formattedKey] == NSNotFound*/) {

                        SMCVal_t info;

                        if (kIOReturnSuccess == SMCReadKey(connection, [formattedKey cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                            [self insertSmcSensorWithConnection:connection name:formattedKey type:[NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding] title:[NSString stringWithFormat:GetLocalizedString(title), shift + index] selector:selector group:group];

                        }
                    }
                }
            }
            else if ([keys indexOfObject:key] != NSNotFound /*&& [excludedKeys indexOfObject:key] == NSNotFound*/) {

                SMCVal_t info;

                if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {
                    [self insertSmcSensorWithConnection:connection name:key type:[NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding] title:GetLocalizedString(title) selector:selector group:group];

                }
            }
        }

    }];
}

-(void)insertSmcSensorsWithKeys:(NSArray*)keys connection:(io_connect_t)connection
{
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupTemperature];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupMultiplier];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupFrequency];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupVoltage];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupCurrent];
    [self insertSmcSensorsWithKeys:keys connection:connection selector:kHWMGroupPower];
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
    HWMSmcFanSensor *fan = [self getSmcFanSensorByDescriptor:descriptor fromGroup:group];

    BOOL newFan = NO;

    if (!fan) {
        fan = [NSEntityDescription insertNewObjectForEntityForName:@"SmcFanSensor" inManagedObjectContext:self.managedObjectContext];

        [fan setName:name];
        [fan setDescriptor:descriptor];
        [fan setGroup:group];

        newFan = YES;
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

        if (newFan || !fan.controller) {
            SMCVal_t info;

            char key[5];

            NSNumber *min, *max;

            // Min
            snprintf(key, 5, KEY_FORMAT_FAN_MIN, index);

            if (kIOReturnSuccess == SMCReadKey(connection, key, &info)) {
                min = [SmcHelper decodeNumericValueFromBuffer:info.bytes length:info.dataSize type:info.dataType];
            }

            // Max
            snprintf(key, 5, KEY_FORMAT_FAN_MAX, index);

            if (kIOReturnSuccess == SMCReadKey(connection, key, &info)) {
                max = [SmcHelper decodeNumericValueFromBuffer:info.bytes length:info.dataSize type:info.dataType];
            }

            if (min && max && [max isGreaterThan:min]) {
                HWMSmcFanController *controller = [NSEntityDescription insertNewObjectForEntityForName:@"SmcFanController" inManagedObjectContext:self.managedObjectContext];

                [controller setMin:min];
                [controller setMax:max];

                [fan setController:controller];

                if (self.isRunningOnMac) {
                    // Target
                    snprintf(key, 5, KEY_FORMAT_FAN_TARGET, index);

                    if (kIOReturnSuccess == SMCReadKey(connection, key, &info)) {
                        [controller addOutputLevel:[SmcHelper decodeNumericValueFromBuffer:info.bytes length:info.dataSize type:info.dataType] forInputLevel:@0];
                    }
                }
                else {
                    [controller addOutputLevel:fan.value forInputLevel:@0];
                }
            }
        }

        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            [fan.controller inputValueChanged];
        }];
    }
    else {
        //[self.managedObjectContext deleteObject:fan];
        [fan setService:@0];
        return nil;
    }

    return fan;
}

- (void)insertSmcFansWithConnection:(io_connect_t)connection keys:(NSArray*)keys
{
    HWMSensorsGroup *group = [self getGroupBySelector:kHWMGroupTachometer];

    for (int i=0; i<0xf; i++) {
        NSString *key = [NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i];

        if ([keys indexOfObject:key] != NSNotFound) {

            SMCVal_t info;

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

- (void)insertSmcGpuFansWithConnection:(io_connect_t)connection keys:(NSArray*)keys
{
    HWMSensorsGroup *group = [self getGroupBySelector:kHWMGroupTachometer];

    // GPU Fans
    for (int i=0; i < 0xf; i++) {
        NSString *key = [NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i];

        if ([keys indexOfObject:key] != NSNotFound) {
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

    [sensor setTitle:_configuration.useBsdDriveNames.boolValue ? sensor.bsdName : [sensor.productName stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]]];
    [sensor setLegend:_configuration.showVolumeNames.boolValue ? sensor.volumeNames : nil];
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
    [sensor setValue:@10];
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
    NSArray *graphs = [[_configuration.graphGroups array] filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];
    
    return graphs && graphs.count ? [graphs objectAtIndex:0] : nil;
}

-(HWMGraphsGroup*)insertGraphsGroupWithSelectors:(NSArray*)selectors name:(NSString*)name icon:(HWMIcon*)icon sensors:(NSArray*)sensors
{
    HWMGraphsGroup *group = [self getGraphsGroupByName:name];
    
    if (!group) {
        group = [NSEntityDescription insertNewObjectForEntityForName:@"GraphsGroup" inManagedObjectContext:self.managedObjectContext];
        
        [group setConfiguration:_configuration];
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
    
    [_configuration.sensorGroups enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
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
