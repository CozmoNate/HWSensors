//
//  HWMAtaSmartSensor.m
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

#import "HWMAtaSmartSensor.h"

#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMSensorsGroup.h"

#import "HWMonitorDefinitions.h"
#import "Localizer.h"

#import "HWMATASmartPluginWrapper.h"

#import <sys/mount.h>
#import <Growl/Growl.h>
#import <ReactiveCocoa/ReactiveCocoa.h>

static void block_device_appeared(void *engine, io_iterator_t iterator);
static void block_device_disappeared(void *engine, io_iterator_t iterator);

@implementation HWMAtaSmartSensor

@dynamic bsdName;
@dynamic productName;
@dynamic volumeNames;
@dynamic revision;
@dynamic serialNumber;
@dynamic rotational;

@synthesize attributes = _attributes;

static NSDictionary * gAvailableMountedPartitions = nil;

static IONotificationPortRef gHWMAtaSmartSensorNotificationPort = MACH_PORT_NULL;
static io_iterator_t gHWMAtaSmartDeviceIterator = 0;

+(void)stopWatchingForBlockStorageDevices
{
    if (gHWMAtaSmartSensorNotificationPort != MACH_PORT_NULL) {
        CFRunLoopRemoveSource(CFRunLoopGetMain(), IONotificationPortGetRunLoopSource(gHWMAtaSmartSensorNotificationPort), kCFRunLoopDefaultMode);

        IONotificationPortDestroy(gHWMAtaSmartSensorNotificationPort);

        gHWMAtaSmartSensorNotificationPort = MACH_PORT_NULL;
    }

    IOObjectRelease(gHWMAtaSmartDeviceIterator);
}

+ (void)startWatchingForBlockStorageDevicesWithEngine:(HWMEngine *)engine
{
    [HWMAtaSmartSensor stopWatchingForBlockStorageDevices];

    // Add notification ports to runloop
    gHWMAtaSmartSensorNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);

    CFRunLoopAddSource([[NSRunLoop currentRunLoop] getCFRunLoop], IONotificationPortGetRunLoopSource(gHWMAtaSmartSensorNotificationPort), kCFRunLoopDefaultMode);

    [HWMAtaSmartSensor discoverDevicesWithEngine:engine matching:IOServiceMatching("IOBlockStorageDevice")];
}

+ (void)discoverDevicesWithEngine:(HWMEngine *)engine matching:(CFDictionaryRef)matching
{
    // Retain matching dictionary so it will stay also while registering kIOTerminatedNotification
    CFRetain(matching);

    // Discover devices and add notification callbacks
    if (!IOServiceAddMatchingNotification(gHWMAtaSmartSensorNotificationPort,
                                          kIOMatchedNotification,
                                          matching,
                                          block_device_appeared,
                                          (__bridge void *)engine,
                                          &gHWMAtaSmartDeviceIterator))
    {
        // Add matched devices
        block_device_appeared((__bridge void*)engine, gHWMAtaSmartDeviceIterator);

        if (!IOServiceAddMatchingNotification(gHWMAtaSmartSensorNotificationPort,
                                              kIOTerminatedNotification,
                                              matching,
                                              block_device_disappeared,
                                              (__bridge void *)engine,
                                              &gHWMAtaSmartDeviceIterator)) {

            while (IOIteratorNext(gHWMAtaSmartDeviceIterator)) {};
        }
    }
}

