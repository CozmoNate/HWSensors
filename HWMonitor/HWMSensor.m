//
//  HWMSensor.m
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
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

#import "HWMSensor.h"
#import "HWMSensorsGroup.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMValueFormatter.h"

#import "Localizer.h"

@implementation HWMSensor

@dynamic service;
@dynamic selector;
@dynamic type;
@dynamic value;
@dynamic group;
@dynamic graph;
@dynamic favorite;

@synthesize alarmLevel = _alarmLevel;

-(void)awakeFromFetch
{
    [self setPrimitiveValue:@0 forKey:@"service"];
    
    [self addObserver:self forKeyPath:@"value" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)awakeFromInsert
{
    [self addObserver:self forKeyPath:@"value" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)prepareForDeletion
{
    [self removeObserver:self forKeyPath:@"value"];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"value"]) {
        _formattedValue = nil;
        _strippedValue = nil;
    }
}

-(NSString *)formattedValue
{
    if (!_formattedValue) {
        _formattedValue = [HWMValueFormatter formattedValue:self.value usingRulesOfGroup:self.selector configuration:self.engine.configuration];
    }
    
    return _formattedValue;
}

-(NSString *)strippedValue
{
    if (!_strippedValue) {
        _strippedValue = [HWMValueFormatter strippedValue:self.value usingRulesOfGroup:self.selector configuration:self.engine.configuration];
    }
    
    return _strippedValue;
}

-(NSUInteger)internalUpdateAlarmLevel
{
    return kHWMSensorLevelNormal;
}

-(NSNumber *)internalUpdateValue
{
    return @0;
}

- (void)doUpdateValue
{
    NSNumber *value = [self internalUpdateValue];

    if (value && (!self.value || ![value isEqualToNumber:self.value])) {
        [self willChangeValueForKey:@"value"];
        [self willChangeValueForKey:@"formattedValue"];

        [self setPrimitiveValue:value forKey:@"value"];

        [self didChangeValueForKey:@"value"];
        [self didChangeValueForKey:@"formattedValue"];

        NSUInteger alarmLevel = [self internalUpdateAlarmLevel];

        if (alarmLevel != _alarmLevel) {
            [self willChangeValueForKey:@"alarmLevel"];
            _alarmLevel = alarmLevel;
            [self didChangeValueForKey:@"alarmLevel"];
        }
    }
}

@end
