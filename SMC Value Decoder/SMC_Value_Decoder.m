//
//  SMC_Value_Decoder.m
//  SMC Value Decoder
//
//  Created by kozlek on 09.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "SMC_Value_Decoder.h"

#import "FakeSMCDefinitions.h"

@implementation SMC_Value_Decoder

- (void)setUp
{
    [super setUp];
    
    // Set-up code here.
    
    _name = @"IC0C";
    _type = @"sp78";
    
    UInt16 data = OSSwapHostToBigInt16(0x09eb); // Big Endian
    
    _data = [NSData dataWithBytes:&data length:2];
}

- (void)tearDown
{
    // Tear-down code here.
    
    [super tearDown];
}

- (int)getIndexOfHexChar:(char)c
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

- (float)decodeNumericValue
{
    if (_type && _data && [_type length] >= 3) {
        if (([_type characterAtIndex:0] == 'u' || [_type characterAtIndex:0] == 's') && [_type characterAtIndex:1] == 'i') {
            
            BOOL signd = [_type characterAtIndex:0] == 's';
            
            switch ([_type characterAtIndex:2]) {
                case '8':
                    if ([_data length] == 1) {
                        UInt8 encoded = 0;
                        
                        bcopy([_data bytes], &encoded, 1);
                        
                        if (signd && bit_get(encoded, BIT(7))) {
                            bit_clear(encoded, BIT(7));
                            return -encoded;
                        }
                        
                        return encoded;
                    }
                    break;
                    
                case '1':
                    if ([_type characterAtIndex:3] == '6' && [_data length] == 2) {
                        UInt16 encoded = 0;
                        
                        bcopy([_data bytes], &encoded, 2);
                        
                        encoded = OSSwapBigToHostInt16(encoded);
                        
                        if (signd && bit_get(encoded, BIT(15))) {
                            bit_clear(encoded, BIT(15));
                            return -encoded;
                        }
                        
                        return encoded;
                    }
                    break;
                    
                case '3':
                    if ([_type characterAtIndex:3] == '2' && [_data length] == 4) {
                        UInt32 encoded = 0;
                        
                        bcopy([_data bytes], &encoded, 4);
                        
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
        else if (([_type characterAtIndex:0] == 'f' || [_type characterAtIndex:0] == 's') && [_type characterAtIndex:1] == 'p' && [_data length] == 2) {
            UInt16 encoded = 0;
            
            bcopy([_data bytes], &encoded, 2);
            
            UInt8 i = [self getIndexOfHexChar:[_type characterAtIndex:2]];
            UInt8 f = [self getIndexOfHexChar:[_type characterAtIndex:3]];
            
            if (i + f != ([_type characterAtIndex:0] == 's' ? 15 : 16) )
                return 0;
            
            UInt16 swapped = OSSwapBigToHostInt16(encoded);
            
            BOOL signd = [_type characterAtIndex:0] == 's';
            BOOL minus = bit_get(swapped, BIT(15));
            
            if (signd && minus) bit_clear(swapped, BIT(15));
            
            return ((float)swapped / (float)BIT(f)) * (signd && minus ? -1 : 1);
        }
    }
    
    return 0;
}

- (void)testDecode
{
    NSLog(@"Decoded Key \"%@\" Value Is %f", _name, [self decodeNumericValue]);
    //STFail(@"Unit tests are not implemented yet in SMC Value Decoder");
}

@end
