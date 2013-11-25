//
//  HWMEngine.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMEngine.h"
#import "HWMIcon.h"
#import "HWMGroup.h"
#import "HWMSmcSensor.h"
#import "HWMSmcFanSensor.h"
#import "HWMAtaSmartSensor.h"
#import "HWMBatterySensor.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "HWMFavorite.h"

#import "smc.h"
#import "Localizer.h"

#import "HWMonitorDefinitions.h"
#import "FakeSMCDefinitions.h"

NSString * const HWMEngineSensorsHasBenUpdatedNotification = @"HWMEngineSensorsHasBenUpdatedNotification";

@implementation HWMEngine

#pragma mark
#pragma mark Global methods

+(HWMEngine*)engineWithBundle:(NSBundle*)bundle;
{
    HWMEngine *me = [[HWMEngine alloc] init];

    if (me) {
        me.bundle = bundle;
    }

    return me;
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

        NSString *path = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject];
        NSURL *url= [NSURL fileURLWithPath:[path stringByAppendingPathComponent: @"HWMonitor/Configuration.xml"]];

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

-(NSArray *)availableItems
{
    if (!_availableItems) {

        @synchronized (self) {

            NSMutableArray *items = [[NSMutableArray alloc] init];

            // Icons
            NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Icon"];

            NSError *error;

            NSArray *icons = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

            if (error) {
                NSLog(@"fetch icons in availableItems error %@", error);
            }
            else {
                NSSortDescriptor *titleDescriptor = [[NSSortDescriptor alloc] initWithKey:@"title" ascending:YES];
                [items addObjectsFromArray:[icons sortedArrayUsingDescriptors:@[titleDescriptor]]];
            }

            // Sensors and groups
            fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Group"];

            NSArray *groups = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

            if (error) {
                NSLog(@"fetch groups in availableItems error %@", error);
            }
            else {
                NSSortDescriptor *orderDescriptor = [[NSSortDescriptor alloc] initWithKey:@"order" ascending:YES];



                groups = [groups sortedArrayUsingDescriptors:@[orderDescriptor]];

                for (HWMGroup *group in groups) {

                    fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Sensor"];

                    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"group == %@", group]];

                    NSArray *sensors = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error] ;

                    if (error) {
                        NSLog(@"fetch sensors in availableItems error %@", error);
                    }

                    __block NSMutableArray *arrangedSensors = [NSMutableArray array];

                    [[sensors sortedArrayUsingDescriptors:@[orderDescriptor]] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                        if ([obj isActive]) {
                            [arrangedSensors addObject:obj];
                        }
                    }];

                    if (arrangedSensors.count) {
                        [items addObject:group];
                        [items addObjectsFromArray:arrangedSensors];
                    }
                }


            }

            [self willChangeValueForKey:@"availableItems"];
            _availableItems = [items copy];
            [self didChangeValueForKey:@"availableItems"];
        }
    }

    return _availableItems;
}

-(NSArray *)favoriteItems
{

    if (!_favoriteItems) {
        @synchronized (self) {

            NSMutableArray *items = [[NSMutableArray alloc] init];

            NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Favorite"];

            NSError *error;

            NSArray *favorites = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

            if (error) {
                NSLog(@"fetch favorite items in favoriteItems error %@", error);
            }
            else {
                NSSortDescriptor *orderDescriptor = [[NSSortDescriptor alloc] initWithKey:@"order" ascending:YES];

                [[favorites sortedArrayUsingDescriptors:@[orderDescriptor]] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {

                    HWMFavorite *favorite = (HWMFavorite *)obj;

                    if ([favorite.item isKindOfClass:[HWMSensor class]]) {
                        if ([(HWMSensor*)favorite.item isActive]) {
                            [items addObject:favorite.item];
                        }
                    }
                    else {
                        [items addObject:favorite.item];
                    }
                }];

                [self willChangeValueForKey:@"favoriteItems"];
                _favoriteItems = [items mutableCopy];
                [self didChangeValueForKey:@"favoriteItems"];
            }
        }
    }

    return _favoriteItems;
}

-(NSArray *)arrangedItems
{
    if (!_arrangedItems) {
        @synchronized (self) {

            // Sensors and groups
            NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Group"];

            NSError *error;

            NSArray *groups = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

            if (error) {
                NSLog(@"fetch groups in arrangedItems error %@", error);
            }
            else {
                NSSortDescriptor *orderDescriptor = [[NSSortDescriptor alloc] initWithKey:@"order" ascending:YES];

                NSMutableArray *items = [NSMutableArray array];

                groups = [groups sortedArrayUsingDescriptors:@[orderDescriptor]];

                for (HWMGroup *group in groups) {

                    fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Sensor"];

                    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"group == %@ AND hidden == 0", group]];

                    NSArray *sensors = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error] ;

                    if (error) {
                        NSLog(@"fetch sensors in arrangedItems error %@", error);
                    }

                    __block NSMutableArray *arrangedSensors = [NSMutableArray array];

                    [[sensors sortedArrayUsingDescriptors:@[orderDescriptor]] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                        if ([obj isActive]) {
                            [arrangedSensors addObject:obj];
                        }
                    }];

                    if (arrangedSensors.count) {
                        [items addObject:group];
                        [items addObjectsFromArray:arrangedSensors];
                    }
                }

                [self willChangeValueForKey:@"arrangedItems"];
                _arrangedItems = [items copy];
                [self didChangeValueForKey:@"arrangedItems"];
            }
        }
    }

    return _arrangedItems;
}

-(NSArray *)graphs
{
    if (!_graphs) {
        @synchronized (self) {
            //

            [self willChangeValueForKey:@"graphs"];
            //_graphs = [items copy];
            [self didChangeValueForKey:@"graphs"];
        }
    }

    return _graphs;
}

#pragma mark
#pragma mark Overriden Methods

- (id)init;
{
    self = [super init];

    if (self) {
        _bundle = [NSBundle mainBundle];
        _syncLock = [[NSRecursiveLock alloc] init];
    }

    return self;
}

#pragma mark
#pragma mark Private Methods

