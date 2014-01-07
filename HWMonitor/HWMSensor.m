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
#import "HWMonitorDefinitions.h"

#import <Growl/Growl.h>

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
    [super awakeFromFetch];
    
    [self addObserver:self forKeyPath:@"value" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)awakeFromInsert
{
    [super awakeFromInsert];

    [self addObserver:self forKeyPath:@"value" options:NSKeyValueObservingOptionNew context:nil];
}

-(void)prepareForDeletion
{
    [super prepareForDeletion];

    [self removeObserver:self forKeyPath:@"value"];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"value"]) {
        _formattedValue = nil;
        _strippedValue = nil;
    }

    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

//-(void)setValue:(NSNumber *)value
//{
//    if ([self.value isNotEqualTo:value]) {
//
//        _formattedValue = nil;
//        _strippedValue = nil;
//
//        [self willChangeValueForKey:@"value"];
//        [self setPrimitiveValue:value forKey:@"value"];
//        [self didChangeValueForKey:@"value"];
//    }
//}

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

-(void)internalSendAlarmNotification
{
    if (self.selector.unsignedIntegerValue == kHWMGroupSmartTemperature ||
        self.selector.unsignedIntegerValue == kHWMGroupSmartTemperature) {
        switch (_alarmLevel) {
            case kHWMSensorLevelExceeded:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ completely overheated!"), self.title]
                                       notificationName:NotifierSensorLevelExceededNotification
                                               iconData:nil
                                               priority:1000
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelHigh:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ is overheating"), self.title]
                                       notificationName:NotifierSensorLevelHighNotification
                                               iconData:nil
                                               priority:500
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelModerate:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ is hot"), self.title]
                                       notificationName:NotifierSensorLevelModerateNotification
                                               iconData:nil
                                               priority:0
                                               isSticky:YES
                                           clickContext:nil];
                break;

            default:
                break;
        }
    }
    else {
        switch (_alarmLevel) {
            case kHWMSensorLevelExceeded:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ alarm level is exceeded limit"), self.title]
                                       notificationName:NotifierSensorLevelExceededNotification
                                               iconData:nil
                                               priority:1000
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelHigh:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ high alarm level"), self.title]
                                       notificationName:NotifierSensorLevelHighNotification
                                               iconData:nil
                                               priority:500
                                               isSticky:YES
                                           clickContext:nil];
                break;

            case kHWMSensorLevelModerate:
                [GrowlApplicationBridge notifyWithTitle:GetLocalizedString(@"Sensor alarm level changed")
                                            description:[NSString stringWithFormat:GetLocalizedString(@"%@ moderate alarm level"), self.title]
                                       notificationName:NotifierSensorLevelModerateNotification
                                               iconData:nil
                                               priority:0
                                               isSticky:YES
                                           clickContext:nil];
                break;

            default:
                break;
        }
    }
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

        if (!self.hidden.boolValue) {
            NSUInteger alarmLevel = [self internalUpdateAlarmLevel];

            if (alarmLevel != _alarmLevel) {
                [self willChangeValueForKey:@"alarmLevel"];
                _alarmLevel = alarmLevel;
                [self didChangeValueForKey:@"alarmLevel"];

                if (self.engine.configuration.notifyAlarmLevelChanges.boolValue) {
                    [self internalSendAlarmNotification];
                }
            }
        }
    }
}

@end