+(void)updatePartitionsList
{
    NSMutableDictionary *partitions = [[NSMutableDictionary alloc] init];

    BOOL first = YES;

    NSArray *mountedVolumeURLs = [[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:@[] options:0];

    for (NSURL *url in mountedVolumeURLs) {
        NSString *path = url.path;
        struct statfs buffer;

        if (statfs([path fileSystemRepresentation], &buffer) == 0)
        {
            NSRange start = [path rangeOfString:@"/Volumes/"];

            if (first == NO && start.length == 0)
            {
                continue;
            }

            if (first)
                first = NO;

            NSString *name = [[NSString stringWithFormat:@"%s",buffer.f_mntfromname] lastPathComponent];

            if ([name hasPrefix:@"disk"] && [name length] > 4)
            {
                NSString *newName = [name substringFromIndex:4];
                NSRange paritionLocation = [newName rangeOfString:@"s"];

                if(paritionLocation.length != 0)
                    name = [NSString stringWithFormat:@"disk%@",[newName substringToIndex: paritionLocation.location]];
            }

            if( [partitions objectForKey:name] )
                [[partitions objectForKey:name] addObject:[[NSFileManager defaultManager] displayNameAtPath:path]];
            else
                [partitions setObject:[[NSMutableArray alloc] initWithObjects:[[NSFileManager defaultManager] displayNameAtPath:path], nil] forKey:name];
        }
    }

    gAvailableMountedPartitions = [partitions copy];
}

+(NSDictionary*)partitions
{
    if (!gAvailableMountedPartitions) {
        [HWMAtaSmartSensor updatePartitionsList];
    }

    return gAvailableMountedPartitions;
}

-(NSArray *)attributes
{
    if (!_attributes) {
        HWMATASmartInterfaceWrapper *wrapper = [HWMATASmartInterfaceWrapper getWrapperForBsdName:self.bsdName];

        if (!wrapper) {
            wrapper = [HWMATASmartInterfaceWrapper wrapperWithService:(io_service_t)self.service.unsignedLongLongValue
                                                              bsdName:self.bsdName
                                                          productName:self.productName
                                                             firmware:self.revision
                                                         isRotational:self.rotational.boolValue];
        }

        if (wrapper) {
            _attributes = wrapper.attributes;
        }
    }

    return _attributes;
}

-(NSString *)title
{
    switch (self.engine.configuration.driveNameSelector.unsignedIntegerValue) {
        case kHWMDriveNameVolumes:
            return self.volumeNames;

        case kHWMDriveNameBSD:
            return self.bsdName;

        case kHWMDriveNameProduct:
        default:
            break;
    }

    return self.productName;
}

-(NSString *)legend
{
    switch (self.engine.configuration.driveLegendSelector.unsignedIntegerValue) {
        case kHWMDriveNameProduct:
            return self.productName;

        case kHWMDriveNameBSD:
            return self.bsdName;

        case kHWMDriveNameVolumes:
        default:
            break;
    }

    return self.volumeNames;
}

-(void)setService:(NSNumber *)service
{
    if (self.service && self.service.unsignedLongLongValue > 0) {
        IOObjectRelease((io_service_t)self.service.unsignedLongLongValue);
    }

    [self willChangeValueForKey:@keypath(self, service)];
    [self setPrimitiveValue:service forKey:@keypath(self, service)];
    [self didChangeValueForKey:@keypath(self, service)];
}

-(void)initialize
{
    [super initialize];

    _temperatureAttributeIndex = -1;
    _remainingLifeAttributeIndex = -1;

    [[RACObserve(self, engine)
      filter:^BOOL(HWMEngine *engine) {
           return engine != nil;
       }]
     subscribeNext:^(HWMEngine *engine) {

         [[RACObserve(engine, configuration.driveNameSelector) filter:^BOOL(id value) {
             return value != nil;
         }] subscribeNext:^(id x) {
             [self willChangeValueForKey:@keypath(self, title)];
             [self didChangeValueForKey:@keypath(self, title)];
         }];

         [[RACObserve(engine, configuration.driveLegendSelector) filter:^BOOL(id value) {
             return value != nil;
         }] subscribeNext:^(id x) {
             [self willChangeValueForKey:@keypath(self, legend)];
             [self didChangeValueForKey:@keypath(self, legend)];
         }];
     }];
}

-(BOOL)findIndexOfAttributeByName:(NSString*)name outIndex:(NSInteger*)index
{
    NSArray *results = [self.attributes filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"name = %@", name]];

    *index = results && results.count ? [[results objectAtIndex:0][@"index"] unsignedIntegerValue] : -1;

    return *index > -1;
}

-(void)updateVolumeNames
{
    NSString *volumes = [[[HWMAtaSmartSensor partitions] objectForKey:self.bsdName] componentsJoinedByString:@", "];

    if (!volumes) {
        NSString *bsdUnit = bsdname_from_service((io_registry_entry_t)self.service.unsignedLongLongValue);
        volumes = [[[HWMAtaSmartSensor partitions] objectForKey:bsdUnit] componentsJoinedByString:@", "];
    }

    [self setPrimitiveValue:volumes ? volumes : self.bsdName forKey:@"volumeNames"];
}

-(NSNumber*)getTemperature
{
    if (_temperatureAttributeIndex < 0) {

        if (![self findIndexOfAttributeByName:@"Temperature_Celsius" outIndex:&_temperatureAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Airflow_Temperature_Cel" outIndex:&_temperatureAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Temperature_Case" outIndex:&_temperatureAttributeIndex])
        {
            return nil;
        }
    }

    if (!self.attributes || _temperatureAttributeIndex >= self.attributes.count)
        return nil;

    NSNumber *raw = [self.attributes objectAtIndex:_temperatureAttributeIndex][@"raw"];

    return [NSNumber numberWithUnsignedShort:raw.unsignedShortValue];
}