- (void)initialize
{
    // Create or load color themes
    [self insertColorThemes];

    // Create or load configuration entity
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Configuration"];

    NSError *error;

    _configuration = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"failed to retrieve configuration %@", error);
    }

    if (!_configuration) {
        _configuration = [NSEntityDescription insertNewObjectForEntityForName:@"Configuration" inManagedObjectContext:self.managedObjectContext];

        [_configuration setColorTheme:[self getColorThemeByName:@"Default"]];
    }

    [self assignPlatformProfile];

    [self loadIconNamed:kHWMonitorIconDefault];
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

    NSUInteger nextGroupOrder = 0;

    [self insertGroupWithSelector:kHWMGroupTemperature name:@"TEMPERATURES" icon:[self getIconByName:kHWMonitorIconTemperatures] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupSmartTemperature name:@"DRIVE TEMPERATURES" icon:[self getIconByName:kHWMonitorIconTemperatures] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupSmartRemainingLife name:@"SSD REMAINING LIFE" icon:[self getIconByName:kHWMonitorIconSsdLife] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupSmartRemainingBlocks name:@"SSD REMAINING BLOCKS" icon:[self getIconByName:kHWMonitorIconSsdLife] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupFrequency name:@"FREQUENCIES" icon:[self getIconByName:kHWMonitorIconFrequencies] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupTachometer name:@"FANS & PUMPS" icon:[self getIconByName:kHWMonitorIconTachometers] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupVoltage name:@"VOLTAGES" icon:[self getIconByName:kHWMonitorIconVoltages] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupCurrent name:@"CURRENTS" icon:[self getIconByName:kHWMonitorIconVoltages] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupPower name:@"POWER CONSUMPTION" icon:[self getIconByName:kHWMonitorIconVoltages] nextOrder:&nextGroupOrder];
    [self insertGroupWithSelector:kHWMGroupBattery name:@"BATTERIES" icon:[self getIconByName:kHWMonitorIconBattery] nextOrder:&nextGroupOrder];

    [self rebuildSensorsList];

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(workspaceDidMountOrUnmount:) name:NSWorkspaceDidMountNotification object:nil];
	[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self selector: @selector(workspaceDidMountOrUnmount:) name:NSWorkspaceDidUnmountNotification object:nil];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(workspaceWillSleep:) name:NSWorkspaceWillSleepNotification object:nil];
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self selector:@selector(workspaceDidWake:) name:NSWorkspaceDidWakeNotification object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(willTerminate:) name:NSApplicationWillTerminateNotification object:nil];

    [self addObserver:self forKeyPath:@"configuration.smcSensorsUpdateRate" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"configuration.smartSensorsUpdateRate" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"configuration.showVolumeNames" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"configuration.useBsdDriveNames" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"configuration.colorTheme" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:nil];
}

- (void)assignPlatformProfile
{
    NSString *model = nil;

    CFDictionaryRef matching = MACH_PORT_NULL;

    if (MACH_PORT_NULL != (matching = IOServiceMatching("FakeSMC"))) {
        io_iterator_t iterator = IO_OBJECT_NULL;

        if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
            if (IO_OBJECT_NULL != iterator) {

                io_service_t service = MACH_PORT_NULL;

                if (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                    model = @"Hackintosh";

                    IOObjectRelease(service);
                }

                IOObjectRelease(iterator);
            }
        }
    }

    if (!model) {
        if (MACH_PORT_NULL != (matching = IOServiceMatching("IOPlatformExpertDevice"))) {
            io_iterator_t iterator = IO_OBJECT_NULL;

            if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
                if (IO_OBJECT_NULL != iterator) {

                    io_service_t service = MACH_PORT_NULL;

                    if (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                        model = [[NSString alloc] initWithData:(__bridge_transfer NSData *)IORegistryEntryCreateCFProperty(service, CFSTR("model"), kCFAllocatorDefault, 0) encoding:NSASCIIStringEncoding];

                        IOObjectRelease(service);
                    }

                    IOObjectRelease(iterator);
                }
            }
        }
    }

    NSString *config;

    if (model) {
        config = [[NSBundle mainBundle] pathForResource:model ofType:@"plist" inDirectory:@"Profiles"];
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
    HWMIcon *icon = [self getIconByName:name];

    if (!icon) {
        icon = [NSEntityDescription insertNewObjectForEntityForName:@"Icon" inManagedObjectContext:self.managedObjectContext];
    }

    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[self.bundle pathForResource:name ofType:@"png"]];
    NSImage *alternate = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[name stringByAppendingString:@"_template"] ofType:@"png"]];

    if (image)
        [image setTemplate:YES];

    if (alternate)
        [alternate setTemplate:YES];

    [icon setName:name];
    [icon setRegular:image];
    [icon setAlternate:alternate];
    [icon setTitle:GetLocalizedString(name)];
    [icon setIdentifier:@"Icon"];

    return icon;
}

-(NSUInteger)getNextOrderValueForSensorsInGroup:(HWMGroup*)group
{
    __block NSUInteger nextOrderValue = 0;

    [group.sensors enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        if ([[obj order] unsignedIntegerValue] > nextOrderValue) {
            nextOrderValue = [[obj order] unsignedIntegerValue];
        }
    }];

    return nextOrderValue + 1;
}

- (void)initSmcAndDevicesTimer
{
    if (_smcAndDevicesSensorsUpdateLoopTimer && _smcAndDevicesSensorsUpdateLoopTimer.timeInterval == _configuration.smcSensorsUpdateRate.floatValue) {
        return;
    }

    if (_smcAndDevicesSensorsUpdateLoopTimer && _smcAndDevicesSensorsUpdateLoopTimer.isValid) {
        [_smcAndDevicesSensorsUpdateLoopTimer invalidate];
    }

    _smcAndDevicesSensorsUpdateLoopTimer = [NSTimer timerWithTimeInterval:self.configuration.smcSensorsUpdateRate.floatValue target:self selector:@selector(updateSmcAndDevicesSensors) userInfo:nil repeats:YES];

    [[NSRunLoop mainRunLoop] addTimer:_smcAndDevicesSensorsUpdateLoopTimer forMode:NSRunLoopCommonModes];
}

