//
//  HWMSmcSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMSmcSensor.h"

#import "FakeSMCDefinitions.h"
#import "SmcHelper.h"
#import "smc.h"

@implementation HWMSmcSensor

- (void)doUpdateValue
{
    SMCVal_t info;

    if (kIOReturnSuccess == SMCReadKey((io_connect_t)self.service.unsignedLongValue, self.name.UTF8String, &info)) {

        NSNumber *value = [NSNumber numberWithFloat:[SmcHelper decodeNumericValueFromBuffer:info.bytes length:info.dataSize type:[self.type cStringUsingEncoding:NSASCIIStringEncoding]]];

        if (value && (!self.value || ![value isEqualToNumber:self.value])) {
            [self willChangeValueForKey:@"value"];
            [self willChangeValueForKey:@"formattedValue"];

            [self setPrimitiveValue:value forKey:@"value"];

            [self didChangeValueForKey:@"value"];
            [self didChangeValueForKey:@"formattedValue"];
        }
    }
}


@end
