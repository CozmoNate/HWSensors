//
//  HWMBatterySensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMBatterySensor.h"


@implementation HWMBatterySensor

@dynamic selector;
@dynamic service;

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
                type = kHWMBatterySensorTypeInternal;
                break;

            case 1:
                matching = IOServiceMatching("BNBMouseDevice");
                type = kHWMBatterySensorTypeMouse;
                break;

            case 2:
                matching = IOServiceMatching("AppleBluetoothHIDKeyboard");
                type = kHWMBatterySensorTypeKeyboard;
                break;

            case 3:
                matching = IOServiceMatching("BNBTrackpadDevice");
                type = kHWMBatterySensorTypeTrackpad;
                break;

            default:
                matching = MACH_PORT_NULL;
                type = kHWMBatterySensorTypeNone;
                break;
        }

        if (MACH_PORT_NULL != matching) {

            io_iterator_t iterator = IO_OBJECT_NULL;

            if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
                if (IO_OBJECT_NULL != iterator) {

                    io_service_t service = MACH_PORT_NULL;

                    while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {

                        [devices addObject:[NSDictionary dictionaryWithObjectsAndKeys:
                                            [NSNumber numberWithUnsignedLongLong:service], @"service",
                                            [NSNumber numberWithUnsignedLongLong:type], @"selector",
                                            nil]];

                    }

                    IOObjectRelease(iterator);
                }
            }
        }
    } while (type != kHWMBatterySensorTypeNone);

    
    return devices;
}

-(void)dealloc
{
    IOObjectRelease((io_service_t)self.service.unsignedLongLongValue);
}

-(void)doUpdateValue
{
    NSNumber *level = nil;

    io_service_t service = (io_service_t)self.service.unsignedLongLongValue;

    if (MACH_PORT_NULL != service) {
        switch (self.selector.unsignedIntegerValue) {
            case kHWMBatterySensorTypeInternal: {
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

    if (level) {
        [self willChangeValueForKey:@"value"];
        [self setPrimitiveValue:level forKey:@"value"];
        [self didChangeValueForKey:@"value"];
    }
}

@end