- (void)initAtaSmartTimer
{
    if (_ataSmartSensorsUpdateLoopTimer && _ataSmartSensorsUpdateLoopTimer.timeInterval == _configuration.smartSensorsUpdateRate.floatValue * 60.0f) {
        return;
    }

    if (_ataSmartSensorsUpdateLoopTimer && _ataSmartSensorsUpdateLoopTimer.isValid) {
        [_ataSmartSensorsUpdateLoopTimer invalidate];
    }

    _ataSmartSensorsUpdateLoopTimer = [NSTimer timerWithTimeInterval:self.configuration.smartSensorsUpdateRate.floatValue * 60.0f target:self selector:@selector(updateAtaSmartSensors) userInfo:nil repeats:YES];

    [[NSRunLoop mainRunLoop] addTimer:_ataSmartSensorsUpdateLoopTimer forMode:NSRunLoopCommonModes];
}

-(void)internalStartEngine
{
    if (_engineState != kHWMEngineStateActive) {
        if (!_smcAndDevicesSensorsUpdateLoopTimer || ![_smcAndDevicesSensorsUpdateLoopTimer isValid]) {
            [self initSmcAndDevicesTimer];
        }

        if (!_ataSmartSensorsUpdateLoopTimer || ![_ataSmartSensorsUpdateLoopTimer isValid]) {
            [self initAtaSmartTimer];
        }
    }
}

-(void)internalStopEngine
{
    if (_engineState != kHWMEngineStateIdle) {
        if (_smcAndDevicesSensorsUpdateLoopTimer) {
            [_smcAndDevicesSensorsUpdateLoopTimer invalidate];
            _smcAndDevicesSensorsUpdateLoopTimer = 0;
        }

        if (_ataSmartSensorsUpdateLoopTimer) {
            [_ataSmartSensorsUpdateLoopTimer invalidate];
            _ataSmartSensorsUpdateLoopTimer = 0;
        }
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

-(void)updateSmcAndDevicesSensors
{
    @synchronized (self) {
        if (!_smcAndDevicesSensors) {

            __block NSMutableArray *sensors = [NSMutableArray array];\

            [self.availableItems enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                if ([obj isKindOfClass:[HWMSensor class]] && ![obj isKindOfClass:[HWMAtaSmartSensor class]]) {
                    [sensors addObject:obj];
                }
            }];

            _smcAndDevicesSensors = [sensors copy];
        }

        for (HWMSensor *sensor in _smcAndDevicesSensors) {
            BOOL doUpdate = FALSE;

            switch (_updateLoopStrategy) {
                case kHWMSensorsUpdateLoopForced:
                    doUpdate = YES;
                    break;

                case kHWMSensorsUpdateLoopOnlyFavorites:
                    doUpdate = [sensor favorite] != nil;
                    break;

                case kHWMSensorsUpdateLoopRegular:
                default:
                    doUpdate = !sensor.hidden.boolValue || sensor.favorite;
                    break;
            }

            if (doUpdate) {
                [sensor doUpdateValue];
            }
        }

        [[NSNotificationCenter defaultCenter] postNotificationName:HWMEngineSensorsHasBenUpdatedNotification object:self];
    }
}

-(void)updateAtaSmartSensors
{
    @synchronized (self) {
        if (!_ataSmartSensors) {
            __block NSMutableArray *sensors = [NSMutableArray array];\

            [self.availableItems enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                if ([obj isKindOfClass:[HWMAtaSmartSensor class]]) {
                    [sensors addObject:obj];
                }
            }];

            _ataSmartSensors = [sensors copy];
        }

        for (HWMAtaSmartSensor *sensor in _ataSmartSensors) {

            BOOL doUpdate = FALSE;

            switch (_updateLoopStrategy) {
                case kHWMSensorsUpdateLoopForced:
                    doUpdate = YES;
                    break;

                case kHWMSensorsUpdateLoopOnlyFavorites:
                    doUpdate = [sensor favorite] != nil;
                    break;

                case kHWMSensorsUpdateLoopRegular:
                default:
                    doUpdate = !sensor.hidden.boolValue || sensor.favorite;
                    break;
            }

            if (doUpdate) {
                [sensor doUpdateValue];
            }

        }

        [[NSNotificationCenter defaultCenter] postNotificationName:HWMEngineSensorsHasBenUpdatedNotification object:self];
    }
}

-(void)rebuildSensorsList
{
    @synchronized (self) {
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
        SMCOpen(&_fakeSmcConnection, "FakeSMCKeyStore");
        NSArray *fakeSmcKeys = [self getSmcKeysFromConnection:_fakeSmcConnection excludedList:nil];
        if (!fakeSmcKeys || !fakeSmcKeys.count) SMCClose(_fakeSmcConnection);

        SMCOpen(&_smcConnection, "AppleSMC");
        NSArray *smcKeys = [self getSmcKeysFromConnection:_smcConnection excludedList:fakeSmcKeys];
        if (!smcKeys || !smcKeys.count) SMCClose(_smcConnection);

        [self insertSmcSensorsWithKeys:fakeSmcKeys connection:_fakeSmcConnection];
        [self insertSmcSensorsWithKeys:smcKeys connection:_smcConnection];

        // SMART

        _ataSmartSensors = nil;

        [self insertAtaSmartSensors];

        // FANS
        [self insertSmcFansWithConnection:_fakeSmcConnection keys:fakeSmcKeys];
        [self insertSmcFansWithConnection:_smcConnection keys:smcKeys];

        // Insert additional GPU fans from FakeSMCKeyStore
        [self insertSmcGpuFansWithConnection:_fakeSmcConnection keys:fakeSmcKeys];

        // BATTERIES
        [self insertBatterySensors];

        // Normalize groups and sensors orders
        fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Group"];

        __block NSSortDescriptor *orderDescriptor = [[NSSortDescriptor alloc] initWithKey:@"order" ascending:YES];

        [fetchRequest setSortDescriptors:@[orderDescriptor]];

        NSArray *groups = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

        if (error) {
            NSLog(@"fetch groups in rebuildSensorsList error %@", error);
        }

        if (groups) {
            [groups enumerateObjectsUsingBlock:^(id group, NSUInteger group_idx, BOOL *stop) {

                [group setOrder:[NSNumber numberWithUnsignedInteger:group_idx]];

                NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Sensor"];

                [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"group == %@", group]];
                [fetchRequest setSortDescriptors:@[orderDescriptor]];

                NSError *error;

                NSArray *sensors = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

                if (error) {
                    NSLog(@"fetch sensors in rebuildSensorsList error %@", error);
                }

                if (sensors) {
                    [sensors enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                        [obj setOrder:[NSNumber numberWithUnsignedInteger:idx]];
                    }];
                }
            }];
        }

        // Update graphs
        [self generateGraphs];

        if (![self.managedObjectContext save:&error])
            NSLog(@"saving context on rebuildSensorsList error %@", error);

        [self setNeedsUpdateLists];

        if (_engineState == kHWMEngineStateActive) {
            [self internalStartEngine];
        }

    }
}

