//
//  HWMBatterySensor.m
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

#import "HWMBatterySensor.h"
#import "HWMSensorsGroup.h"
#import "HWMEngine.h"

#import "HWMonitorDefinitions.h"
#import "Localizer.h"

#import <IOKit/hid/IOHIDKeys.h>
#import <Growl/Growl.h>

#define kHWMBatterySensorInternal       1
#define kHWMBatterySensorBluetooth      2

const NSString *kHWMBatterySensorMaxCapacity            = @"MaxCapacity";
const NSString *kHWMBatterySensorCurrentCapacity        = @"CurrentCapacity";
const NSString *kHWMBatterySensorBatteryPercent         = @"BatteryPercent";
const NSString *kHWMBatterySensorProductName            = @kIOHIDProductKey;
const NSString *kHWMBatterySensorDeviceName             = @"DeviceName";
const NSString *kHWMBatterySensorSerialNumber           = @kIOHIDSerialNumberKey;
const NSString *kHWMBatterySensorBatterySerialNumber    = @"BatterySerialNumber";

static IONotificationPortRef gHWMBatterySensorNotificationPort = MACH_PORT_NULL;
static io_iterator_t gHWMBatteryDeviceIterator = 0;

static NSNumber* registry_entry_read_number(io_registry_entry_t entry, const NSString* key)
{
    return (__bridge_transfer NSNumber*)IORegistryEntryCreateCFProperty(entry, (__bridge CFStringRef)(key), kCFAllocatorDefault, 0);
}

static NSString* registry_entry_read_string(io_registry_entry_t entry, const NSString* key)
{
    return (__bridge_transfer NSString*)IORegistryEntryCreateCFProperty(entry, (__bridge CFStringRef)(key), kCFAllocatorDefault, 0);
}

static void hid_device_appeared(void *engine, io_iterator_t iterator)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        io_object_t object;

        __block NSMutableArray *devices = [NSMutableArray array];

        while ((object = IOIteratorNext(iterator))) {

            NSLog(@"battery device appeared %u", object);

            NSUInteger deviceType = 0;
            NSUInteger tryCount = 0;

#ifndef kHWMonitorDebugBattery
            do {
                // Internal battery check
                if (registry_entry_read_number(object, kHWMBatterySensorMaxCapacity) &&
                    registry_entry_read_number(object, kHWMBatterySensorCurrentCapacity)) {
                    deviceType = kHWMBatterySensorInternal;
                }
                // Bluetooth device check
                else if (registry_entry_read_number(object, kHWMBatterySensorBatteryPercent)) {
                    deviceType = kHWMBatterySensorBluetooth;
                }

                [NSThread sleepForTimeInterval:5];

            } while (!deviceType && tryCount++ < 5);
#else
            deviceType = kHWMBatterySensorBluetooth;
#endif

            NSString *productName;
            NSString *serialNumber;

            switch (deviceType) {
                case kHWMBatterySensorInternal:
                    //productName = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(object, (__bridge CFStringRef)(kHWMBatterySensorDeviceName), kCFAllocatorDefault, 0);
                    productName = @"Internal Battery";
                    serialNumber = registry_entry_read_string(object, kHWMBatterySensorBatterySerialNumber);
                    break;

                case kHWMBatterySensorBluetooth:
                    productName = registry_entry_read_string(object, kHWMBatterySensorProductName);
                    serialNumber = registry_entry_read_string(object, kHWMBatterySensorSerialNumber);
                    break;
            }

            if (!productName) {
                continue;
            }

            if (!serialNumber) {
                serialNumber = productName;
            }

            NSDictionary *properties = @{@"service"        : [NSNumber numberWithUnsignedLongLong:object],
                                         @"deviceType"     : [NSNumber numberWithUnsignedLongLong:deviceType],
                                         @"serialNumber"   : serialNumber,
                                         @"productName"    : productName};
            
            [devices addObject:properties];
        }
        
        [(__bridge HWMEngine*)engine systemDidAddBatteryDevices:devices];
    });
}
static void hid_device_disappeared(void *engine, io_iterator_t iterator)
{
    io_object_t object;
    __block NSMutableArray *devices = [NSMutableArray array];

    while ((object = IOIteratorNext(iterator))) {
        NSLog(@"battery device disappeared %u", object);

        [devices addObject:[NSNumber numberWithUnsignedLongLong:object]];

        IOObjectRelease(object);
    }

    [(__bridge HWMEngine*)engine systemDidRemoveBatteryDevices:devices];
}

@implementation HWMBatterySensor

@synthesize deviceType;

