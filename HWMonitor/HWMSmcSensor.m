//
//  HWMSmcSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "HWMSmcSensor.h"

#import "FakeSMCDefinitions.h"
#import "smc.h"

@implementation HWMSmcSensor

@dynamic connection;

+ (int)getIndexOfHexChar:(char)c
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

+ (BOOL)isValidIntegerSmcType:(NSString *)type
{
    if (type && [type length] >= 3) {
        if (([type characterAtIndex:0] == 'u' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'i') {

            switch ([type characterAtIndex:2]) {
                case '8':
                    return YES;
                case '1':
                    return [type characterAtIndex:3] == '6' ? YES : NO;
                case '3':
                    return [type characterAtIndex:3] == '2'? YES : NO;
            }
        }
    }

    return NO;
}

+ (BOOL)isValidFloatingSmcType:(NSString *)type
{
    if (type && [type length] >= 3) {
        if (([type characterAtIndex:0] == 'f' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'p') {
            UInt8 i = [HWMSmcSensor getIndexOfHexChar:[type characterAtIndex:2]];
            UInt8 f = [HWMSmcSensor getIndexOfHexChar:[type characterAtIndex:3]];

            if (i + f != ([type characterAtIndex:0] == 's' ? 15 : 16))
                return NO;

            return YES;
        }
    }

    return NO;
}

+ (float)decodeNumericValueFromBuffer:(void *)data length:(NSUInteger)length type:(const char *)type
{
    if (type || data) {

        size_t typeLength = strnlen(type, 4);

        if (typeLength >= 3) {
            if ((type[0] == 'u' || type[0] == 's') && type[1] == 'i') {

                BOOL signd = type[0] == 's';

                switch (type[2]) {
                    case '8':
                        if (length == 1) {
                            UInt8 encoded = 0;

                            bcopy(data, &encoded, 1);

                            if (signd && bit_get(encoded, BIT(7))) {
                                bit_clear(encoded, BIT(7));
                                return -encoded;
                            }

                            return encoded;
                        }
                        break;

                    case '1':
                        if (type[3] == '6' && length == 2) {
                            UInt16 encoded = 0;

                            bcopy(data, &encoded, 2);

                            encoded = OSSwapBigToHostInt16(encoded);

                            if (signd && bit_get(encoded, BIT(15))) {
                                bit_clear(encoded, BIT(15));
                                return -encoded;
                            }

                            return encoded;
                        }
                        break;

                    case '3':
                        if (type[3] == '2' && length == 4) {
                            UInt32 encoded = 0;

                            bcopy(data, &encoded, 4);

                            encoded = OSSwapBigToHostInt32(encoded);

                            if (signd && bit_get(encoded, BIT(31))) {
                                bit_clear(encoded, BIT(31));
                                return -encoded;
                            }

                            return encoded;
                        }
                        break;
                }
            }
            else if (typeLength > 0 && (type[0] == 'f' || type[0] == 's') && type[1] == 'p' && length == 2) {
                UInt16 encoded = 0;

                bcopy(data, &encoded, 2);

                UInt8 i = [HWMSmcSensor getIndexOfHexChar:type[2]];
                UInt8 f = [HWMSmcSensor getIndexOfHexChar:type[3]];

                if (i + f != (type[0] == 's' ? 15 : 16) )
                    return MAXFLOAT;
                
                UInt16 swapped = OSSwapBigToHostInt16(encoded);
                
                BOOL signd = type[0] == 's';
                BOOL minus = bit_get(swapped, BIT(15));
                
                if (signd && minus) bit_clear(swapped, BIT(15));
                
                return ((float)swapped / (float)BIT(f)) * (signd && minus ? -1 : 1);
            }
        }
    }
    
    return MAXFLOAT;
}

- (void)doUpdateValue
{
    SMCVal_t info;

    if (kIOReturnSuccess == SMCReadKey((io_connect_t)self.connection.unsignedLongValue, self.name.UTF8String, &info)) {

        NSNumber *value = [NSNumber numberWithFloat:[HWMSmcSensor decodeNumericValueFromBuffer:info.bytes length:info.dataSize type:[self.type cStringUsingEncoding:NSASCIIStringEncoding]]];

        if (value && (!self.value || ![value isEqualToNumber:self.value])) {
            [self willChangeValueForKey:@"value"];
            [self setPrimitiveValue:value forKey:@"value"];
            [self didChangeValueForKey:@"value"];

            [self didChangeValueForKey:@"formattedValue"];
        }
    }
}


@end
