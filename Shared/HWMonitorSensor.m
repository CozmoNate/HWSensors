//
//  HWMonitorSensor.m
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorSensor.h"
#import "HWMonitorEngine.h"

#include "FakeSMCDefinitions.h"

inline UInt8 get_index(char c)
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
};

@implementation HWMonitorSensor

@synthesize engine=_engine;
@synthesize name=_name;
@synthesize type=_type;
@synthesize group=_group;
@synthesize caption=_caption;
@synthesize title=_title;
@synthesize data=_data;
@synthesize disk=_disk;

@synthesize level=_level;
@synthesize value=_value;

@synthesize favorite=_favorite;

@synthesize valueHasBeenChanged=_valueHasBeenChanged;
@synthesize levelHasBeenChanged=_levelHasBeenChanged;

@synthesize menuItem=_menuItem;

+ (int)getIndexOfHexChar:(char)c
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

+ (HWMonitorSensor*)sensor
{
    HWMonitorSensor *me = [[HWMonitorSensor alloc] init];
    
    if (me) {
        me->_levelHasBeenChanged = true;
        me->_valueHasBeenChanged = true;
        
        return me;
    }
    
    return nil;
}

- (void)setType:(NSString *)newType
{
    if (![_type isEqualToString:newType]) { 
        _valueHasBeenChanged = true;
        _type = newType;
    }
}

- (NSString *)type
{
    _valueHasBeenChanged = false;
    return _type;
}

- (void)setData:(NSData *)newData
{
    if (![_data isEqualToData:newData]) {
        _valueHasBeenChanged = true;
        _data = newData;
    }
}

- (NSData *)data
{
    _valueHasBeenChanged = false;
    return _data;
}

- (void)setLevel:(NSUInteger)newLevel
{
    if (_level != newLevel) {
        _levelHasBeenChanged = true;
        _level = newLevel;
    }
}

- (NSUInteger)level
{
    _levelHasBeenChanged = false;
    return _level;
}

- (float)decodeValue
{
    if (_type && _data) {
        if (([_type characterAtIndex:0] == 'u' || [_type characterAtIndex:0] == 's') && [_type characterAtIndex:1] == 'i') {
            
            BOOL signd = [_type characterAtIndex:0] == 's';
            
            switch ([_type characterAtIndex:2]) {
                case '8':
                    if ([_data length] == 1) {
                        UInt8 encoded = 0;
                        
                        bcopy([_data bytes], &encoded, 1);
                        
                        return (signd && encoded & 0x80 ? -encoded : encoded);
                    }
                    break;
                    
                case '1':
                    if ([_type characterAtIndex:3] == '6' && [_data length] == 2) {
                        UInt16 encoded = 0;
                        
                        bcopy([_data bytes], &encoded, 2);
                        
                        encoded = OSSwapBigToHostInt16(encoded);
                        
                        return (signd && encoded & 0x8000 ? -encoded : encoded);
                    }
                    break;
                    
                case '3':
                    if ([_type characterAtIndex:3] == '2' && [_data length] == 4) {
                        UInt32 encoded = 0;
                        
                        bcopy([_data bytes], &encoded, 4);
                        
                        encoded = OSSwapBigToHostInt32(encoded);
                        
                        return (signd && encoded & 0x80000000 ? -encoded : encoded);
                    }
                    break;
            }
        }
        else if (([_type characterAtIndex:0] == 'f' || [_type characterAtIndex:0] == 's') && [_type characterAtIndex:1] == 'p' && [_data length] == 2) {
            UInt16 encoded = 0;
            
            bcopy([_data bytes], &encoded, 2);
            
            UInt8 i = [HWMonitorSensor getIndexOfHexChar:[_type characterAtIndex:2]];
            UInt8 f = [HWMonitorSensor getIndexOfHexChar:[_type characterAtIndex:3]];
            
            if (i + f != ([_type characterAtIndex:0] == 's' ? 15 : 16) ) 
                return 0;
            
            UInt16 swapped = OSSwapBigToHostInt16(encoded);
            
            BOOL minus = swapped & 0x8000;
            
            if (minus) 
                swapped = swapped & 0x7fff;
            
            return ((float)swapped / (float)(0x1 << f)) * ([_type characterAtIndex:0] == 's' && minus ? -1 : 1);
        }
    }
    
    return 0;
}

