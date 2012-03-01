//
//  NSHardwareMonitorSensor.m
//  HWSensors
//
//  Created by Natan Zalkin on 23/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#import "NSHardwareMonitorSensor.h"

#include "FakeSMCDefinitions.h"

inline UInt8 get_index(char c)
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
};

@implementation NSHardwareMonitorSensor

@synthesize key;
@synthesize type;
@synthesize group;
@synthesize caption;
@synthesize value;
@synthesize disk;

@synthesize menuItem;
@synthesize favorite;

+ (int)getIndexOfHexChar:(char)c
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

+ (float)decodeSMCFloatOfType:(const char*)type fraction:(UInt16) encoded
{
    UInt8 i = 0, f = 0;

    encoded = OSSwapBigToHostInt16(encoded);
    
    if (type[0] == 's' || type[0] == 'f') {
        if (type[1] == 'p') {
            i = [NSHardwareMonitorSensor getIndexOfHexChar:type[2]];
            f = [NSHardwareMonitorSensor getIndexOfHexChar:type[3]];
            
            if (i + f != (type[0] == 's' ? 15 : 16) ) 
                return encoded;
        }
        else return encoded;
        
        float value = (float)encoded / (float)(0x1 << f);
        
        return value * (type[0] == 's' && encoded & 0x8000 ? -1 : 1);
    }
    
    return encoded;
}

+ (NSHardwareMonitorSensor*)sensor
{
    NSHardwareMonitorSensor *me = [[NSHardwareMonitorSensor alloc] init];
    
    if (me) return me;
    
    return nil;
}

- (float)decodeValue
{
    if (value != NULL) {
        if ([type characterAtIndex:0] == 'u' && [type characterAtIndex:1] == 'i') {
            switch ([type characterAtIndex:3]) {
                case '8':
                    if ([type characterAtIndex:4] == '\0' && [value length] == 1) {
                        UInt8 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 1);
                        
                        return encoded;
                    }
                    break;
                    
                case '1':
                    if ([type characterAtIndex:4] == '6' && [value length] == 2) {
                        UInt16 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 2);
                        
                        return OSSwapBigToHostInt16(encoded);
                    }
                    break;
                    
                case '3':
                    if ([type characterAtIndex:4] == '2' && [value length] == 4) {
                        UInt32 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 4);
                        
                        return OSSwapBigToHostInt32(encoded);
                    }
                    break;
            }
        }
        else if ([type characterAtIndex:0] == 's' && [type characterAtIndex:1] == 'i') {
            switch ([type characterAtIndex:3]) {
                case '8':
                    if ([type characterAtIndex:4] == '\0' && [value length] == 1) {
                        SInt8 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 1);
                        
                        return encoded;
                    }
                    break;
                    
                case '1':
                    if ([type characterAtIndex:4] == '6' && [type length] == 2) {
                        SInt16 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 2);
                        
                        return OSSwapBigToHostInt16(encoded);
                    }
                    break;
                    
                case '3':
                    if ([type characterAtIndex:4] == '2' && [value length] == 4) {
                        SInt32 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 4);
                        
                        return OSSwapBigToHostInt32(encoded);
                    }
                    break;
            }
        }
        else if (([type characterAtIndex:0] == 'f' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'p' && [value length] == 2) {
            UInt16 encoded = 0;
            
            bcopy([value bytes], &encoded, 2);
            
            return [NSHardwareMonitorSensor decodeSMCFloatOfType:[type cStringUsingEncoding:NSASCIIStringEncoding] fraction:encoded];
        }
    }
    
    return 0;
}

- (NSString*)formatValue
{
    if (value != NULL) {
        switch (group) {
            case kHWSMARTTemperatureGroup:
            {
                UInt16 t = 0;
                
                bcopy([value bytes], &t, 2);
                
                return [[NSString alloc] initWithFormat:@"%d°",t];
                
            }
                
            case kHWSMARTRemainingLifeGroup:
            {
                UInt64 life = 0;
                
                bcopy([value bytes], &life, [value length]);
                
                //return [[NSString alloc] initWithFormat:@"%d%C",100-life,0x0025];
                return [[NSString alloc] initWithFormat:@"(%d)",life];
                
            }
                
            case kHWTemperatureGroup:
                return [[NSString alloc] initWithFormat:@"%1.0f°",[self decodeValue]];
                
            case kHWVoltageGroup:
                return [[NSString alloc] initWithFormat:@"%1.2fV",[self decodeValue]];
                
            case kHWTachometerGroup:
                return [[NSString alloc] initWithFormat:@"%1.0frpm",[self decodeValue]];
                
            case kHWMultiplierGroup:
                return [[NSString alloc] initWithFormat:@"x%1.1f",[self decodeValue]];
        }
    }
    
    return [[NSString alloc] initWithString:@"-"];
}


@end
