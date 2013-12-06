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

@implementation HWMBatterySensor

@dynamic productName;
@dynamic serialNumber;
@dynamic selector;

+ (NSArray*)discoverDevices
{
    NSMutableArray *devices = [[NSMutableArray alloc] init];
    NSInteger type;
    int count = 0;

    do {
        CFDictionaryRef matching;

        switch (count++) {
            case 0:
                matching = IOServiceMatching("IOPMPowerSource");
                type = kHWMGroupBatteryInternal;
                break;

            case 1:
                matching = IOServiceMatching("BNBMouseDevice");
                type = kHWMGroupBatteryMouse;
                break;

            case 2:
                matching = IOServiceMatching("AppleBluetoothHIDKeyboard");
                type = kHWMGroupBatteryKeyboard;
                break;

            case 3:
                matching = IOServiceMatching("BNBTrackpadDevice");
                type = kHWMGroupBatteryTrackpad;
                break;

            default:
                matching = MACH_PORT_NULL;
                type = 0;
                break;
        }

        if (MACH_PORT_NULL != matching) {

            io_iterator_t iterator = IO_OBJECT_NULL;

            if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
                if (IO_OBJECT_NULL != iterator) {

                    io_service_t service = MACH_PORT_NULL;

                    while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {

                        NSString *serialNumber;
                        NSString *productName;

                        switch (type) {
                            case kHWMGroupBatteryInternal:
                                productName = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("DeviceName"), kCFAllocatorDefault, 0);
                                serialNumber = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("BatterySerialNumber"), kCFAllocatorDefault, 0);

                                if (!serialNumber)
                                    serialNumber = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("SerialNumber"), kCFAllocatorDefault, 0);
                                break;

                            default:
                                productName = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("Product"), kCFAllocatorDefault, 0);
                                serialNumber = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("SerialNumber"), kCFAllocatorDefault, 0);
                                break;
                        }

                        [devices addObject:@{   @"service" : [NSNumber numberWithUnsignedLongLong:service],
                                                @"selector" : [NSNumber numberWithUnsignedLongLong:type],
                                                @"serialNumber" : serialNumber ? serialNumber : @"",
                                                @"productName" : productName ? productName : @""}];
                    }

                    IOObjectRelease(iterator);
                }
            }
        }
    } while (type);

    //TEST BATTERY
//    [devices addObject:@{   @"service" : @1,
//                            @"selector" : @kHWMGroupBatteryMouse,
//                            @"serialNumber" : @"123",
//                            @"productName" : @"test battery"}];

    return devices;
}

-(void)prepareForDeletion
{
    IOObjectRelease((io_service_t)self.service.unsignedLongLongValue);
}

-(NSUInteger)internalUpdateAlarmLevel
{
    float floatValue = self.value.floatValue;

    switch (self.selector.unsignedIntegerValue) {

        case kHWMGroupBatteryInternal:
        case kHWMGroupBatteryKeyboard:
        case kHWMGroupBatteryMouse:
        case kHWMGroupBatteryTrackpad:
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
    io_service_t service = (io_service_t)self.service.unsignedLongLongValue;

    if (MACH_PORT_NULL != service) {
        switch (self.selector.unsignedIntegerValue) {
            case kHWMGroupBatteryInternal: {
                NSNumber *max = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, CFSTR("MaxCapacity"), kCFAllocatorDefault, 0);
                NSNumber *current = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, CFSTR("CurrentCapacity"), kCFAllocatorDefault, 0);

                if (max && current && [max doubleValue] > 0) {
                    double percent = (([current doubleValue] / [max doubleValue]) + 0.005) * 100;
                    return [NSNumber numberWithFloat:percent];
                }

                break;
            }

            default:
                return (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, CFSTR("BatteryPercent"), kCFAllocatorDefault, 0);
        }

    }

    return @0;
}

@end
