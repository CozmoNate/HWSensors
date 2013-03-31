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
@synthesize stringValue = _formattedValue;

+ (int)getIndexOfHexChar:(char)c
{
	return c > 96 && c < 103 ? c - 87 : c > 47 && c < 58 ? c - 48 : 0;
}

+ (BOOL)isValidIntegetType:(NSString *)type
{
    if (type && [type length] >= 3) {
        if (([type characterAtIndex:0] == 'u' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'i') {
            
            switch ([type characterAtIndex:2]) {
                case '8':
                    return YES;
                case '1':
                    return [type characterAtIndex:3] == '6' ? YES : NO;
                case '3':
                    return [type characterAtIndex:3] == '2'? YES : NO;
            }
        }
    }
    
    return NO;
}

+ (BOOL)isValidFloatingType:(NSString *)type
{
    if (type && [type length] >= 3) {
        if (([type characterAtIndex:0] == 'f' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'p') {
            UInt8 i = [HWMonitorSensor getIndexOfHexChar:[type characterAtIndex:2]];
            UInt8 f = [HWMonitorSensor getIndexOfHexChar:[type characterAtIndex:3]];
            
            if (i + f != ([type characterAtIndex:0] == 's' ? 15 : 16))
                return NO;
            
            return YES;
        }
    }
    
    return NO;
}

+ (float)decodeNumericData:(NSData*)data ofType:(NSString*)type
{
    if (type && data && [type length] >= 3) {
        if (([type characterAtIndex:0] == 'u' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'i') {
            
            BOOL signd = [type characterAtIndex:0] == 's';
            
            switch ([type characterAtIndex:2]) {
                case '8':
                    if ([data length] == 1) {
                        UInt8 encoded = 0;
                        
                        bcopy([data bytes], &encoded, 1);
                        
                        if (signd && bit_get(encoded, BIT(7))) {
                            bit_clear(encoded, BIT(7));
                            return -encoded;
                        }
                        
                        return encoded;
                    }
                    break;
                    
                case '1':
                    if ([type characterAtIndex:3] == '6' && [data length] == 2) {
                        UInt16 encoded = 0;
                        
                        bcopy([data bytes], &encoded, 2);
                        
                        encoded = OSSwapBigToHostInt16(encoded);
                        
                        if (signd && bit_get(encoded, BIT(15))) {
                            bit_clear(encoded, BIT(15));
                            return -encoded;
                        }
                        
                        return encoded;
                    }
                    break;
                    
                case '3':
                    if ([type characterAtIndex:3] == '2' && [data length] == 4) {
                        UInt32 encoded = 0;
                        
                        bcopy([data bytes], &encoded, 4);
                        
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
        else if (([type characterAtIndex:0] == 'f' || [type characterAtIndex:0] == 's') && [type characterAtIndex:1] == 'p' && [data length] == 2) {
            UInt16 encoded = 0;
            
            bcopy([data bytes], &encoded, 2);
            
            UInt8 i = [HWMonitorSensor getIndexOfHexChar:[type characterAtIndex:2]];
            UInt8 f = [HWMonitorSensor getIndexOfHexChar:[type characterAtIndex:3]];
            
            if (i + f != ([type characterAtIndex:0] == 's' ? 15 : 16) )
                return MAXFLOAT;
            
            UInt16 swapped = OSSwapBigToHostInt16(encoded);
            
            BOOL signd = [type characterAtIndex:0] == 's';
            BOOL minus = bit_get(swapped, BIT(15));
            
            if (signd && minus) bit_clear(swapped, BIT(15));
            
            return ((float)swapped / (float)BIT(f)) * (signd && minus ? -1 : 1);
        }
    }
    
    return MAXFLOAT;
}

+ (HWMonitorSensor*)sensor
{
    HWMonitorSensor *me = [[HWMonitorSensor alloc] init];
    
    if (me) {
        me->_valueHasBeenChanged = true;
        
        return me;
    }
    
    return nil;
}

- (void)setType:(NSString *)newType
{
    if (!_type || ![_type isEqualToString:newType]) {
        _valueHasBeenChanged = true;
        _type = newType;
    }
}

- (void)setData:(NSData *)newData
{
    if (!_data || ![_data isEqualToData:newData]) {
        _valueHasBeenChanged = true;
        _data = newData;
    }
}

-(void)setLevel:(HWSensorLevel)level
{
    if (_level != level) {
        _valueHasBeenChanged = true;
        _level = level;
    }
}

- (float)decodeNumericValue
{
    return [HWMonitorSensor decodeNumericData:_data ofType:_type];
}

- (NSNumber*)rawValue
{
    if ((_rawValue == nil || _valueHasBeenChanged) && _data) {
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

- (NSString*)stringValue
{
    NSNumber *value = [self rawValue];
    
    if (value) {
        if (_group & kSMARTGroupTemperature) {
            if ([_engine useFahrenheit]) {
                _formattedValue = [NSString stringWithFormat:@"%1.0f°", [value floatValue] * (9.0f / 5.0f) + 32.0f];
            }
            else {
                _formattedValue = [NSString stringWithFormat:@"%d°", [value intValue]];
            }
        }
        else if (_group & (kSMARTGroupRemainingLife | kBluetoothGroupBattery | kHWSensorGroupPWM)) {
            if ([value floatValue] < 0 || [value floatValue] > 100) {
                _formattedValue = @"-";
            }
            else {
                _formattedValue = [[NSString alloc] initWithFormat:@"%1.0f%%", [value floatValue]];
            }
        }
        else if (_group & kSMARTGroupRemainingBlocks) {
            _formattedValue = [NSString stringWithFormat:@"%lld", [value longLongValue]];
        }
        else if (_group & kHWSensorGroupTemperature) {
            if ([_engine useFahrenheit]) {
                _formattedValue = [NSString stringWithFormat:@"%1.0f°", [value floatValue] * (9.0f / 5.0f) + 32.0f];
            }
            else {
                _formattedValue = [NSString stringWithFormat:@"%d°", [value intValue]];
            }
        }
        else if (_group & kHWSensorGroupMultiplier) {
            _formattedValue = [NSString stringWithFormat:@"x%1.1f", [value floatValue]];
        }
        else if (_group & kHWSensorGroupFrequency) {
            if ([value floatValue] > 1e6)
                _formattedValue = [NSString stringWithFormat:@"%1.2fTHz", [value floatValue] / 1e6];
            else if ([value floatValue] > 1e3)
                _formattedValue = [NSString stringWithFormat:@"%1.2fGHz", [value floatValue] / 1e3];
            else 
                _formattedValue = [NSString stringWithFormat:@"%1.0fMHz", [value floatValue]];
        }
        else if (_group & kHWSensorGroupTachometer) {
            if ([value floatValue] < 10) {
                //rehabman: it is normal on a laptop to have a fan read 0 RPM...
                //[self setLevel:kHWSensorLevelExceeded];
                _formattedValue = @"-";
            }
            else {
                _formattedValue = [NSString stringWithFormat:@"%1.0frpm", [value floatValue]];
            }
        }
        else if (_group & kHWSensorGroupVoltage) {
            _formattedValue = [NSString stringWithFormat:@"%1.2fV", [value floatValue]];
        }
        else if (_group & kHWSensorGroupCurrent) {
            _formattedValue = [NSString stringWithFormat:@"%1.2fA", [value floatValue]];
        }
        else if (_group & kHWSensorGroupPower) {
            _formattedValue = [NSString stringWithFormat:@"%1.2fW", [value floatValue]];
        }
        else {
            _formattedValue = @"-";
        }
    }
    else {
        _formattedValue = @"-";
    }
    
    return _formattedValue;
}

-(NSInteger)intValue
{
    return [[self rawValue] intValue];
}

-(double)doubleValue
{
    return [[self rawValue] doubleValue];
}

-(float)floatValue
{
    return [[self rawValue] floatValue];
}

@end
