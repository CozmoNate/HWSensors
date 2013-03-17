//
//  HWMonitorSensor.m
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */


#import "HWMonitorSensor.h"
#import "HWMonitorEngine.h"

#include "FakeSMCDefinitions.h"

@implementation HWMonitorSensor

@synthesize type = _type;
@synthesize data = _data;
@synthesize level = _level;
@synthesize formattedValue = _formattedValue;

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
            
            UInt8 i = [HWMonitorSensor getIndexOfHexChar:[_type characterAtIndex:2]];
            UInt8 f = [HWMonitorSensor getIndexOfHexChar:[_type characterAtIndex:3]];
            
            if (i + f != ([_type characterAtIndex:0] == 's' ? 15 : 16) )
                return 0;
            
            UInt16 swapped = OSSwapBigToHostInt16(encoded);
            
            BOOL signd = [_type characterAtIndex:0] == 's';
            BOOL minus = bit_get(swapped, BIT(15));
            
            if (signd && minus) bit_clear(swapped, BIT(15));
            
            return ((float)swapped / (float)BIT(f)) * (signd && minus ? -1 : 1);
        }
    }
    
    return MAXFLOAT;
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
    if (!_data || ![_data isEqualToData:newData]) {
        _valueHasBeenChanged = true;
        _data = newData;
    }
}

- (NSData *)data
{
    _valueHasBeenChanged = false;
    return _data;
}

- (void)setLevel:(HWSensorLevel)newLevel
{
    if (_level != newLevel) {
        _levelHasBeenChanged = true;
        _level = newLevel;
    }
}

- (HWSensorLevel)level
{
    _levelHasBeenChanged = false;
    return _level;
}