- (NSString*)value
{
    if (_valueHasBeenChanged && _data) {
        if (_group & kSMARTSensorGroupTemperature) {
            UInt16 t = 0;
            
            [_data getBytes:&t length:2];
            
            if (_level != kHWSensorLevelExceeded && [_disk isRotational])
                [self setLevel:t >= 55 ? kHWSensorLevelExceeded : t >= 50 ? kHWSensorLevelHigh : t >= 40 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
            
            if ([_engine useFahrenheit]) 
                _value = [[NSString alloc] initWithFormat:@"%1.0f°", t * (9.0f / 5.0f) + 32.0f];
            else 
                _value = [[NSString alloc] initWithFormat:@"%d°", t];
        }
        else if (_group & kSMARTSensorGroupRemainingLife) {
            UInt64 life = 0;
            
            [_data getBytes:&life length:[_data length]];
            
            if (_level != kHWSensorLevelExceeded)
                [self setLevel:life >= 90 ? kHWSensorLevelExceeded : life >= 80 ? kHWSensorLevelHigh : life >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
            
            _value = [[NSString alloc] initWithFormat:@"%d%C",100-life,0x0025];
        }
        else if (_group & kSMARTSensorGroupRemainingBlocks) {
            UInt64 blocks = 0;
            
            [_data getBytes:&blocks length:[_data length]];
            
            _value = [[NSString alloc] initWithFormat:@"%d",blocks];
        }
        else if (_group & kHWSensorGroupTemperature) {
            float t = [self decodeValue];
            
            [self setLevel:t >= 100 ? kHWSensorLevelExceeded : t >= 85 ? kHWSensorLevelHigh : t >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
            
            if ([_engine useFahrenheit]) 
                _value = [[NSString alloc] initWithFormat:@"%1.0f°", t * (9.0f / 5.0f) + 32.0f];
            else 
                _value = [[NSString alloc] initWithFormat:@"%1.0f°", t];
        }
        else if (_group & kHWSensorGroupPWM) {
            _value = [[NSString alloc] initWithFormat:@"%1.0f%C", [self decodeValue], 0x0025];
        }
        else if (_group & kHWSensorGroupMultiplier) {
            _value = [[NSString alloc] initWithFormat:@"%1.1fx", [self decodeValue]];
        }
        else if (_group & kHWSensorGroupFrequency) {
            float f = [self decodeValue];
            
            if (f > 1e6)
                _value = [[NSString alloc] initWithFormat:@"%1.2fTHz", f / 1e6];
            else if (f > 1e3)
                _value = [[NSString alloc] initWithFormat:@"%1.2fGHz", f / 1e3];
            else 
                _value = [[NSString alloc] initWithFormat:@"%1.0fMHz", f]; 
        }
        else if (_group & kHWSensorGroupTachometer) {
            float rpm = [self decodeValue];
            
            if (rpm == 0) {
                [self setLevel:kHWSensorLevelExceeded];
                _value = [[NSString alloc] initWithString:@"-"];
            }
            else {
                if (_level != kHWSensorLevelNormal)
                    [self setLevel: kHWSensorLevelNormal];
                
                _value = [[NSString alloc] initWithFormat:@"%1.0frpm", rpm];
            }
        }
        else if (_group & kHWSensorGroupVoltage) {
            _value = [[NSString alloc] initWithFormat:@"%1.3fV", [self decodeValue]];
        }
        else _value = [[NSString alloc] initWithString:@"-"];
    }
    
    return _value;
}

@end