-(void)setNeedsUpdateLists
{
    @synchronized (self) {
        [self willChangeValueForKey:@"availableItems"];
        _availableItems = nil;
        [self didChangeValueForKey:@"availableItems"];

        [self willChangeValueForKey:@"favoriteItems"];
        _favoriteItems = nil;
        [self willChangeValueForKey:@"favoriteItems"];

        [self willChangeValueForKey:@"arrangedItems"];
        _arrangedItems = nil;
        [self didChangeValueForKey:@"arrangedItems"];
    }
}

-(void)setNeedsRecalculateSensorValues
{
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
        }];
    }
}

-(void)startEngine
{
    [self internalStartEngine];

    _engineState = kHWMEngineStateActive;
}

-(void)stopEngine
{
    [self internalStopEngine];

    _engineState = kHWMEngineStateIdle;
}

-(void)insertItemToFavorites:(HWMItem*)item atIndex:(NSUInteger)index
{
    if (item && item.managedObjectContext == _managedObjectContext && ([item isKindOfClass:[HWMSensor class]] ? ![self.favoriteItems containsObject:item] : YES)) {

        @synchronized (self) {

            [self willChangeValueForKey:@"favoriteItems"];
            [_favoriteItems insertObject:item atIndex:index];
            [self didChangeValueForKey:@"favoriteItems"];

            HWMFavorite *favorite;

            if ([item isKindOfClass:[HWMSensor class]]) {
                favorite = [self getFavoriteByItem:item];
            }

            if (!favorite) {
                favorite = [NSEntityDescription insertNewObjectForEntityForName:@"Favorite" inManagedObjectContext:self.managedObjectContext];

                [favorite setOrder:[NSNumber numberWithInteger:index]];
                [favorite setItem:item];
            }

            [self updateFavoriteItemOrders];
            [self saveContext];
        }
    }
}

-(void)removeItemFromFavoritesAtIndex:(NSUInteger)index
{
    HWMItem *item = [_favoriteItems objectAtIndex:index];

    if (item) {
        @synchronized (self) {
            [self willChangeValueForKey:@"favoriteItems"];
            [_favoriteItems removeObjectAtIndex:index];
            [self didChangeValueForKey:@"favoriteItems"];

            HWMFavorite *favorite = [self getFavoriteByItem:item withOrder:index];

            if (favorite) {
                [self.managedObjectContext deleteObject:favorite];
            }

            [self updateFavoriteItemOrders];
            [self saveContext];
        }
    }
}

#pragma mark
#pragma mark Events

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    @synchronized (self) {
        if (![[change objectForKey:NSKeyValueChangeNewKey] isEqualTo:[change objectForKey:NSKeyValueChangeOldKey]]) {
            if ([keyPath isEqual:@"configuration.smcSensorsUpdateRate"]) {

                [self initSmcAndDevicesTimer];

            }
            else if ([keyPath isEqual:@"configuration.smartSensorsUpdateRate"]) {

                [self initAtaSmartTimer];

            }
            else if ([keyPath isEqual:@"configuration.showVolumeNames"]) {

                NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"AtaSmartSensor"];

                NSError *error;

                NSArray *sensors = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

                if (error) {
                    NSLog(@"fetch AtaSmartSensors in observeValueForKeyPath error %@", error);
                }

                if (sensors) {
                    for (HWMAtaSmartSensor *sensor in sensors) {
                        [sensor setLegend:_configuration.showVolumeNames.boolValue ? sensor.volumeNames : nil];
                    }
                }

                [self willChangeValueForKey:@"arrangedItems"];
                [self didChangeValueForKey:@"arrangedItems"];

            }
            else if ([keyPath isEqual:@"configuration.useBsdDriveNames"]) {

                NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"AtaSmartSensor"];

                NSError *error;

                NSArray *sensors = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

                if (error) {
                    NSLog(@"fetch AtaSmartSensors in observeValueForKeyPath error %@", error);
                }

                if (sensors) {
                    for (HWMAtaSmartSensor *sensor in sensors) {
                        [sensor setTitle:_configuration.useBsdDriveNames.boolValue ? sensor.bsdName : sensor.productName];
                    }
                }

                [self willChangeValueForKey:@"arrangedItems"];
                [self didChangeValueForKey:@"arrangedItems"];
                
            }
            else if ([keyPath isEqual:@"configuration.colorTheme"]) {

                NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Icon"];

                NSError *error;

                NSArray *icons = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

                if (error) {
                    NSLog(@"fetch Icon in observeValueForKeyPath error %@", error);
                }

                if (icons) {
                    for (HWMIcon *icon in icons) {
                        [icon setImage:_configuration.colorTheme.useDarkIcons.boolValue ? icon.alternate : icon.regular];
                    }
                }

                [self willChangeValueForKey:@"arrangedItems"];
                [self didChangeValueForKey:@"arrangedItems"];

            }
        }
    }
}

-(void)awakeFromNib
{
    [self initialize];
    //[self startEngine];
}

