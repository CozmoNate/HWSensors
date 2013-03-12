//
//  HWMonitorEngine.m
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


#include "HWMonitorEngine.h"

#include "smc.h"
#include "FakeSMCDefinitions.h"
#include "SmcKeysDefinitions.h"

@implementation HWMonitorEngine

#define GetLocalizedString(key) \
[_bundle localizedStringForKey:(key) value:@"" table:nil]

+ (HWMonitorEngine*)engineWithBundle:(NSBundle*)bundle;
{
    HWMonitorEngine *me = [[HWMonitorEngine alloc] init];
    
    if (me) {
        [me setBundle:bundle];
        [me rebuildSensorsList];
    }
    
    return me;
}

+ (NSString*)copyTypeFromKeyInfo:(NSArray*)info
{
    if (info && [info count] == 2) {
        //NSString *type = (NSString*)[info objectAtIndex:0];
        
        //NSLog(@"%s", [type cStringUsingEncoding:NSASCIIStringEncoding]);
        
        return [NSString stringWithString:(NSString*)[info objectAtIndex:0]];
    }
    
    return nil;
}

+ (NSData*)copyValueFromKeyInfo:(NSArray*)info
{
    if (info && [info count] == 2)
        return [NSData dataWithData:(NSData *)[info objectAtIndex:1]];
    
    return nil;
}

-(void)setUseBSDNames:(BOOL)useBSDNames
{
    //if (_useBSDNames != useBSDNames) {
        _useBSDNames = useBSDNames;
        
        for (HWMonitorSensor *sensor in [self sensors])
            if ([sensor genericDevice])
                [sensor setTitle:_useBSDNames ? [[sensor genericDevice] bsdName] : [[[sensor genericDevice] productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]]];
    //}
}

- (NSArray*)populateInfoForKey:(NSString*)key
{
    NSArray * info = nil;
    
    if (_connection || kIOReturnSuccess == SMCOpen(&_connection)) {
        SMCVal_t val;
        UInt32Char_t name;
        
        strncpy(name, [key cStringUsingEncoding:NSASCIIStringEncoding], 5);
        
        if (kIOReturnSuccess == SMCReadKey(_connection, name, &val)) {
            info = [NSArray arrayWithObjects:
                    [NSString stringWithCString:val.dataType encoding:NSASCIIStringEncoding],
                    [NSData dataWithBytes:val.bytes length:val.dataSize],
                    nil];
        }
        
        //SMCClose(_connection);
        //_connection = 0;
    }
    
    return info;
}

- (HWMonitorSensor*)addSensorWithKey:(NSString*)key title:(NSString*)title group:(NSUInteger)group
{
    HWMonitorSensor *sensor = nil;
    NSString *type = nil;
    NSData *value = nil;
    //BOOL smartSensor = FALSE;
    
    switch (group) {
        case kSMARTGroupTemperature:
        case kSMARTGroupRemainingLife:
        case kSMARTGroupRemainingBlocks:
            //smartSensor = TRUE;
            break;
            
        default: {
            NSArray *info = [self populateInfoForKey:key];
            
            if (!info || [info count] != 2)
                return nil;
            
            type = [HWMonitorEngine copyTypeFromKeyInfo:info];
            value = [HWMonitorEngine copyValueFromKeyInfo:info];
            
            if (!type || [type length] == 0 || !value)
                return nil;
            
            switch (group) {
                case kHWSensorGroupTemperature:
                    [sensor setLevel:kHWSensorLevelDisabled];
                    break;
                default:
                    [sensor setLevel:kHWSensorLevelUnused];
                    break;
            }
            
            break;
        }
    }
    
    sensor = [HWMonitorSensor sensor];
    
    [sensor setEngine:self];
    [sensor setName:key];
    [sensor setType:type];
    [sensor setTitle:title];
    [sensor setData:value];
    [sensor setGroup:group];
    
    /*if (!smartSensor && _hideDisabledSensors && [[sensor value] isEqualToString:@"-"]) {
        [sensor setEngine:nil];
        sensor = nil;
        return nil;
    }*/
    
    [_sensors addObject:sensor];
    [_keys setObject:sensor forKey:key];
        
    return sensor;
}

