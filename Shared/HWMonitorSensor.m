//
//  HWMonitorSensor.m
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#import "HWMonitorSensor.h"
#import "HWMonitorEngine.h"

#include "FakeSMCDefinitions.h"

inline UInt8 get_index(char c)
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
};

@implementation HWMonitorSensor

@synthesize engine = _engine;
@synthesize name = _name;
@synthesize type = _type;
@synthesize group = _group;
@synthesize title = _title;
@synthesize data = _data;
@synthesize disk = _disk;

@synthesize level = _level;
@synthesize rawValue = _rawValue;
@synthesize formattedValue = _formattedValue;

@synthesize valueHasBeenChanged = _valueHasBeenChanged;
@synthesize levelHasBeenChanged = _levelHasBeenChanged;

@synthesize representedObject = _representedObject;

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

- (void)setLevel:(enum HWSensorLevel)newLevel
{
    if (_level != newLevel) {
        _levelHasBeenChanged = true;
        _level = newLevel;
    }
}

- (enum HWSensorLevel)level
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

- (NSString*)formattedValue
{
    if (_valueHasBeenChanged && _data) {
        if (_group & kSMARTSensorGroupTemperature) {
            UInt16 t = 0;
            
            [_data getBytes:&t length:2];
            
            if (_level != kHWSensorLevelExceeded) {
                if ([_disk isRotational]) {
                    [self setLevel:t >= 55 ? kHWSensorLevelExceeded : t >= 50 ? kHWSensorLevelHigh : t >= 40 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
                }
                else {
                    [self setLevel:t >= 100 ? kHWSensorLevelExceeded : t >= 85 ? kHWSensorLevelHigh : t >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
                }
            }
            
            if ([_engine useFahrenheit]) {
                _rawValue = [NSNumber numberWithDouble:t];
                _formattedValue = [NSString stringWithFormat:@"%1.0f°", t * (9.0f / 5.0f) + 32.0f];
            }
            else {
                _rawValue = [NSNumber numberWithDouble:t];
                _formattedValue = [NSString stringWithFormat:@"%d°", t];
            }
        }
        else if (_group & kSMARTSensorGroupRemainingLife) {
            UInt64 life = 0;
            
            [_data getBytes:&life length:[_data length]];
            
            if (_level != kHWSensorLevelExceeded)
                [self setLevel:life >= 90 ? kHWSensorLevelExceeded : life >= 80 ? kHWSensorLevelHigh : life >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
            
            _rawValue = [NSNumber numberWithLongLong:100-life];
            _formattedValue = [NSString stringWithFormat:@"%ld%c",[_rawValue integerValue],0x0025];
        }
        else if (_group & kSMARTSensorGroupRemainingBlocks) {
            UInt64 blocks = 0;
            
            [_data getBytes:&blocks length:[_data length]];
            
            _rawValue = [NSNumber numberWithLongLong:blocks];
            _formattedValue = [NSString stringWithFormat:@"%lld",blocks];
        }
        else if (_group & kHWSensorGroupTemperature) {
            float t = [self decodeValue];
            
            [self setLevel:t >= 100 ? kHWSensorLevelExceeded : t >= 85 ? kHWSensorLevelHigh : t >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
            
            if ([_engine useFahrenheit]) {
                _rawValue = [NSNumber numberWithDouble:t];
                _formattedValue = [NSString stringWithFormat:@"%1.0f°", t * (9.0f / 5.0f) + 32.0f];
            }
            else {
                _rawValue = [NSNumber numberWithDouble:t];
                _formattedValue = [NSString stringWithFormat:@"%1.0f°", t];
            }
        }
        else if (_group & kHWSensorGroupPWM) {
            _rawValue = [NSNumber numberWithFloat:[self decodeValue]];
            _formattedValue = [[NSString alloc] initWithFormat:@"%1.0f%c", [_rawValue floatValue], 0x0025];
        }
        else if (_group & kHWSensorGroupMultiplier) {
            _rawValue = [NSNumber numberWithFloat:[self decodeValue]];
            //_formattedValue = [NSString stringWithFormat:[_rawValue floatValue] < 10 ? @"x  %1.1f" : @"x%1.1f", [_rawValue floatValue]];
            _formattedValue = [NSString stringWithFormat:@"x%1.1f", [_rawValue floatValue]];
        }
        else if (_group & kHWSensorGroupFrequency) {
            float f = [self decodeValue];
            
            _rawValue = [NSNumber numberWithFloat:f];
            
            if ([_rawValue floatValue] > 1e6)
                _formattedValue = [NSString stringWithFormat:@"%1.2fTHz", f / 1e6];
            else if ([_rawValue floatValue] > 1e3)
                _formattedValue = [NSString stringWithFormat:@"%1.2fGHz", f / 1e3];
            else 
                _formattedValue = [NSString stringWithFormat:@"%1.0fMHz", f];
        }
        else if (_group & kHWSensorGroupTachometer) {
            _rawValue = [NSNumber numberWithFloat:[self decodeValue]];
            
            if ([_rawValue floatValue] == 0) {
                [self setLevel:kHWSensorLevelExceeded];
                _formattedValue = [NSString stringWithFormat:@"-"];
            }
            else {
                if (_level != kHWSensorLevelNormal)
                    [self setLevel: kHWSensorLevelNormal];
                
                _formattedValue = [NSString stringWithFormat:@"%1.0frpm", [_rawValue floatValue]];
            }
        }
        else if (_group & kHWSensorGroupVoltage) {
            _rawValue = [NSNumber numberWithFloat:[self decodeValue]];
            _formattedValue = [NSString stringWithFormat:@"%1.3fV", [_rawValue floatValue]];
        }
        else {
            _rawValue = [NSNumber numberWithInt:0];
            _formattedValue = [NSString stringWithFormat:@"-"];
        }
    }
    else if (!_formattedValue) {
        _formattedValue = [NSString stringWithFormat:@"-"];
    }
    
    return _formattedValue;
}

@end