-(void)workspaceDidMountOrUnmount:(id)sender
{
    [self insertAtaSmartSensors];

    // Update graphs
    [self generateGraphs];

    _ataSmartSensors = nil;

    [self willChangeValueForKey:@"availableItems"];
    _availableItems = nil;
    [self didChangeValueForKey:@"availableItems"];

    [self willChangeValueForKey:@"favoriteItems"];
    _favoriteItems = nil;
    [self didChangeValueForKey:@"favoriteItems"];
}

-(void)workspaceWillSleep:(id)sender
{
    if (_engineState == kHWMEngineStateActive) {
        [self internalStopEngine];
    }
}

-(void)workspaceDidWake:(id)sender
{
    if (_engineState == kHWMEngineStateActive) {
        [self internalStartEngine];
    }
}

- (void)willTerminate:(id)sender
{
    [self removeObserver:self forKeyPath:@"configuration.smcSensorsUpdateRate"];
    [self removeObserver:self forKeyPath:@"configuration.smartSensorsUpdateRate"];
    [self removeObserver:self forKeyPath:@"configuration.showVolumeNames"];
    [self removeObserver:self forKeyPath:@"configuration.useBsdDriveNames"];
    [self removeObserver:self forKeyPath:@"configuration.colorTheme"];

    [self internalStopEngine];

    [self saveContext];

    if (_smcConnection)
        SMCClose(_smcConnection);

    if (_fakeSmcConnection)
        SMCClose(_fakeSmcConnection);
}

#pragma mark
#pragma mark Favorites

-(HWMFavorite*)getFavoriteByItem:(HWMItem*)item
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Favorite"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"item == %@", item]];

    NSError *error;

    HWMFavorite *favorite = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"fetch Favorite in getFavoriteByItem: error %@", error);
    }

    return favorite;
}

-(HWMFavorite*)getFavoriteByItem:(HWMItem*)item withOrder:(NSUInteger)order
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Favorite"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"item == %@ AND order == %d", item, order]];

    NSError *error;

    HWMFavorite *favorite = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"fetch Favorite in getFavoriteByItem:AndIndex: error %@", error);
    }

    return favorite;
}

-(void)updateFavoriteItemOrders
{

        if (_favoriteItems) {
            [_favoriteItems enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [[obj favorite] setOrder:[NSNumber numberWithUnsignedInteger:idx]];
            }];
        }

            //            [(HWMFavorite*)obj setOrder:[NSNumber numberWithUnsignedInteger:idx]];
            //        }];
//    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Favorite"];
//
//    NSError *error;
//
//    NSArray *favorites = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];
//
//    if (error) {
//        NSLog(@"fetch favorites in updateFavoriteOrders error %@", error);
//    }
//
//    if (favorites) {
//        [[favorites sortedArrayUsingDescriptors:@[[[NSSortDescriptor alloc] initWithKey:@"order" ascending:YES]]] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
//            [(HWMFavorite*)obj setOrder:[NSNumber numberWithUnsignedInteger:idx]];
//        }];
//    }
}

#pragma mark
#pragma mark Color Themes

-(HWMColorTheme*)getColorThemeByName:(NSString*)name
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"ColorTheme"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];

    NSError *error;

    HWMColorTheme *colorTheme = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"getColorThemeByName error %@", error);
    }

    return colorTheme;
}

