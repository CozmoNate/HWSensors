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

#import <IOKit/hid/IOHIDKeys.h>

const NSString *kHWMBatterySensorMaxCapacity            = @"MaxCapacity";
const NSString *kHWMBatterySensorCurrentCapacity        = @"CurrentCapacity";
const NSString *kHWMBatterySensorBatteryPercent         = @"BatteryPercent";
const NSString *kHWMBatterySensorProductName            = @kIOHIDProductKey;
const NSString *kHWMBatterySensorDeviceName             = @"DeviceName";
const NSString *kHWMBatterySensorSerialNumber           = @kIOHIDSerialNumberKey;
const NSString *kHWMBatterySensorBatterySerialNumber    = @"BatterySerialNumber";

static IONotificationPortRef gHWMBatterySensorNotificationPort = MACH_PORT_NULL;
static io_iterator_t gHWMBatteryDeviceIterator = 0;

static void hid_device_appeared(void *engine, io_iterator_t iterator)
{
    io_object_t object;

    while ((object = IOIteratorNext(iterator))) {

        NSUInteger deviceType = 0;

        if ((__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(object, (__bridge CFStringRef)(kHWMBatterySensorMaxCapacity), kCFAllocatorDefault, 0) &&
            (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(object, (__bridge CFStringRef)(kHWMBatterySensorCurrentCapacity), kCFAllocatorDefault, 0)) {
            deviceType = kHWMBatterySensorInternal;
        }
        else if ((__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(object, (__bridge CFStringRef)(kHWMBatterySensorBatteryPercent), kCFAllocatorDefault, 0)) {
            deviceType = kHWMBatterySensorHID;
        }

        NSString *productName;
        NSString *serialNumber;

        switch (deviceType) {
            case kHWMBatterySensorInternal:
                //productName = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(object, (__bridge CFStringRef)(kHWMBatterySensorDeviceName), kCFAllocatorDefault, 0);
                productName = @"Internal Battery";
                serialNumber = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(object, (__bridge CFStringRef)(kHWMBatterySensorBatterySerialNumber), kCFAllocatorDefault, 0);
                break;

            case kHWMBatterySensorHID:
                productName = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(object, (__bridge CFStringRef)(kHWMBatterySensorProductName), kCFAllocatorDefault, 0);
                serialNumber = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(object, (__bridge CFStringRef)(kHWMBatterySensorSerialNumber), kCFAllocatorDefault, 0);
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

        [(__bridge HWMEngine*)engine systemDidAddBatteryDevice:properties];
    }
}
static void hid_device_disappeared(void *engine, io_iterator_t iterator)
{
    io_object_t object;

    while ((object = IOIteratorNext(iterator))) {

        [(__bridge HWMEngine*)engine systemDidRemoveBatteryDevice:object];

        IOObjectRelease(object);
    }
}

@implementation HWMBatterySensor

@synthesize deviceType;

+ (void)discoverDevicesWithEngine:(HWMEngine *)engine matching:(CFDictionaryRef)matching
{
    // Retain matching dictionary so it will stay also while registering kIOTerminatedNotification
    CFRetain(matching);

    // Discover devices and add notification callbacks
    if (!IOServiceAddMatchingNotification(gHWMBatterySensorNotificationPort,
                                         kIOFirstMatchNotification,
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

+ (void)discoverBatteryDevicesWithEngine:(HWMEngine *)engine
{
    [HWMBatterySensor stopWatchingForBatteryDevices];

    // Add notification ports to runloop
    gHWMBatterySensorNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);

    CFRunLoopAddSource([[NSRunLoop currentRunLoop] getCFRunLoop], IONotificationPortGetRunLoopSource(gHWMBatterySensorNotificationPort), kCFRunLoopDefaultMode);

    [HWMBatterySensor discoverDevicesWithEngine:engine matching:IOServiceMatching("IOPMPowerSource")];
    [HWMBatterySensor discoverDevicesWithEngine:engine matching:IOServiceMatching("IOHIDDevice")];
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
        case kHWMBatterySensorHID:
            return floatValue < 5 ? kHWMSensorLevelExceeded :
                   floatValue < 10 ? kHWMSensorLevelHigh :
                   floatValue < 30 ? kHWMSensorLevelModerate :
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
                NSNumber *max = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, (__bridge CFStringRef)(kHWMBatterySensorMaxCapacity), kCFAllocatorDefault, 0);
                NSNumber *current = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, (__bridge CFStringRef)(kHWMBatterySensorCurrentCapacity), kCFAllocatorDefault, 0);

                if (max && current && [max doubleValue] > 0) {
                    double percent = (([current doubleValue] / [max doubleValue]) + 0.005) * 100;
                    return [NSNumber numberWithFloat:percent];
                }

                break;
            }

            case kHWMBatterySensorHID:
                return (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, (__bridge CFStringRef)(kHWMBatterySensorBatteryPercent), kCFAllocatorDefault, 0);
        }

    }

    return nil;
}

@end
