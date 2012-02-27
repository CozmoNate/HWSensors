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

+ (float)decodeSMCFloatOfType:(const char*)type fraction:(UInt16) encoded;
{
    UInt8 i = 0, f = 0;
    
    encoded = ((encoded & 0xff00) >> 8) | ((encoded & 0xff) << 8);
    
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

- (NSString*)formatValue;
{
    if (value != NULL) {
        switch (group) {
            case kHWSMARTTemperatureGroup:
            {
                UInt16 t = 0;
                
                bcopy([value bytes], &t, 2);
                
                return [[NSString alloc] initWithFormat:@"%d°",t];
                
            } break;
                
            case kHWSMARTRemainingLifeGroup:
            {
                UInt64 life = 0;
                
                bcopy([value bytes], &life, [value length]);
                
                //return [[NSString alloc] initWithFormat:@"%d%C",100-life,0x0025];
                return [[NSString alloc] initWithFormat:@"(%d)",life];
                
            } break;
                
            case kHWTemperatureGroup:
            {
                UInt16 encoded = 0;
                
                bcopy([value bytes], &encoded, 2);
                
                return [[NSString alloc] initWithFormat:@"%1.0f°",[NSHardwareMonitorSensor decodeSMCFloatOfType:[type cStringUsingEncoding:NSASCIIStringEncoding] fraction:encoded]];
                
            } break;
                
            case kHWVoltageGroup:
            {
                UInt16 encoded = 0;
                
                bcopy([value bytes], &encoded, 2);
                
                return [[NSString alloc] initWithFormat:@"%1.2fV",[NSHardwareMonitorSensor decodeSMCFloatOfType:[type cStringUsingEncoding:NSASCIIStringEncoding] fraction:encoded]];
                
            } break;
                
            case kHWTachometerGroup:
            {
                UInt16 encoded = 0;
                
                bcopy([value bytes], &encoded, 2);
                    
                return [[NSString alloc] initWithFormat:@"%1.0frpm",[NSHardwareMonitorSensor decodeSMCFloatOfType:[type cStringUsingEncoding:NSASCIIStringEncoding] fraction:encoded]];
                
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