-(HWMColorTheme*)getColorThemeByIndex:(NSUInteger)index
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"ColorTheme"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"order == %d", index]];

    NSError *error;

    HWMColorTheme *colorTheme = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"getColorThemeByIndex error %@", error);
    }

    return colorTheme;
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
                                nextOrder:(NSUInteger*)nextOrder
{
    HWMColorTheme *colorTheme = [self getColorThemeByName:name];

    if (!colorTheme) {
        colorTheme = [NSEntityDescription insertNewObjectForEntityForName:@"ColorTheme" inManagedObjectContext:self.managedObjectContext];

        [colorTheme setName:name];
        [colorTheme setOrder:[NSNumber numberWithInteger:*nextOrder]];

        *nextOrder += 1;
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
    //    theme.name = @"Default";
    //    theme.toolbarEndColor = [NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:0.95];
    //    theme.toolbarStartColor = [theme.toolbarEndColor highlightWithLevel:0.6];
    //    theme.toolbarTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    //    theme.toolbarShadowColor = [NSColor colorWithCalibratedWhite:1.0 alpha:0.3];
    //    theme.toolbarStrokeColor = [NSColor colorWithCalibratedWhite:0.0 alpha:0.35];
    //    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:0.95];
    //    theme.listStrokeColor = [NSColor colorWithCalibratedWhite:0.15 alpha:0.35];
    //    theme.groupStartColor = [NSColor colorWithCalibratedWhite:0.95 alpha:0.5];
    //    theme.groupEndColor = [NSColor colorWithCalibratedWhite:0.85 alpha:0.5];
    //    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    //    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    //    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.45 alpha:1.0];
    //    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    //    theme.useDarkIcons = YES;

    NSUInteger order = 0;

    [self insertColorThemeWithName:@"Default"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.85 alpha:0.5]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.95 alpha:0.5]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.6 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.45 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.15 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.0 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.95]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.15 alpha:0.35]
                   toolbarEndColor:[NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:0.95]
                toolbarShadowColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.3]
                 toolbarStartColor:[[NSColor colorWithCalibratedRed:0.05 green:0.25 blue:0.85 alpha:0.95] highlightWithLevel:0.6]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.35]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:NO
                         nextOrder:&order];

    //    theme.name = @"Gray";
    //    theme.toolbarEndColor = [NSColor colorWithCalibratedWhite:0.23 alpha:0.95];
    //    theme.toolbarStartColor = [theme.toolbarEndColor highlightWithLevel:0.55];
    //    theme.toolbarTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    //    theme.toolbarShadowColor = [NSColor colorWithCalibratedWhite:0.7 alpha:0.3];
    //    theme.toolbarStrokeColor = [NSColor colorWithCalibratedWhite:0.0 alpha:0.7];
    //    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:0.95];
    //    theme.listStrokeColor = [NSColor colorWithCalibratedWhite:0.15 alpha:0.35];
    //    theme.groupStartColor = [NSColor colorWithCalibratedWhite:0.95 alpha:0.5];
    //    theme.groupEndColor = [NSColor colorWithCalibratedWhite:0.85 alpha:0.5];
    //    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    //    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    //    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.45 alpha:1.0];
    //    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    //    theme.useDarkIcons = YES;

    [self insertColorThemeWithName:@"Gray"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.85 alpha:0.5]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.95 alpha:0.5]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.6 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.45 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.15 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.0 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.95]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.15 alpha:0.35]
                   toolbarEndColor:[NSColor colorWithCalibratedWhite:0.23 alpha:0.95]
                toolbarShadowColor:[NSColor colorWithCalibratedWhite:0.7 alpha:0.3]
                 toolbarStartColor:[[NSColor colorWithCalibratedWhite:0.23 alpha:0.95] highlightWithLevel:0.55]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.7]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:NO
                         nextOrder:&order];

    //    theme.name = @"Dark";
    //    theme.toolbarEndColor = [NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:0.98];
    //    theme.toolbarStartColor = [theme.toolbarEndColor highlightWithLevel:0.55];
    //    theme.toolbarTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    //    theme.toolbarShadowColor = [NSColor colorWithCalibratedWhite:1.0 alpha:0.3];
    //    theme.toolbarStrokeColor = [NSColor colorWithCalibratedWhite:0.0 alpha:0.35];
    //    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:0.15 alpha:0.95];
    //    theme.listStrokeColor = [NSColor colorWithCalibratedWhite:0.0 alpha:0.55];
    //    theme.groupStartColor = [NSColor colorWithCalibratedWhite:0.2 alpha:0.5];
    //    theme.groupEndColor = [NSColor colorWithCalibratedWhite:0.14 alpha:0.5];
    //    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.45 alpha:1.0];
    //    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.85 alpha:1.0];
    //    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.65 alpha:1.0];
    //    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.95 alpha:1.0];
    //    theme.useDarkIcons = NO;

    [self insertColorThemeWithName:@"Dark"
                     groupEndColor:[NSColor colorWithCalibratedWhite:0.14 alpha:0.5]
                   groupStartColor:[NSColor colorWithCalibratedWhite:0.2 alpha:0.5]
                   groupTitleColor:[NSColor colorWithCalibratedWhite:0.45 alpha:1.0]
                 itemSubTitleColor:[NSColor colorWithCalibratedWhite:0.65 alpha:1.0]
                    itemTitleColor:[NSColor colorWithCalibratedWhite:0.85 alpha:1.0]
               itemValueTitleColor:[NSColor colorWithCalibratedWhite:0.95 alpha:1.0]
               listBackgroundColor:[NSColor colorWithCalibratedWhite:0.15 alpha:0.95]
                   listStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.55]
                   toolbarEndColor:[NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:0.98]
                toolbarShadowColor:[NSColor colorWithCalibratedWhite:1.0 alpha:0.3]
                 toolbarStartColor:[[NSColor colorWithCalibratedRed:0.03 green:0.23 blue:0.8 alpha:0.98] highlightWithLevel:0.55]
                toolbarStrokeColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.35]
                 toolbarTitleColor:[NSColor colorWithCalibratedWhite:1.0 alpha:1.0]
                      useDarkIcons:YES
                         nextOrder:&order];
}

#pragma mark
#pragma mark Groups

-(HWMGroup*)getGroupBySelector:(NSUInteger)selector
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"Group"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"selector == %d", selector]];

    NSError *error;

    HWMGroup *group = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"getGroupBySelector error %@", error);
    }

    return group;
}

-(HWMGroup*)insertGroupWithSelector:(NSUInteger)selector name:(NSString*)name icon:(HWMIcon*)icon nextOrder:(NSUInteger*)nextOrder
{
    HWMGroup *group = [self getGroupBySelector:selector];

    if (!group) {
        group = [NSEntityDescription insertNewObjectForEntityForName:@"Group" inManagedObjectContext:self.managedObjectContext];

        [group setOrder:[NSNumber numberWithInteger:*nextOrder]];

        *nextOrder += 1;
    }

    [group setName:name];
    [group setTitle:GetLocalizedString(group.name)];
    [group setIcon:icon];
    [group setSelector:[NSNumber numberWithInteger:selector]];
    [group setIdentifier:@"Group"];

    [group setEngine:self];

    return group;
}

#pragma mark
#pragma mark SMC Sensors

- (NSArray*)getSmcKeysFromConnection:(io_connect_t)connection excludedList:(NSArray*)excluded
{
    if (!connection)
        return nil;

    SMCVal_t val;

    SMCReadKey(connection, "#KEY", &val);

    UInt32 count = [HWMSmcSensor decodeNumericValueFromBuffer:val.bytes length:val.dataSize type:val.dataType];

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

-(HWMSmcSensor*)getSmcSensorByName:(NSString*)name
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"SmcSensor"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];

    NSError *error;

    HWMSmcSensor *sensor = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"getSmcSensorByName error %@", error);
    }

    return sensor;
}

-(HWMSmcSensor*)insertSmcSensorWithConnection:(io_connect_t)connection name:(NSString*)name type:(NSString*)type title:(NSString*)title selector:(NSUInteger)selector group:(HWMGroup*)group nextOrder:(NSUInteger*)nextOrder
{
    HWMSmcSensor *sensor = [self getSmcSensorByName:name];

    if (!sensor) {
        sensor = [NSEntityDescription insertNewObjectForEntityForName:@"SmcSensor" inManagedObjectContext:self.managedObjectContext];

        [sensor setOrder:[NSNumber numberWithInteger:*nextOrder]];

        *nextOrder += 1;
    }

    [sensor setService:[NSNumber numberWithLongLong:connection]];
    [sensor setName:name];
    [sensor setType:type];
    [sensor setSelector:[NSNumber numberWithUnsignedInteger:selector]];

    [sensor setTitle:title];
    [sensor setIdentifier:@"Sensor"];

    if (![group.sensors containsObject:sensor]) {
        [group addSensorsObject:sensor];
    }

    [sensor setEngine:self];

    return sensor;
}