+ (void)discoverDevicesWithEngine:(HWMEngine *)engine matching:(CFDictionaryRef)matching
{
    // Retain matching dictionary so it will stay also while registering kIOTerminatedNotification
    CFRetain(matching);

    // Discover devices and add notification callbacks
    if (!IOServiceAddMatchingNotification(gHWMBatterySensorNotificationPort,
                                         kIOMatchedNotification,
                                         matching,
                                         hid_device_appeared,
                                         (__bridge void *)engine,
                                         &gHWMBatteryDeviceIterator))
    {
        // Add matched devices
        hid_device_appeared((__bridge void*)engine, gHWMBatteryDeviceIterator);

        if (!IOServiceAddMatchingNotification(gHWMBatterySensorNotificationPort,
                                              kIOTerminatedNotification,
                                              matching,
                                              hid_device_disappeared,
                                              (__bridge void *)engine,
                                              &gHWMBatteryDeviceIterator)) {

            while (IOIteratorNext(gHWMBatteryDeviceIterator)) {};
        }
    }
}

+(void)stopWatchingForBatteryDevices
{
    if (gHWMBatterySensorNotificationPort != MACH_PORT_NULL) {
        CFRunLoopRemoveSource(CFRunLoopGetMain(), IONotificationPortGetRunLoopSource(gHWMBatterySensorNotificationPort), kCFRunLoopDefaultMode);
        IONotificationPortDestroy(gHWMBatterySensorNotificationPort);

        gHWMBatterySensorNotificationPort = MACH_PORT_NULL;
    }

    IOObjectRelease(gHWMBatteryDeviceIterator);
}

+ (void)startWatchingForBatteryDevicesWithEngine:(HWMEngine *)engine
{
    [HWMBatterySensor stopWatchingForBatteryDevices];

    // Add notification ports to runloop
    gHWMBatterySensorNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);

    CFRunLoopAddSource([[NSRunLoop currentRunLoop] getCFRunLoop], IONotificationPortGetRunLoopSource(gHWMBatterySensorNotificationPort), kCFRunLoopDefaultMode);

    [HWMBatterySensor discoverDevicesWithEngine:engine matching:IOServiceMatching("IOPMPowerSource")];
    [HWMBatterySensor discoverDevicesWithEngine:engine matching:IOServiceMatching("IOAppleBluetoothHIDDriver")];
#ifdef kHWMonitorDebugBattery
    [HWMBatterySensor discoverDevicesWithEngine:engine matching:IOServiceMatching("IOHIDDevice")];
#endif
}

-(void)prepareForDeletion
{
    self.service = @0;

    IOObjectRelease((io_registry_entry_t)self.service.unsignedLongLongValue);
}

-(NSUInteger)internalUpdateAlarmLevel
{
    float floatValue = self.value.floatValue;

    switch (self.deviceType) {

        case kHWMBatterySensorInternal:
        case kHWMBatterySensorBluetooth:
            return floatValue < 5 ? kHWMSensorLevelExceeded :
                   floatValue < 10 ? kHWMSensorLevelHigh :
                   floatValue < 20 ? kHWMSensorLevelModerate :
                   kHWMSensorLevelNormal;

        default:
            break;
    }

    return _alarmLevel;
}

-(NSNumber *)internalUpdateValue
{
    io_registry_entry_t service = (io_registry_entry_t)self.service.unsignedLongLongValue;

    if (MACH_PORT_NULL != service) {
        switch (self.deviceType) {
            case kHWMBatterySensorInternal: {
                NSNumber *max = registry_entry_read_number(service, kHWMBatterySensorMaxCapacity);
                NSNumber *current = registry_entry_read_number(service, kHWMBatterySensorCurrentCapacity);

                if (max && current && [max doubleValue] > 0) {
                    double percent = (([current doubleValue] / [max doubleValue]) + 0.005) * 100;
                    return [NSNumber numberWithFloat:percent];
                }

                break;
            }

            case kHWMBatterySensorBluetooth:
                return registry_entry_read_number(service, kHWMBatterySensorBatteryPercent);
        }

    }

    return nil;
}

-(void)internalSendAlarmNotification
{
    // Draining
    if ([self.value isLessThan:_previousAlaramLevelValue]) {
        switch (_alarmLevel) {
            case kHWMSensorLevelExceeded:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ completely discharged!"), self.title]
                                       notificationName:NotifierSensorLevelExceededNotification
                                               iconData:nil
                                               priority:1000
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelHigh:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ needs to be recharged"), self.title]
                                       notificationName:NotifierSensorLevelHighNotification
                                               iconData:nil
                                               priority:500
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelModerate:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ is low"), self.title]
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

    _previousAlaramLevelValue = [self.value copy];
}

@end
