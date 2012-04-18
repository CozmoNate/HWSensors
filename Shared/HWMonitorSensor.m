//
//  NSHardwareMonitorSensor.m
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#import "HWMonitorSensor.h"

#include "FakeSMCDefinitions.h"

inline UInt8 get_index(char c)
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
};

@implementation HWMonitorSensor

@synthesize key;
@synthesize type;
@synthesize group;
@synthesize caption;
@synthesize value;
@synthesize disk;

@synthesize favorite;
@synthesize level;
@synthesize levelHasBeenChanged;

@synthesize menuItem;

- (NSUInteger)level
{
    levelHasBeenChanged = false;
    
    return level;
}

- (void)setLevel:(NSUInteger)newLevel
{
    levelHasBeenChanged = level != newLevel;
    
    level = newLevel;
}

+ (int)getIndexOfHexChar:(char)c
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

+ (HWMonitorSensor*)sensor
{
    HWMonitorSensor *me = [[HWMonitorSensor alloc] init];
    
    if (me) return me;
    
    return nil;
}

- (float)decodedValue
{
    if (value != NULL) {
        if (([type characterAtIndex:0] == 'u' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'i') {
            
            BOOL signd = [type characterAtIndex:0] == 's';
            
            switch ([type characterAtIndex:2]) {
                case '8':
                    if ([type characterAtIndex:3] == '\0' && [value length] == 1) {
                        UInt8 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 1);
                        
                        return (signd && encoded & 0x80 ? -encoded : encoded);
                    }
                    break;
                    
                case '1':
                    if ([type characterAtIndex:3] == '6' && [value length] == 2) {
                        UInt16 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 2);
                        
                        encoded = OSSwapBigToHostInt16(encoded);
                        
                        return (signd && encoded & 0x8000 ? -encoded : encoded);
                    }
                    break;
                    
                case '3':
                    if ([type characterAtIndex:3] == '2' && [value length] == 4) {
                        UInt32 encoded = 0;
                        
                        bcopy([value bytes], &encoded, 4);
                        
                        encoded = OSSwapBigToHostInt32(encoded);
                        
                        return (signd && encoded & 0x80000000 ? -encoded : encoded);
                    }
                    break;
            }
        }
        else if (([type characterAtIndex:0] == 'f' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'p' && [value length] == 2) {
            UInt16 encoded = 0;
            
            bcopy([value bytes], &encoded, 2);
            
            UInt8 i = [HWMonitorSensor getIndexOfHexChar:[type characterAtIndex:2]];
            UInt8 f = [HWMonitorSensor getIndexOfHexChar:[type characterAtIndex:3]];
            
            if (i + f != ([type characterAtIndex:0] == 's' ? 15 : 16) ) 
                return 0;
            
            UInt16 swapped = OSSwapBigToHostInt16(encoded);
            
            BOOL minus = swapped & 0x8000;
            
            if (minus) 
                swapped = swapped & 0x7fff;
            
            return ((float)swapped / (float)(0x1 << f)) * ([type characterAtIndex:0] == 's' && minus ? -1 : 1);
        }
    }
    
    return 0;
}

- (NSString*)formattedValue
{
    if (value != NULL) {
        switch (group) {
            case kSMARTSensorGroupTemperature: {
                UInt16 t = 0;
                
                [value getBytes:&t length:2];
                               
                if (level != kHWSensorLevelExceeded && [disk isRotational])
                    [self setLevel:t >= 55 ? kHWSensorLevelExceeded : t >= 50 ? kHWSensorLevelHigh : t >= 40 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
                
                return [[NSString alloc] initWithFormat:@"%d°",t];
                
            }
                
            case kSMARTSensorGroupRemainingLife: {
                UInt64 life = 0;
                
                [value getBytes:&life length:[value length]];
                
                if (level != kHWSensorLevelExceeded)
                    [self setLevel:life >= 90 ? kHWSensorLevelExceeded : life >= 80 ? kHWSensorLevelHigh : life >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
                
                return [[NSString alloc] initWithFormat:@"%d%C",100-life,0x0025];
            }
                
            case kSMARTSensorGroupRemainingBlocks: {
                UInt64 blocks = 0;
                
                [value getBytes:&blocks length:[value length]];
                 
                return [[NSString alloc] initWithFormat:@"%d",blocks];
            }
                
            case kHWSensorGroupTemperature: {
                float t = [self decodedValue];
                
                [self setLevel:t >= 100 ? kHWSensorLevelExceeded : t >= 85 ? kHWSensorLevelHigh : t >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
                
                return [[NSString alloc] initWithFormat:@"%1.0f°", t];
            }
                
            case kHWSensorGroupVoltage:
                return [[NSString alloc] initWithFormat:@"%1.3fV", [self decodedValue]];
                
            case kHWSensorGroupTachometer: {
                
                float rpm = [self decodedValue];
                
                [self setLevel:rpm == 0 ? kHWSensorLevelExceeded : kHWSensorLevelNormal];
                
                return [[NSString alloc] initWithFormat:@"%1.0frpm", rpm];
            }
                
            case kHWSensorGroupMultiplier:
                return [[NSString alloc] initWithFormat:@"x%1.1f", [self decodedValue]];
                
            case kHWSensorGroupFrequency: {
                float f = [self decodedValue];
                
                if (f > 1e6)
                    return [[NSString alloc] initWithFormat:@"%1.2fTHz", f / 1e6];
                else if (f > 1e3)
                    return [[NSString alloc] initWithFormat:@"%1.2fGHz", f / 1e3];
                else 
                    return [[NSString alloc] initWithFormat:@"%1.1fMHz", f]; 
            }
        }
    }
    
    return [[NSString alloc] initWithString:@"-"];
}


@end