-(void)insertSmcSensorsWithKeys:(NSArray*)keys connection:(io_connect_t)connection selector:(NSUInteger)selector
{
    if (!connection || !selector)
        return;

    NSString *prefix = nil;
    HWMGroup *group = nil;

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

    __block NSUInteger nextOrderValue = [self getNextOrderValueForSensorsInGroup:group];

    // Excluding all existing keys
    //    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"SmcSensor"];
    //
    //    [fetchRequest setResultType:NSDictionaryResultType];
    //    [fetchRequest setReturnsDistinctResults:YES];
    //    [fetchRequest setPropertiesToFetch:@[@"name"]];
    //
    //    NSError *error;
    //
    //    NSArray *objects = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];
    //
    //    NSArray *excludedKeys = [objects valueForKey:@"name"];

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

                            [self insertSmcSensorWithConnection:connection name:formattedKey type:[NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding] title:[NSString stringWithFormat:GetLocalizedString(title), shift + index] selector:selector group:group nextOrder:&nextOrderValue];

                        }
                    }
                }
            }
            else if ([keys indexOfObject:key] != NSNotFound /*&& [excludedKeys indexOfObject:key] == NSNotFound*/) {

                SMCVal_t info;

                if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {
                    [self insertSmcSensorWithConnection:connection name:key type:[NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding] title:GetLocalizedString(title) selector:selector group:group nextOrder:&nextOrderValue];

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
#pragma mark ATA SMART Sensors

-(HWMAtaSmartSensor*)getAtaSmartSensorByName:(NSString*)name
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"AtaSmartSensor"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];

    NSError *error;

    HWMAtaSmartSensor *sensor = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"getAtaSmartSensorByName error %@", error);
    }

    return sensor;
}

-(HWMAtaSmartSensor*)insertAtaSmartSensorFromDictionary:(NSDictionary*)attributes group:(HWMGroup*)group
{
    NSString *name = [NSString stringWithFormat:@"%@%@", [attributes objectForKey:@"serialNumber"], group.selector];

    HWMAtaSmartSensor *sensor = [self getAtaSmartSensorByName:name];

    if (!sensor) {
        sensor = [NSEntityDescription insertNewObjectForEntityForName:@"AtaSmartSensor" inManagedObjectContext:self.managedObjectContext];

        NSUInteger nextOrderValue = [self getNextOrderValueForSensorsInGroup:group];

        [sensor setOrder:[NSNumber numberWithInteger:nextOrderValue]];
    }

    [sensor setService:[attributes objectForKey:@"service"]];
    [sensor setSelector:group.selector];

    [sensor doUpdateValue];

    if (sensor.value) {
        [sensor setName:name];
        [sensor setTitle:[attributes objectForKey:@"productName"]];
        [sensor setProductName:[attributes objectForKey:@"productName"]];
        [sensor setBsdName:[attributes objectForKey:@"bsdName"]];
        [sensor setVolumeNames:[attributes objectForKey:@"volumesNames"]];
        [sensor setSerialNumber:[attributes objectForKey:@"serialNumber"]];
        [sensor setRotational:[attributes objectForKey:@"rotational"]];
        [sensor setIdentifier:@"Drive"];

        [sensor setEngine:self];

        if (![group.sensors containsObject:sensor]) {
            [group addSensorsObject:sensor];
        }
    }
    else {
        [self.managedObjectContext deleteObject:sensor];
    }

    return sensor;
}

-(void)insertAtaSmartSensors
{
    NSError *error;

    // Reset "service" attribute for all smart sensors
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"AtaSmartSensor"];
    NSArray *objects = [self.managedObjectContext executeFetchRequest:fetchRequest error:&error];

    for (HWMAtaSmartSensor *sensor in objects) {
        [sensor setService:nil];
    }

    HWMGroup *smartTemperatures = [self getGroupBySelector:kHWMGroupSmartTemperature];
    HWMGroup *smartLife = [self getGroupBySelector:kHWMGroupSmartRemainingLife];
    HWMGroup *smartBlocks = [self getGroupBySelector:kHWMGroupSmartRemainingBlocks];

    NSArray *drives = [HWMAtaSmartSensor discoverDrives];

    for (id drive in drives) {
        [self insertAtaSmartSensorFromDictionary:drive group:smartTemperatures];
        [self insertAtaSmartSensorFromDictionary:drive group:smartLife];
        [self insertAtaSmartSensorFromDictionary:drive group:smartBlocks];
    }
}

#pragma mark
#pragma mark SMC Fan Sensors

-(HWMSmcSensor*)insertSmcFanWithConnection:(io_connect_t)connection name:(NSString*)name type:(NSString*)type title:(NSString*)title selector:(NSUInteger)selector group:(HWMGroup*)group nextOrder:(NSUInteger*)nextOrder
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"SmcFanSensor"];

    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"unique == %@", title]];

    NSError *error;

    HWMSmcFanSensor *fan = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];

    if (error) {
        NSLog(@"get sensor by title error in insertSmcFanWithConnection %@", error);
    }


    if (!fan) {
        fan = [NSEntityDescription insertNewObjectForEntityForName:@"SmcFanSensor" inManagedObjectContext:self.managedObjectContext];

        [fan setOrder:[NSNumber numberWithInteger:*nextOrder]];

        *nextOrder += 1;
    }

    [fan setService:[NSNumber numberWithLongLong:connection]];
    [fan setName:name];
    [fan setType:type];
    [fan setSelector:[NSNumber numberWithUnsignedInteger:selector]];

    [fan setTitle:title];
    [fan setUnique:title];
    [fan setIdentifier:@"Sensor"];

    if (![group.sensors containsObject:fan]) {
        [group addSensorsObject:fan];
    }

    [fan setEngine:self];

    return fan;
}

