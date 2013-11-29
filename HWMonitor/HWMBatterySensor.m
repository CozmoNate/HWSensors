//
//  HWMBatterySensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

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
                                                @"serialNumber" : serialNumber,
                                                @"productName" : productName}];
                    }

                    IOObjectRelease(iterator);
                }
            }
        }
    } while (type);

//    TEST BATTERY
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

-(void)doUpdateValue
{
    NSNumber *level = nil;

    io_service_t service = (io_service_t)self.service.unsignedLongLongValue;

    if (MACH_PORT_NULL != service) {
        switch (self.selector.unsignedIntegerValue) {
            case kHWMGroupBatteryInternal: {
                NSNumber *max = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, CFSTR("MaxCapacity"), kCFAllocatorDefault, 0);
                NSNumber *current = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, CFSTR("CurrentCapacity"), kCFAllocatorDefault, 0);

                if (max && current && [max doubleValue] > 0) {
                    double percent = (([current doubleValue] / [max doubleValue]) + 0.005) * 100;
                    level = [NSNumber numberWithDouble:percent];
                }

                break;
            }

            default:
                level = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(service, CFSTR("BatteryPercent"), kCFAllocatorDefault, 0);
                break;
        }

    }

    if (level && (!self.value || ![level isEqualToNumber:self.value])) {
        [self willChangeValueForKey:@"value"];
        [self willChangeValueForKey:@"formattedValue"];

        [self setPrimitiveValue:level forKey:@"value"];

        [self didChangeValueForKey:@"value"];
        [self didChangeValueForKey:@"formattedValue"];
    }
}

@end