- (HWMonitorSensor*)addSMARTSensorWithGenericDisk:(ATAGenericDisk*)disk group:(NSUInteger)group
{
    NSData * value = nil;
    
    switch (group) {
        case kSMARTGroupTemperature:
            value = [disk getTemperature];

            UInt16 t = 0;
            
            [value getBytes:&t length:2];
            
            // Don't add sensor if value is insane
            if (t == 0 || t > 99)
                return nil;
            
            break;
            
        case kSMARTGroupRemainingLife:
            value = [disk getRemainingLife];
            
            UInt64 life = 0;
            
            [value getBytes:&life length:[value length]];
            
            if (life > 100)
                return nil;
            
            break;
            
        case kSMARTGroupRemainingBlocks:
            value = [disk getRemainingBlocks];
            
            UInt64 blocks = 0;
            
            [value getBytes:&blocks length:[value length]];
            
            if (blocks == 0xffffffffffff)
                return nil;
            
            break;
    }
    
    if (value) {
        HWMonitorSensor *sensor = [self addSensorWithKey:[NSString stringWithFormat:@"%@%lx", [disk serialNumber], group] title:_useBSDNames ? [disk bsdName] : [[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] group:group];
        
        [sensor setData:value];
        [sensor setGenericDevice:disk];
        if ([disk isExceeded]) [sensor setLevel:kHWSensorLevelExceeded];
        
        return sensor;
    }
    
    return nil;
}

- (HWMonitorSensor*)addBluetoothSensorWithGenericDevice:(BluetoothGenericDevice*)device group:(NSUInteger)group
{
    NSData *level = [device getBatteryLevel];
    
    if (level) {
        /*HWMonitorSensor *sensor = [self addSensorWithKey:[NSString stringWithFormat:@"%@%lx", [disk serialNumber], group] title:_useBSDNames ? [disk bsdName] : [[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] group:group];
        
        [sensor setData:value];
        [sensor setGenericDevice:disk];
        if ([disk isExceeded]) [sensor setLevel:kHWSensorLevelExceeded];
        
        return sensor;*/
    }
    
    return nil;
}

- (id)init;
{
    self = [super init];
    
    _smartReporter = [NSATASmartReporter smartReporterByDiscoveringDrives];
    _sensors = [[NSMutableArray alloc] init];
    _keys = [[NSMutableDictionary alloc] init];
    _bundle = [NSBundle mainBundle];
    _sensorsLock = [[NSLock alloc] init];
    
    return self;
}

- (id)initWithBundle:(NSBundle*)mainBundle;
{
    self = [super init];
    
    _smartReporter = [NSATASmartReporter smartReporterByDiscoveringDrives];
    _sensors = [[NSMutableArray alloc] init];
    _keys = [[NSMutableDictionary alloc] init];
    _bundle = mainBundle;
    _sensorsLock = [[NSLock alloc] init];
    
    return self;
}

- (void)dealloc
{
    if (_connection) {
        SMCClose(_connection);
        _connection = 0;
    }
}

- (NSIndexSet*)getKeyInfosInGroup:(SMCKeyGroup)group
{
    int count = sizeof(SMCKeyInfoList) / sizeof(SMCKeyInfo);
    
    NSMutableIndexSet *indexSet = [[NSMutableIndexSet alloc] init];
    
    for (int index = 0; index < count; index++) {
        if (SMCKeyInfoList[index].group == group) {
            [indexSet addIndex:index];
        }
    }
    
    return [indexSet copy];
}

- (void)addSensorsFromSMCKeyGroup:(SMCKeyGroup)fromGroup toHWSensorGroup:(HWSensorGroup)toGroup
{
    [[self getKeyInfosInGroup:fromGroup] enumerateIndexesWithOptions:NSSortStable usingBlock:^(NSUInteger idx, BOOL *stop) {
        if (SMCKeyInfoList[idx].count) {
            for (NSUInteger index = 0; index < SMCKeyInfoList[idx].count; index++) {
                NSString *keyFormat = [NSString stringWithCString:SMCKeyInfoList[idx].key encoding:NSASCIIStringEncoding];
                NSString *titleFormat = GetLocalizedString([NSString stringWithCString:SMCKeyInfoList[idx].title encoding:NSASCIIStringEncoding]);
                [self addSensorWithKey:[NSString stringWithFormat:keyFormat, index + SMCKeyInfoList[idx].offset] title:[NSString stringWithFormat:titleFormat, index + SMCKeyInfoList[idx].shift] group:toGroup];
            }
        }
        else {
            NSString *key = [NSString stringWithCString:SMCKeyInfoList[idx].key encoding:NSASCIIStringEncoding];
            NSString *title = GetLocalizedString([NSString stringWithCString:SMCKeyInfoList[idx].title encoding:NSASCIIStringEncoding]);
            [self addSensorWithKey:key title:title group:toGroup];
        }
    }];
}

- (void)rebuildSensorsList
{
    [_sensorsLock lock];
    
    [_sensors removeAllObjects];
    [_keys removeAllObjects];
    
    //Temperatures
    
    [self addSensorsFromSMCKeyGroup:kSMCKeyGroupTemperature toHWSensorGroup:kHWSensorGroupTemperature];
    
    if ([_smartReporter drives]) {
        for (NSUInteger i = 0; i < [[_smartReporter drives] count]; i++) {
            ATAGenericDisk * disk = [[_smartReporter drives] objectAtIndex:i];
            
            if (disk) { 
                // Hard Drive Temperatures
                [self addSMARTSensorWithGenericDisk:disk group:kSMARTGroupTemperature];
                
                if (![disk isRotational]) {
                    // SSD Remaining Life
                    [self addSMARTSensorWithGenericDisk:disk group:kSMARTGroupRemainingLife];
                    // SSD Remaining Blocks
                    [self addSMARTSensorWithGenericDisk:disk group:kSMARTGroupRemainingBlocks];
                }
            }
        }
    }
    
    // Multipliers
    [self addSensorsFromSMCKeyGroup:kSMCKeyGroupMultiplier toHWSensorGroup:kHWSensorGroupMultiplier];
    
    // Frequency
    [self addSensorsFromSMCKeyGroup:kSMCKeyGroupFrequency toHWSensorGroup:kHWSensorGroupFrequency];
    
    // Fans
    for (int i=0; i<0xf; i++) {
        NSString *keyType = [HWMonitorEngine copyTypeFromKeyInfo:[self populateInfoForKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i]]];
        
        if ([keyType isEqualToString:@TYPE_CH8]) {
            NSString * caption = [[NSString alloc] initWithData:[HWMonitorEngine copyValueFromKeyInfo:[self populateInfoForKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_ID,i]]] encoding: NSUTF8StringEncoding];
            
            if ([caption length] == 0)
                caption = [[NSString alloc] initWithFormat:GetLocalizedString(@"Fan %X"),i + 1];
            
            if (![caption hasPrefix:@"GPU "])
                [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i] title:GetLocalizedString(caption) group:kHWSensorGroupTachometer];
        }
        else if ([keyType isEqualToString:@TYPE_FDS]) {
            FanTypeDescStruct *fds = (FanTypeDescStruct*)[[HWMonitorEngine copyValueFromKeyInfo:[self populateInfoForKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i]]] bytes];
            
            if (fds) {
                NSString *caption = [NSString stringWithCString:fds->strFunction encoding:NSASCIIStringEncoding];
                
                if ([caption length] == 0)
                    caption = [[NSString alloc] initWithFormat:@"Fan %X", i + 1];
                
                switch (fds->type) {
                    case GPU_FAN_RPM:
                    case GPU_FAN_PWM_CYCLE:
                        break;
                        
                    case FAN_PWM_TACH:
                    //case FAN_PWM_NOTACH:
                    case PUMP_PWM:
                    case FAN_RPM:
                    case PUMP_RPM:
                    default:
                        [self addSensorWithKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_SPEED,i] title:GetLocalizedString(caption) group:kHWSensorGroupTachometer];
                        break;
                }
            }
        }
    }
    
    // GPU Fans
    for (int i=0; i < 0xf; i++) {
        NSString *keyType = [HWMonitorEngine copyTypeFromKeyInfo:[self populateInfoForKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i]]];
        
        if ([keyType isEqualToString:@TYPE_CH8]) {
            NSString * caption = [[NSString alloc] initWithData:[HWMonitorEngine copyValueFromKeyInfo:[self populateInfoForKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_ID,i]]] encoding: NSUTF8StringEncoding];
            
            if ([caption hasPrefix:@"GPU "]) {
                UInt8 cardIndex = [[caption substringFromIndex:4] intValue] - 1;
                NSString *title = cardIndex == 0 ? GetLocalizedString(@"GPU Fan") : [NSString stringWithFormat:GetLocalizedString(@"GPU %X Fan"), cardIndex + 1];
                [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i] title:title group:kHWSensorGroupTachometer];
            }
        }
        else if ([keyType isEqualToString:@TYPE_FDS]) {
            FanTypeDescStruct *fds = (FanTypeDescStruct*)[[HWMonitorEngine copyValueFromKeyInfo:[self populateInfoForKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i]]] bytes];
            
            switch (fds->type) {
                case GPU_FAN_RPM: {
                    NSString *title = fds->ui8Zone == 0 ? GetLocalizedString(@"GPU Fan") : [NSString stringWithFormat:GetLocalizedString(@"GPU %X Fan"), fds->ui8Zone + 1];
                    [self addSensorWithKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_SPEED, i] title:title group:kHWSensorGroupTachometer];
                    break;
                }
                    
                case GPU_FAN_PWM_CYCLE: {
                    NSString *title = fds->ui8Zone == 0 ? GetLocalizedString(@"GPU PWM") : [NSString stringWithFormat:GetLocalizedString(@"GPU %X PWM"), fds->ui8Zone + 1];
                    [self addSensorWithKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_SPEED, i] title:title group:kHWSensorGroupPWM];
                    break;
                }
            }
        }
    }
    
    // Voltages
    [self addSensorsFromSMCKeyGroup:kSMCKeyGroupVoltage toHWSensorGroup:kHWSensorGroupVoltage];
    
    // Currents
    [self addSensorsFromSMCKeyGroup:kSMCKeyGroupCurrent toHWSensorGroup:kHWSensorGroupCurrent];
    
    // Powers
    [self addSensorsFromSMCKeyGroup:kSMCKeyGroupPower toHWSensorGroup:kHWSensorGroupPower];
    
    [_sensorsLock unlock];
}

