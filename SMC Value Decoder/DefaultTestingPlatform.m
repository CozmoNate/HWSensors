//
//  SMC_Value_Decoder.m
//  SMC Value Decoder
//
//  Created by kozlek on 09.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "DefaultTestingPlatform.h"

#import "FakeSMCDefinitions.h"
#import "SmcKeysDefinitions.h"

@implementation DefaultTestingPlatform

- (void)setUp
{
    [super setUp];
    
    // Set-up code here.
    
    _name = @"VN0R";
    _type = @"sp3c";
    
    UInt16 data = OSSwapHostToBigInt16(0x0854); // Big Endian
    
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

- (void)testValueDecode
{
    NSLog(@"Decoded Key \"%@\" Value Is %f", _name, [self decodeNumericValue]);
    //STFail(@"Unit tests are not implemented yet in SMC Value Decoder");
}

- (void)testGetMacModel
{
    CFDictionaryRef matching = IOServiceMatching("IOPlatformExpertDevice");

    if (MACH_PORT_NULL != matching) {
        io_iterator_t iterator = IO_OBJECT_NULL;

        if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
            if (IO_OBJECT_NULL != iterator) {

                io_service_t service = MACH_PORT_NULL;

                while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                    NSString *model = [[NSString alloc] initWithData:(__bridge_transfer NSData *)IORegistryEntryCreateCFProperty(service, CFSTR("model"), kCFAllocatorDefault, 0) encoding:NSASCIIStringEncoding];

                    NSLog(@"Mac model is %@", model);
                }

                IOObjectRelease(iterator);
            }
        }
    }
}

- (NSIndexSet*)getKeyInfosInGroup:(SmcKeyGroup)group
{
    int count = sizeof(SMCKeyInfoList) / sizeof(SMCKeyInfo);
    
    NSMutableIndexSet *indexSet = [[NSMutableIndexSet alloc] init];
    
    for (int index = 0; index < count; index++) {
        if (SMCKeyInfoList[index].group == group) {
            [indexSet addIndex:index];
        }
    }
    
    return indexSet;
}

- (NSArray*)addSensorsFromSMCKeyGroup:(SmcKeyGroup)fromGroup
{
    NSMutableArray *list = [NSMutableArray new];
    
    [[self getKeyInfosInGroup:fromGroup] enumerateIndexesWithOptions:NSSortStable usingBlock:^(NSUInteger idx, BOOL *stop) {
        if (SMCKeyInfoList[idx].count) {
            NSString *keyFormat = [NSString stringWithCString:SMCKeyInfoList[idx].key encoding:NSASCIIStringEncoding];
            NSString *titleFormat = [NSString stringWithCString:SMCKeyInfoList[idx].title encoding:NSASCIIStringEncoding];
            
            for (NSUInteger index = 0; index < SMCKeyInfoList[idx].count; index++) {
                [list addObject:[NSDictionary dictionaryWithObjectsAndKeys:
                                 [NSString stringWithFormat:keyFormat, index + SMCKeyInfoList[idx].offset], @"key",
                                 [NSString stringWithFormat:titleFormat, index + SMCKeyInfoList[idx].shift], @"title",
                                 nil]];
            }
        }
        else {
            NSString *key = [NSString stringWithCString:SMCKeyInfoList[idx].key encoding:NSASCIIStringEncoding];
            NSString *title = [NSString stringWithCString:SMCKeyInfoList[idx].title encoding:NSASCIIStringEncoding];
            
            [list addObject:[NSDictionary dictionaryWithObjectsAndKeys:
                             key, @"key",
                             title, @"title",
                             nil]];
        }
    }];
    
    return list;
}

- (void)testExportSmcKeys
{
    NSMutableDictionary *list = [NSMutableDictionary new];
    
    [list setObject:[self addSensorsFromSMCKeyGroup:kSMCKeyGroupTemperature] forKey:@"Temperatures"];
    [list setObject:[self addSensorsFromSMCKeyGroup:kSMCKeyGroupMultiplier] forKey:@"Multiplier"];
    [list setObject:[self addSensorsFromSMCKeyGroup:kSMCKeyGroupFrequency] forKey:@"Frequency"];
    [list setObject:[self addSensorsFromSMCKeyGroup:kSMCKeyGroupVoltage] forKey:@"Voltage"];
    [list setObject:[self addSensorsFromSMCKeyGroup:kSMCKeyGroupCurrent] forKey:@"Current"];
    [list setObject:[self addSensorsFromSMCKeyGroup:kSMCKeyGroupPower] forKey:@"Power"];
    
    [list writeToFile:@"/Users/kozlek/Documents/HWSensors/SMCKeyS.plist" atomically:YES];
    
}

@end
