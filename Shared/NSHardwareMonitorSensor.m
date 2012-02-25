//
//  NSHardwareMonitorSensor.m
//  HWSensors
//
//  Created by Natan Zalkin on 23/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#import "NSHardwareMonitorSensor.h"

#include "FakeSMCDefinitions.h"

UInt16 swap_value(UInt16 value)
{
	return ((value & 0xff00) >> 8) | ((value & 0xff) << 8);
};

UInt8 get_index(char c)
{
	return c >= 'a' ? c - 87 : c >= 'A' ? c - 55 : c - 48;
};

float decode_float(const char * type, UInt16 encoded)
{
    UInt8 i = 0, f = 0;
    
    encoded = swap_value(encoded);
    
    if (type[0] == 's' || type[0] == 'f') {
        if (type[1] == 'p') {
            i = get_index(type[2]);
            f = get_index(type[3]);
            
            if (i + f != (type[0] == 's' ? 15 : 16) ) 
                return encoded;
        }
        else return encoded;
        
        float value = (float)encoded / (float)(0x1 << f);
        
        return value;
    }
    
    return encoded;
}

@implementation NSHardwareMonitorSensor

@synthesize key;
@synthesize group;
@synthesize caption;
@synthesize value;
@synthesize disk;

@synthesize menuItem;
@synthesize favorite;

+ (NSHardwareMonitorSensor*)sensor
{
    NSHardwareMonitorSensor *me = [[NSHardwareMonitorSensor alloc] init];
    
    if (me) return me;
    
    return nil;
}

- (NSString*)formatValue;
{
    if (value != NULL) {
        switch (group) {
            case kHWTemperatureGroup:
            case kHWSMARTTemperatureGroup:
            {
                UInt16 encoded = 0;
                
                bcopy([value bytes], &encoded, 2);
                
                return [[NSString alloc] initWithFormat:@"%1.0f°",decode_float(TYPE_SP78, encoded)];
                
            } break;
                
            case kHWSMARTRemainingLifeGroup:
            {
                UInt64 life = 0;
                
                bcopy([value bytes], &life, [value length]);
                
                //return [[NSString alloc] initWithFormat:@"%d%C",100-life,0x0025];
                return [[NSString alloc] initWithFormat:@"(%d)",life];
                
            } break;
                
            case kHWVoltageGroup:
            {
                UInt16 encoded = 0;
                
                bcopy([value bytes], &encoded, 2);
                
                return [[NSString alloc] initWithFormat:@"%1.2fV",decode_float(TYPE_FP2E, encoded)];
                
            } break;
                
            case kHWTachometerGroup:
            {
                UInt16 encoded = 0;
                
                bcopy([value bytes], &encoded, 2);
                    
                return [[NSString alloc] initWithFormat:@"%1.0frpm",decode_float(TYPE_FPE2, encoded)];
                
            } break;
                
            case kHWMultiplierGroup:
            {
                UInt16 mlt = 0;
                
                bcopy([value bytes], &mlt, 2);
                
                return [[NSString alloc] initWithFormat:@"x%1.1f",(float)mlt / 10.0f];
                
            } break;
        }
    }
    
    return [[NSString alloc] initWithString:@"-"];
}


@end
