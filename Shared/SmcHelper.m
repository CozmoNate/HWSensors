//
//  SmcHelper.m
//  HWMonitor
//
//  Created by Kozlek on 28/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "SmcHelper.h"
#import "smc.h"
#import "FakeSMCDefinitions.h"

@implementation SmcHelper

+ (int)getIndexFromHexChar:(char)c
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : -1;
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
                    return [type characterAtIndex:3] == '2' ? YES : NO;
            }
        }
    }

    return NO;
}

+ (BOOL)isValidFloatingSmcType:(NSString *)type
{
    if (type && [type length] >= 3) {
        if (([type characterAtIndex:0] == 'f' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'p') {
            UInt8 i = [SmcHelper getIndexFromHexChar:[type characterAtIndex:2]];
            UInt8 f = [SmcHelper getIndexFromHexChar:[type characterAtIndex:3]];

            if (i + f != ([type characterAtIndex:0] == 's' ? 15 : 16))
                return NO;

            return YES;
        }
    }

    return NO;
}

+ (NSNumber*)decodeNumericValueFromBuffer:(void *)data length:(NSUInteger)length type:(const char *)type
{
    if (type && data) {

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
                                return [NSNumber numberWithInteger:-encoded];
                            }

                            return [NSNumber numberWithUnsignedInteger:encoded];
                        }
                        break;

                    case '1':
                        if (type[3] == '6' && length == 2) {
                            UInt16 encoded = 0;

                            bcopy(data, &encoded, 2);

                            encoded = OSSwapBigToHostInt16(encoded);

                            if (signd && bit_get(encoded, BIT(15))) {
                                bit_clear(encoded, BIT(15));
                                return [NSNumber numberWithInteger:-encoded];
                            }

                            return [NSNumber numberWithUnsignedInteger:encoded];
                        }
                        break;

                    case '3':
                        if (type[3] == '2' && length == 4) {
                            UInt32 encoded = 0;

                            bcopy(data, &encoded, 4);

                            encoded = OSSwapBigToHostInt32(encoded);

                            if (signd && bit_get(encoded, BIT(31))) {
                                bit_clear(encoded, BIT(31));
                                return [NSNumber numberWithInteger:-encoded];
                            }

                            return [NSNumber numberWithUnsignedInteger:encoded];
                        }
                        break;
                }
            }
            else if ((type[0] == 'f' || type[0] == 's') && type[1] == 'p' && length == 2) {
                UInt16 encoded = 0;

                bcopy(data, &encoded, 2);

                UInt8 i = [SmcHelper getIndexFromHexChar:type[2]];
                UInt8 f = [SmcHelper getIndexFromHexChar:type[3]];

                if (i + f != (type[0] == 's' ? 15 : 16) )
                    return nil;
                
                UInt16 swapped = OSSwapBigToHostInt16(encoded);
                
                BOOL signd = type[0] == 's';
                BOOL minus = bit_get(swapped, BIT(15));
                
                if (signd && minus) bit_clear(swapped, BIT(15));
                
                return [NSNumber numberWithFloat:((float)swapped / (float)BIT(f)) * (signd && minus ? -1 : 1)];
            }
        }
    }
    
    return nil;
}

+ (BOOL)encodeNumericValue:(NSNumber*)value length:(NSUInteger)length type:(const char *)type outBuffer:(void*)outBuffer
{
    if (type && outBuffer) {
        
        size_t typeLength = strnlen(type, 4);
        
        if (typeLength >= 3) {
            if ((type[0] == 'u' || type[0] == 's') && type[1] == 'i') {

                int intValue = [value intValue];

                bool minus = intValue < 0;
                bool signd = type[0] == 's';
                
                if (minus) intValue = -intValue;
                
                switch (type[2]) {
                    case '8':
                        if (type[3] == '\0' && length == 1) {
                            UInt8 encoded = (UInt8)intValue;
                            if (signd) bit_write(signd && minus, encoded, BIT(7));
                            bcopy(&encoded, outBuffer, 1);
                            return YES;
                        }
                        break;
                        
                    case '1':
                        if (type[3] == '6' && length == 2) {
                            UInt16 encoded = (UInt16)intValue;
                            if (signd) bit_write(signd && minus, encoded, BIT(15));
                            OSWriteBigInt16(outBuffer, 0, encoded);
                            return YES;
                        }
                        break;
                        
                    case '3':
                        if (type[3] == '2' && length == 4) {
                            UInt32 encoded = (UInt32)intValue;
                            if (signd) bit_write(signd && minus, encoded, BIT(31));
                            OSWriteBigInt32(outBuffer, 0, encoded);
                            return YES;
                        }
                        break;
                }
            }
            else if ((type[0] == 'f' || type[0] == 's') && type[1] == 'p') {
                float floatValue = [value floatValue];
                bool minus = floatValue < 0;
                bool signd = type[0] == 's';
                UInt8 i = [SmcHelper getIndexFromHexChar:type[2]];
                UInt8 f = [SmcHelper getIndexFromHexChar:type[3]];
                
                if (i + f == (signd ? 15 : 16)) {
                    if (minus) floatValue = -floatValue;
                    UInt16 encoded = floatValue * (float)BIT(f);
                    if (signd) bit_write(minus, encoded, BIT(15));
                    OSWriteBigInt16(outBuffer, 0, encoded);
                    return YES;
                }
            }
        }
    }
    
    return NO;
}

+ (void)writeKey:(NSString*)key value:(NSNumber*)value connection:(io_connect_t)connection
{
    SMCVal_t info;

    if (kIOReturnSuccess == SMCReadKey(connection, [key cStringUsingEncoding:NSASCIIStringEncoding], &info)) {
        if ([SmcHelper encodeNumericValue:value length:info.dataSize type:info.dataType outBuffer:info.bytes]) {
            SMCWriteKeyUnsafe(connection, &info);
        }
    }
}

@end