- (NSArray*)updateSmartSensors
{
    [_sensorsLock lock];
    
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    for (HWMonitorSensor *sensor in [self sensors]) {
        if ([sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[ATAGenericDisk class]]) {
            switch ([sensor group]) {
                case kSMARTGroupTemperature:
                    [sensor setData:[[sensor genericDevice] getTemperature]];
                    break;
                    
                case kSMARTGroupRemainingLife:
                    [sensor setData:[[sensor genericDevice] getRemainingLife]];
                    break;
                    
                case kSMARTGroupRemainingBlocks:
                    [sensor setData:[[sensor genericDevice] getRemainingBlocks]];
                    break;
                        
                default:
                    break;
            }
            
            if ([sensor valueHasBeenChanged]) {
                [list addObject:sensor];
            }
        }
    }
    
    [_sensorsLock unlock];
    
    return list;
}

- (NSArray*)updateSmcSensors
{
    [_sensorsLock lock];
    
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    if (_connection || kIOReturnSuccess == SMCOpen(&_connection)) {
        for (HWMonitorSensor *sensor in [self sensors]) {
            if (![sensor genericDevice]) {
                SMCVal_t val;
                UInt32Char_t name;
                
                strncpy(name, [[sensor name] cStringUsingEncoding:NSASCIIStringEncoding], 5);
                
                if (kIOReturnSuccess == SMCReadKey(_connection, name, &val)) {
                    [sensor setType:[NSString stringWithCString:val.dataType encoding:NSASCIIStringEncoding]];
                    [sensor setData:[NSData dataWithBytes:val.bytes length:val.dataSize]];
                    
                    if ([sensor valueHasBeenChanged]) {
                        [list addObject:sensor];
                    }
                }
            }
        }
    }
    else if (_connection) {
        SMCClose(_connection);
        _connection = 0;
    }
    
    [_sensorsLock unlock];
    
    return list;
}