-(NSNumber*)getRemainingLife
{
    if (_remainingLifeAttributeIndex < 0) {

        if (![self findIndexOfAttributeByName:@"SSD_Life_Left" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Remaining_Lifetime_Perc" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Perc_Rated_Life_Used" outIndex:&_remainingLifeAttributeIndex] &&
            /*![self findIndexOfAttributeByName:@"Wear_Leveling_Count" outIndex:&_remainingLifeAttributeIndex] &&*/
            ![self findIndexOfAttributeByName:@"Media_Wearout_Indicator" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Available_Reservd_Space" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Reserve_Block_Count" outIndex:&_remainingLifeAttributeIndex] &&
            ![self findIndexOfAttributeByName:@"Bad_Block_Count" outIndex:&_remainingLifeAttributeIndex])
        {
            return nil;
        }
    }

    if (!self.attributes || _remainingLifeAttributeIndex >= self.attributes.count)
        return nil;

    return [self.attributes objectAtIndex:_remainingLifeAttributeIndex][@"value"];
}

-(NSUInteger)internalUpdateAlarmLevel
{
    float floatValue = self.value.floatValue;

    switch (self.selector.unsignedIntegerValue) {

        case kHWMGroupSmartTemperature:
            if (self.rotational.boolValue) {
                return  floatValue >= 55 ? kHWMSensorLevelExceeded :
                floatValue >= 50 ? kHWMSensorLevelHigh :
                floatValue >= 40 ? kHWMSensorLevelModerate :
                kHWMSensorLevelNormal;
            }
            else {
                return  floatValue >= 100 ? kHWMSensorLevelExceeded :
                floatValue >= 85 ? kHWMSensorLevelHigh :
                floatValue >= 70 ? kHWMSensorLevelModerate :
                kHWMSensorLevelNormal;
            }

        case kHWMGroupSmartRemainingLife:
            return  floatValue < 10 ? kHWMSensorLevelExceeded :
            floatValue < 50 ? kHWMSensorLevelHigh :
            floatValue < 70 ? kHWMSensorLevelModerate :
            kHWMSensorLevelNormal;

        default:
            break;
    }

    return _alarmLevel;
}

-(NSNumber *)internalUpdateValue
{
    if (self.hidden.boolValue)
        return nil;

    [self willChangeValueForKey:@keypath(self, attributes)];

    NSNumber *value = nil;

    _attributes = nil;

    switch (self.selector.unsignedIntegerValue) {
        case kHWMGroupTemperature:
        case kHWMGroupSmartTemperature:
            value = [self getTemperature];
            break;

        case kHWMGroupSmartRemainingLife:
            value = [self getRemainingLife];
            break;
    }

    [self didChangeValueForKey:@keypath(self, attributes)];

    return value;
}

-(void)internalSendAlarmNotification
{
    if (self.selector.unsignedIntegerValue == kHWMGroupSmartRemainingLife) {
        switch (_alarmLevel) {
            case kHWMSensorLevelExceeded:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ is exceeded limit"), self.title]
                                       notificationName:NotifierSensorLevelExceededNotification
                                               iconData:nil
                                               priority:1000
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelHigh:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ is critical"), self.title]
                                       notificationName:NotifierSensorLevelHighNotification
                                               iconData:nil
                                               priority:500
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelModerate:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ lifespan is low"), self.title]
                                       notificationName:NotifierSensorLevelModerateNotification
                                               iconData:nil
                                               priority:0
                                               isSticky:YES
                                           clickContext:nil];
                break;
                
            default:
                break;
        }
    }
    else {
        [super internalSendAlarmNotification];
    }
}

@end

