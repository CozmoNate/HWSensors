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
#import "HWMGraph.h"
#import "HWMSensorController.h"
#import "HWMSensorsGroup.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMValueFormatter.h"
#import "HWMonitorDefinitions.h"
#import "Localizer.h"

#import <Growl/Growl.h>
#import <ReactiveCocoa/ReactiveCocoa.h>

@implementation HWMSensor

@dynamic forced;
@dynamic selector;
@dynamic service;
@dynamic type;
@dynamic value;
@dynamic graph;
@dynamic group;
@dynamic controller;
@dynamic consumers;

@synthesize alarmLevel = _alarmLevel;

-(void)initialize
{
    [super initialize];

    [[RACObserve(self, value)
      takeUntil:self.hasBeenDeletedSignal]
     subscribeNext:^(id x) {
         _formattedValue = nil;
         _strippedValue = nil;
     }];
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

-(void)internalSendAlarmNotification
{
    switch (self.selector.unsignedIntegerValue) {

        case kHWMGroupTemperature:
        case kHWMGroupSmartTemperature:
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
                                                   priority:100
                                                   isSticky:YES
                                               clickContext:nil];
                    break;

                default:
                    break;
            }

            break;

        default:

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

            break;
    }
}

- (BOOL)isActive
{
    return self.service.unsignedLongLongValue > 0;
}

- (void)doUpdateValue
{
    NSNumber *value = [self internalUpdateValue];

    if (value) {

        //_lastUpdated = [NSDate date];

        if (value && (!self.value || ![value isEqualToNumber:self.value])) {
            [self willChangeValueForKey:@keypath(self, value)];
            [self setPrimitiveValue:value forKey:@keypath(self, value)];
            [self didChangeValueForKey:@keypath(self, value)];

            [self willChangeValueForKey:@keypath(self, formattedValue)];
            [self didChangeValueForKey:@keypath(self, formattedValue)];

            if (!self.hidden.boolValue) {

                NSUInteger alarmLevel = [self internalUpdateAlarmLevel];

                if (alarmLevel != _alarmLevel || _alarmLevel == 0) {
                    [self willChangeValueForKey:@keypath(self, alarmLevel)];
                    _alarmLevel = alarmLevel;
                    [self didChangeValueForKey:@keypath(self, alarmLevel)];
                    
                    if (self.engine.configuration.notifyAlarmLevelChanges.boolValue) {
                        [self internalSendAlarmNotification];
                    }
                }
            }
        }
    }
}

@end