- (void)insertSmcFansWithConnection:(io_connect_t)connection keys:(NSArray*)keys
{
    HWMGroup *group = [self getGroupBySelector:kHWMGroupTachometer];
    NSUInteger nextOrderValue = [self getNextOrderValueForSensorsInGroup:group];

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
                        caption = [[NSString alloc] initWithFormat:GetLocalizedString(@"Fan %X"),i + 1];

                    if (![caption hasPrefix:@"GPU "]) {

                        key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                        if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                            type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];

                            [self insertSmcFanWithConnection:connection name:key type:type title:GetLocalizedString(caption) selector:group.selector.unsignedIntegerValue group:group nextOrder:&nextOrderValue];
                        }
                    }
                }
                else if ([type isEqualToString:@TYPE_FDS]) {
                    FanTypeDescStruct *fds = (FanTypeDescStruct*)[value bytes];

                    if (fds) {
                        NSString *caption = [[NSString stringWithCString:fds->strFunction encoding:NSASCIIStringEncoding] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

                        if ([caption length] == 0)
                            caption = [[NSString alloc] initWithFormat:GetLocalizedString(@"Fan %X"), i + 1];

                        switch (fds->type) {
                            case GPU_FAN_RPM:
                            case GPU_FAN_PWM_CYCLE:
                                // Add it later as GPU sensors
                                break;

                            default:
                                key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                                if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                                    type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];

                                    [self insertSmcFanWithConnection:connection name:key type:type title:GetLocalizedString(caption) selector:group.selector.unsignedIntegerValue group:group nextOrder:&nextOrderValue];
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
    HWMGroup *group = [self getGroupBySelector:kHWMGroupTachometer];
    NSUInteger nextOrderValue = [self getNextOrderValueForSensorsInGroup:group];

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
                        NSString *title = cardIndex == 0 ? GetLocalizedString(@"GPU Fan") : [NSString stringWithFormat:GetLocalizedString(@"GPU %X Fan"), cardIndex + 1];

                        key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                        if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {

                            type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];

                            [self insertSmcFanWithConnection:connection name:key type:type title:title selector:group.selector.unsignedIntegerValue group:group nextOrder:&nextOrderValue];
                        }
                    }
                }
                else if ([type isEqualToString:@TYPE_FDS]) {
                    FanTypeDescStruct *fds = (FanTypeDescStruct*)[value bytes];

                    switch (fds->type) {
                        case GPU_FAN_RPM: {
                            NSString *title = fds->ui8Zone == 0 ? GetLocalizedString(@"GPU Fan") : [NSString stringWithFormat:GetLocalizedString(@"GPU %X Fan"), fds->ui8Zone + 1];

                            key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];

                            if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {
                                
                                type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];
                                
                                [self insertSmcFanWithConnection:connection name:key type:type title:title selector:group.selector.unsignedIntegerValue group:group nextOrder:&nextOrderValue];
                            }
                            
                            break;
                        }
                            
                        case GPU_FAN_PWM_CYCLE: {
                            NSString *title = fds->ui8Zone == 0 ? GetLocalizedString(@"GPU PWM") : [NSString stringWithFormat:GetLocalizedString(@"GPU %X PWM"), fds->ui8Zone + 1];
                            
                            key = [[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i];
                            
                            if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {
                                
                                type = [NSString stringWithCString:info.dataType encoding:NSASCIIStringEncoding];
                                
                                [self insertSmcFanWithConnection:connection name:key type:type title:title selector:kHWMGroupPWM group:group nextOrder:&nextOrderValue];
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
#pragma mark Battery Sensors

-(HWMBatterySensor*)getBatterySensorByName:(NSString*)name
{
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"BatterySensor"];
    
    [fetchRequest setPredicate:[NSPredicate predicateWithFormat:@"name == %@", name]];
    
    NSError *error;
    
    HWMBatterySensor *sensor = [[self.managedObjectContext executeFetchRequest:fetchRequest error:&error] lastObject];
    
    if (error) {
        NSLog(@"getBatterySensorByName error %@", error);
    }
    
    return sensor;
}

-(HWMBatterySensor*)insertBatterySensorFromDictionary:(NSDictionary*)attributes group:(HWMGroup*)group
{
    NSString *name = [attributes objectForKey:@"serialNumber"];
    
    if (!name || name.length == 0) {
        name = [attributes objectForKey:@"productName"];
    }
    
    HWMBatterySensor *sensor = [self getBatterySensorByName:name];
    
    if (!sensor) {
        sensor = [NSEntityDescription insertNewObjectForEntityForName:@"BatterySensor" inManagedObjectContext:self.managedObjectContext];
        
        NSUInteger nextOrderValue = [self getNextOrderValueForSensorsInGroup:group];
        
        [sensor setOrder:[NSNumber numberWithInteger:nextOrderValue]];
    }
    
    [sensor setService:[attributes objectForKey:@"service"]];
    [sensor setSelector:[attributes objectForKey:@"selector"]];
    
    [sensor doUpdateValue];
    
    if (sensor.value) {
        [sensor setName:name];
        
        switch (sensor.selector.unsignedIntegerValue) {
            case kHWMBatterySensorTypeInternal:
                [sensor setTitle:GetLocalizedString(@"Internal Battery")];
                break;
            case kHWMBatterySensorTypeKeyboard:
                [sensor setTitle:GetLocalizedString(@"Keyboard")];
                break;
            case kHWMBatterySensorTypeMouse:
                [sensor setTitle:GetLocalizedString(@"Mouse")];
                break;
            case kHWMBatterySensorTypeTrackpad:
                [sensor setTitle:GetLocalizedString(@"Trackpad")];
                break;
                
            default:
                [sensor setTitle:GetLocalizedString(@"Battery")];
                break;
        }
        
        [sensor setProductName:[attributes objectForKey:@"productName"]];
        [sensor setSerialNumber:[attributes objectForKey:@"serialNumber"]];
        
        [sensor setEngine:self];
        
        if (![group.sensors containsObject:sensor]) {
            [group addSensorsObject:sensor];
        }
    }
    else {
        [self.managedObjectContext deleteObject:sensor];
    }
    
    return sensor;
}

-(void)insertBatterySensors
{
    HWMGroup *group = [self getGroupBySelector:kHWMGroupBattery];
    
    NSArray *devices = [HWMBatterySensor discoverDevices];
    
    for (id device in devices) {
        [self insertBatterySensorFromDictionary:device group:group];
    }
}

#pragma mark
#pragma mark Graphs

-(void)generateGraphs
{

}

@end