-(NSNumber *)rawValue
{
    if ((_valueHasBeenChanged || !_rawValue) && _data) {
        if (_group & kSMARTGroupTemperature) {
            UInt64 t = 0;
            
            [_data getBytes:&t length:[_data length]];
            
            if (_level != kHWSensorLevelExceeded) {
                if ([_genericDevice isRotational]) {
                    [self setLevel:t >= 55 ? kHWSensorLevelExceeded : t >= 50 ? kHWSensorLevelHigh : t >= 40 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
                }
                else {
                    [self setLevel:t >= 100 ? kHWSensorLevelExceeded : t >= 85 ? kHWSensorLevelHigh : t >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
                }
            }
            
            _rawValue = [NSNumber numberWithLongLong:t];
        }
        else if (_group & kSMARTGroupRemainingLife) {
            UInt64 life = 0;
            
            [_data getBytes:&life length:[_data length]];
            
            if (_level != kHWSensorLevelExceeded)
                [self setLevel:life >= 90 ? kHWSensorLevelExceeded : life >= 80 ? kHWSensorLevelHigh : life >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
            
            _rawValue = [NSNumber numberWithLongLong:100 - life];
        }
        else if (_group & kSMARTGroupRemainingBlocks) {
            UInt64 blocks = 0;
            
            [_data getBytes:&blocks length:[_data length]];
            
            _rawValue = [NSNumber numberWithLongLong:blocks];
        }
        else if (_group & kBluetoothGroupBattery) {
            UInt64 percent = 0;
            
            [_data getBytes:&percent length:[_data length]];
            
            if (_level != kHWSensorLevelExceeded)
                [self setLevel:percent < 5 ? kHWSensorLevelExceeded : percent < 10 ? kHWSensorLevelHigh : percent < 30 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
            
            _rawValue = [NSNumber numberWithLongLong:percent];
        }
        else if (_group & kHWSensorGroupTemperature) {
            _rawValue = [NSNumber numberWithDouble:[self decodeNumericValue]];
            
            [self setLevel:[_rawValue intValue] >= 100 ? kHWSensorLevelExceeded : [_rawValue intValue] >= 85 ? kHWSensorLevelHigh : [_rawValue intValue] >= 70 ? kHWSensorLevelModerate : kHWSensorLevelNormal];
        }
        else if (_group & kHWSensorGroupPWM) {
            _rawValue = [NSNumber numberWithFloat:[self decodeNumericValue]];
        }
        else if (_group & kHWSensorGroupMultiplier) {
            _rawValue = [NSNumber numberWithFloat:[self decodeNumericValue]];
        }
        else if (_group & kHWSensorGroupFrequency) {
            _rawValue = [NSNumber numberWithFloat:[self decodeNumericValue]];
        }
        else if (_group & kHWSensorGroupTachometer) {
            _rawValue = [NSNumber numberWithFloat:[self decodeNumericValue]];
            
            if ([_rawValue floatValue] >= 1) {
                if (_level != kHWSensorLevelNormal) {
                    [self setLevel: kHWSensorLevelNormal];
                }
            }
        }
        else if (_group & kHWSensorGroupVoltage) {
            _rawValue = [NSNumber numberWithFloat:[self decodeNumericValue]];
        }
        else if (_group & kHWSensorGroupCurrent) {
            _rawValue = [NSNumber numberWithFloat:[self decodeNumericValue]];
        }
        else if (_group & kHWSensorGroupPower) {
            _rawValue = [NSNumber numberWithFloat:[self decodeNumericValue]];
        }
        else {
            _rawValue = [NSNumber numberWithFloat:MAXFLOAT];
        }
    }
    
    _valueHasBeenChanged = false;
    
    return _rawValue;
}

- (NSString*)formattedValue
{
    if ((_valueHasBeenChanged || !_formattedValue) && _data) {
        if (_group & kSMARTGroupTemperature) {
            if ([_engine useFahrenheit]) {
                _formattedValue = [NSString stringWithFormat:@"%1.0f°", [[self rawValue] floatValue] * (9.0f / 5.0f) + 32.0f];
            }
            else {
                _formattedValue = [NSString stringWithFormat:@"%d°", [[self rawValue] intValue]];
            }
        }
        else if (_group & (kSMARTGroupRemainingLife | kBluetoothGroupBattery)) {
            _formattedValue = [NSString stringWithFormat:@"%d%%", [[self rawValue] intValue]];
        }
        else if (_group & kSMARTGroupRemainingBlocks) {
            _formattedValue = [NSString stringWithFormat:@"%lld", [[self rawValue] longLongValue]];
        }
        else if (_group & kHWSensorGroupTemperature) {
            if ([_engine useFahrenheit]) {
                _formattedValue = [NSString stringWithFormat:@"%1.0f°", [[self rawValue] floatValue] * (9.0f / 5.0f) + 32.0f];
            }
            else {
                _formattedValue = [NSString stringWithFormat:@"%d°", [[self rawValue] intValue]];
            }
        }
        else if (_group & kHWSensorGroupPWM) {
            if ([[self rawValue] floatValue] < 0 || [[self rawValue] floatValue] > 100) {
                _formattedValue = @"-";
            }
            else {
                _formattedValue = [[NSString alloc] initWithFormat:@"%1.0f%c", [[self rawValue] floatValue], 0x0025];
            }
        }
        else if (_group & kHWSensorGroupMultiplier) {
            _formattedValue = [NSString stringWithFormat:@"x%1.1f", [[self rawValue] floatValue]];
        }
        else if (_group & kHWSensorGroupFrequency) {
            if ([_rawValue floatValue] > 1e6)
                _formattedValue = [NSString stringWithFormat:@"%1.2fTHz", [[self rawValue] floatValue] / 1e6];
            else if ([_rawValue floatValue] > 1e3)
                _formattedValue = [NSString stringWithFormat:@"%1.2fGHz", [[self rawValue] floatValue] / 1e3];
            else 
                _formattedValue = [NSString stringWithFormat:@"%1.0fMHz", [[self rawValue] floatValue]];
        }
        else if (_group & kHWSensorGroupTachometer) {
            if ([[self rawValue] floatValue] < 1) {
                //rehabman: it is normal on a laptop to have a fan read 0 RPM...
                //[self setLevel:kHWSensorLevelExceeded];
                _formattedValue = @"-";
            }
            else {
                _formattedValue = [NSString stringWithFormat:@"%1.0frpm", [[self rawValue] floatValue]];
            }
        }
        else if (_group & kHWSensorGroupVoltage) {
            _formattedValue = [NSString stringWithFormat:@"%1.2fV", [[self rawValue] floatValue]];
        }
        else if (_group & kHWSensorGroupCurrent) {
            _formattedValue = [NSString stringWithFormat:@"%1.2fA", [[self rawValue] floatValue]];
        }
        else if (_group & kHWSensorGroupPower) {
            _formattedValue = [NSString stringWithFormat:@"%1.2fW", [[self rawValue] floatValue]];
        }
        else {
            _formattedValue = @"-";
        }
    }
    
    return _formattedValue;
}

@end
