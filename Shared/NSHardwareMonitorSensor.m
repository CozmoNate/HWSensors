//
//  NSHardwareMonitorSensor.m
//  HWSensors
//
//  Created by Natan Zalkin on 23/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#import "NSHardwareMonitorSensor.h"

@implementation NSHardwareMonitorSensor

@synthesize key;
@synthesize group;
@synthesize caption;
@synthesize value;
@synthesize disk;

@synthesize menuItem;
@synthesize favorite;


+ (unsigned int)swapBytes:(unsigned int)value
{
    return ((value & 0xff00) >> 8) | ((value & 0xff) << 8);
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
            case kHWTemperatureGroup:
            case kHWSMARTTemperatureGroup:
            {
                unsigned int t = 0;
                
                bcopy([value bytes], &t, 2);
                
                return [[NSString alloc] initWithFormat:@"%d°",t];
                
            } break;
                
            case kHWSMARTRemainingLifeGroup:
            {
                unsigned int life = 0;
                
                bcopy([value bytes], &life, 2);
                
                return [[NSString alloc] initWithFormat:@"%d%C",100-life,0x0025];
                
            } break;
                
            case kHWVoltageGroup:
            {
                unsigned int encoded = 0;
                
                bcopy([value bytes], &encoded, 2);
                
                encoded = [NSHardwareMonitorSensor swapBytes:encoded];
                
                float v = ((encoded & 0xc000) >> 14) + ((encoded & 0x3fff) >> 4) / 1000.0;
                
                return [[NSString alloc] initWithFormat:@"%1.3fV",v];
            } break;
                
            case kHWTachometerGroup:
            {
                unsigned int rpm = 0;
                
                bcopy([value bytes], &rpm, 2);
                
                rpm = [NSHardwareMonitorSensor swapBytes:rpm] >> 2;
                
                return [[NSString alloc] initWithFormat:@"%drpm",rpm];
                
            } break;
                
            case kHWMultiplierGroup:
            {
                unsigned int mlt = 0;
                
                bcopy([value bytes], &mlt, 2);
                
                return [[NSString alloc] initWithFormat:@"x%1.1f",(float)mlt / 10.0];
                
            } break;
        }
    }
    
    return [[NSString alloc] initWithString:@"-"];
}


@end
