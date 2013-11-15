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
        [self willChangeValueForKey:@"data"];
        [self setPrimitiveValue:level forKey:@"data"];
        [self didChangeValueForKey:@"data"];
    }
}

@end