-(NSArray*)updateSmcSensorsList:(NSArray *)sensors
{
    if (!sensors) return nil; // [self updateSmcSensors];
    
    [_sensorsLock lock];
    
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    for (id object in sensors) {
        if ([object isKindOfClass:[HWMonitorSensor class]] && [[self sensors] containsObject:object] && ![object genericDevice])
            [list addObject:object];
    }
    
    if (_connection || kIOReturnSuccess == SMCOpen(&_connection)) {
        for (HWMonitorSensor *sensor in list) {
            SMCVal_t val;
            UInt32Char_t name;
            
            strncpy(name, [[sensor name] cStringUsingEncoding:NSASCIIStringEncoding], 5);
            
            if (kIOReturnSuccess == SMCReadKey(_connection, name, &val)) {
                [sensor setType:[NSString stringWithCString:val.dataType encoding:NSASCIIStringEncoding]];
                [sensor setData:[NSData dataWithBytes:val.bytes length:val.dataSize]];
            }
        }
    }
    
    [_sensorsLock unlock];
    
    return list;
}

- (NSArray*)getAllSensorsInGroup:(NSUInteger)group
{
    [_sensorsLock lock];
    
    NSMutableArray * list = [[NSMutableArray alloc] init];
    
    for (HWMonitorSensor *sensor in [self sensors])
        if (group & [sensor group])
            [list addObject:sensor];
    
    [_sensorsLock unlock];
    
    return [list count] > 0 ? [NSArray arrayWithArray:list] : nil;
}

@end