NSString* bsdname_from_service(io_registry_entry_t object)
{
    CFMutableDictionaryRef propertiesRef;

    if (KERN_SUCCESS == IORegistryEntryCreateCFProperties(object, &propertiesRef, kCFAllocatorDefault, 0)) {

        NSDictionary *properties = CFBridgingRelease(propertiesRef);

        NSString *bsdName = [properties[@"BSD Name"] copy];
        NSNumber *open = properties[@"Open"];
        NSNumber *leaf = properties[@"Leaf"];
        NSNumber *whole = properties[@"Whole"];

        if (bsdName &&
            open && open.boolValue &&
            leaf && leaf.boolValue &&
            whole && whole.boolValue) {
            return bsdName;
        }
    }

    io_iterator_t childIterator;

    if (KERN_SUCCESS == IORegistryEntryGetChildIterator(object, kIOServicePlane, &childIterator)) {

        io_registry_entry_t child;
        NSString *bsdName = nil;

        while ((child = IOIteratorNext(childIterator))) {

            bsdName = bsdname_from_service(child);

            IOObjectRelease(child);

            if (bsdName) {
                break;
            }
        }

        IOObjectRelease(childIterator);

        return bsdName;
    }

    return nil;
}

static void block_device_appeared(void *engine, io_iterator_t iterator)
{
    dispatch_async(dispatch_get_main_queue(), ^{

        io_object_t object;

        __block NSMutableArray *devices = [NSMutableArray array];

        while ((object = IOIteratorNext(iterator))) {

            NSLog(@"ATA block storage device appeared %u", object);

            CFMutableDictionaryRef propertiesRef;

            if (KERN_SUCCESS == IORegistryEntryCreateCFProperties(object, &propertiesRef, kCFAllocatorDefault, 0)) {

                NSDictionary *properties = CFBridgingRelease(propertiesRef);

                NSNumber *capable = properties[@kIOPropertySMARTCapableKey];

                if (capable && capable.boolValue) {

                    NSDictionary * characteristics = properties[@"Device Characteristics"];

                    if (characteristics) {
                        NSString *name = [(NSString*)characteristics[@"Product Name"] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
                        NSString *medium = characteristics[@"Medium Type"];
                        NSString *revision = [(NSString*)characteristics[@"Product Revision Level"] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

                        NSString *serial = [(NSString*)characteristics[@"Serial Number"] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

                        // Try to obtain serial number from device nub
                        if (!serial) {
                            CFStringRef serialRef = IORegistryEntrySearchCFProperty(object, kIOServicePlane, CFSTR("device serial"), kCFAllocatorDefault, kIORegistryIterateParents | kIORegistryIterateRecursively);

                            if (MACH_PORT_NULL != serialRef) {
                                serial = CFBridgingRelease(serialRef);
                            }
                        }

                        if (name && revision) {
                            NSString *bsdName;
                            NSString *volumes;

                            CFStringRef bsdNameRef = IORegistryEntrySearchCFProperty(object, kIOServicePlane, CFSTR("BSD Name"), kCFAllocatorDefault, kIORegistryIterateRecursively);

                            if (MACH_PORT_NULL != bsdNameRef) {
                                bsdName = CFBridgingRelease(bsdNameRef);
                                volumes = [[[HWMAtaSmartSensor partitions] objectForKey:bsdName] componentsJoinedByString:@", "];

                                if (!volumes) {
                                    NSString *bsdUnit = bsdname_from_service(object);
                                    volumes = [[[HWMAtaSmartSensor partitions] objectForKey:bsdUnit] componentsJoinedByString:@", "];
                                }

                                if (bsdName) {
                                    [devices addObject:@{@"service" : [NSNumber numberWithUnsignedLongLong:object],
                                                         @"productName": name,
                                                         @"bsdName" :bsdName,
                                                         @"volumesNames" : (volumes ? volumes : bsdName) ,
                                                         @"serialNumber" : serial ? serial : revision,
                                                         @"revision" : revision,
                                                         @"rotational" : [NSNumber numberWithBool:medium ? ![medium isEqualToString:@"Solid State"] : TRUE]}
                                     ];
                                }
                            }
                        }
                    }
                }
            }
        }

        [[NSNotificationCenter defaultCenter] postNotificationName:HWMAtaSmartSensorDidAddBlockStorageDevices object:(__bridge HWMEngine*)(engine) userInfo:@{@"addedDevices": devices}];
    });
}

static void block_device_disappeared(void *engine, io_iterator_t iterator)
{
    io_object_t object;
    __block NSMutableArray *devices = [NSMutableArray array];

    while ((object = IOIteratorNext(iterator))) {
        NSLog(@"ATA block storage device disappeared %u", object);
        
        [devices addObject:[NSNumber numberWithUnsignedLongLong:object]];
        
        IOObjectRelease(object);
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:HWMAtaSmartSensorDidAddBlockStorageDevices object:(__bridge HWMEngine*)(engine) userInfo:@{@"removedDevices": devices}];
}
