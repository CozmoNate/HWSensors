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
#include "HWMonitorProfiles.h"

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

+ (NSString*)getTypeStringFromSmcKeyInfo:(NSData*)info
{
    if (info && [info length] == sizeof(SMCVal_t)) {
        SMCVal_t *val = (SMCVal_t*)[info bytes];
        return [NSString stringWithCString:val->dataType encoding:NSASCIIStringEncoding];
    }
    
    return nil;
}

+ (NSData*)getValueDataFromSmcKeyInfo:(NSData*)info
{
    if (info && [info length] == sizeof(SMCVal_t)) {
        SMCVal_t *val = (SMCVal_t*)[info bytes];
        return [NSData dataWithBytes:val->bytes length:val->dataSize];
    }
    return nil;
}

-(void)setUseFahrenheit:(BOOL)useFahrenheit
{
    _useFahrenheit = useFahrenheit;
    
    [_sensorsLock lock];
    
    for (HWMonitorSensor *sensor in [self sensors]) {
        if ([sensor group] & (kHWSensorGroupTemperature | kSMARTGroupTemperature)) {
            [sensor setValueHasBeenChanged:YES];
        }
    }
        
    [_sensorsLock unlock];
}

-(void)setUseBsdNames:(BOOL)useBsdNames
{
    _useBsdNames = useBsdNames;

    [_sensorsLock lock];

    for (HWMonitorSensor *sensor in [self sensors])
        if ([sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[ATAGenericDrive class]])
            [sensor setTitle:_useBsdNames ? [[sensor genericDevice] bsdName] : [[[sensor genericDevice] productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]]];
    
    [_sensorsLock unlock];
}

- (NSData*)getSmcKeyInfoForKey:(NSString*)key
{
    SMCVal_t result;
    
    if (_connection || kIOReturnSuccess == SMCOpen(&_connection)) {
        
        UInt32Char_t name;
        
        [key getCString:name maxLength:5 encoding:NSASCIIStringEncoding];
        
        if (kIOReturnSuccess != SMCReadKey(_connection, name, &result)) {
            return nil;
        }
    }
    
    return [NSData dataWithBytes:&result length:sizeof(result)];
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
        case kBluetoothGroupBattery:
            //smartSensor = TRUE;
            break;
            
        default: {
            NSData *data = [self getSmcKeyInfoForKey:key];
            SMCVal_t *info = (SMCVal_t*)[data bytes];
            
            if (!data || [data length] != sizeof(SMCVal_t))
                return nil;
            
            type = [NSString stringWithCString:info->dataType encoding:NSASCIIStringEncoding];
            value = [NSData dataWithBytes:info->bytes length:info->dataSize];
            
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
    
    [sensor setName:key];
    [sensor setType:type];
    [sensor setTitle:title];
    [sensor setData:value];
    [sensor setGroup:group];
    
    // Sanity checks
    switch (group) {
        case kHWSensorGroupTemperature:
        {
            float t = [sensor floatValue];
            
            if (t < 0 || t > 100) {
                return nil;
            }
            
            break;
        }
        
        default:
            break;
    }
    
    /*if (!smartSensor && _hideDisabledSensors && [[sensor value] isEqualToString:@"-"]) {
        [sensor setEngine:nil];
        sensor = nil;
        return nil;
    }*/
    
    [sensor setEngine:self];
    [_sensors addObject:sensor];
    [_keys setObject:sensor forKey:key];
        
    return sensor;
}

- (HWMonitorSensor*)addSmartSensorWithGenericDisk:(ATAGenericDrive*)disk group:(NSUInteger)group
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
            
            if (blocks >= 0xffffffffff)
                return nil;
            
            break;
    }
    
    if (value) {
        HWMonitorSensor *sensor = [self addSensorWithKey:[NSString stringWithFormat:@"%@%lx", [disk serialNumber], group] title:_useBsdNames ? [disk bsdName] : [[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] group:group];
        
        [sensor setData:value];
        [sensor setGenericDevice:disk];
        if ([disk isExceeded]) [sensor setLevel:kHWSensorLevelExceeded];
        
        return sensor;
    }
    
    return nil;
}

- (HWMonitorSensor*)addBluetoothSensorWithGenericDevice:(GenericBatteryDevice*)device group:(NSUInteger)group
{
    if ([device getBatteryLevel]) {
        
        HWMonitorSensor *sensor = nil;
        
        if (![device serialNumber] || [[device serialNumber] length] == 0) {
            [device setSerialNumber:[NSString stringWithFormat:@"%X", device.service]];
        }
        
        NSString *title = nil;
        
        switch ([device deviceType]) {
            case kBluetoothDeviceTypeKeyboard:
                title = @"Keyboard";
                break;
            case kBluetoothDeviceTypeMouse:
                title = @"Mouse";
                break;
            case kBluetoothDeviceTypeTrackpad:
                title = @"Trackpad";
                break;
            case kInternalBatteryType:
                title = @"Built-in Battery";
                break;
            default:
                title = @"Unknown";
                break;
        }
        
        sensor = [self addSensorWithKey:[device serialNumber] title:GetLocalizedString(title) group:group];
        
        [sensor setGenericDevice:device];
        [sensor setData:[device getBatteryLevel]];
        
        return sensor;
    }
    
    return nil;
}

- (void)assignPlatformProfile
{
    NSDictionary *profiles = [HWMonitorProfiles profiles];
    
    NSString *model = nil;
    
    CFDictionaryRef matching = MACH_PORT_NULL;
    
    if (MACH_PORT_NULL != (matching = IOServiceMatching("FakeSMC"))) {
        io_iterator_t iterator = IO_OBJECT_NULL;
        
        if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
            if (IO_OBJECT_NULL != iterator) {
                
                io_service_t service = MACH_PORT_NULL;
                
                if (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                    model = @"Hackintosh";
                    
                    IOObjectRelease(service);
                }
                
                IOObjectRelease(iterator);
            }
        }
    }
    
    if (!model) {
        if (MACH_PORT_NULL != (matching = IOServiceMatching("IOPlatformExpertDevice"))) {
            io_iterator_t iterator = IO_OBJECT_NULL;
            
            if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
                if (IO_OBJECT_NULL != iterator) {
                    
                    io_service_t service = MACH_PORT_NULL;

                    if (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                        model = [[NSString alloc] initWithData:(__bridge_transfer NSData *)IORegistryEntryCreateCFProperty(service, CFSTR("model"), kCFAllocatorDefault, 0) encoding:NSASCIIStringEncoding];
                        
                        IOObjectRelease(service);
                    }
                    
                    IOObjectRelease(iterator);
                }
            }
        }
    }
    
    if (model) {

        NSLog(@"Running on %@", model);
        
        [profiles enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
            if ([model isCaseInsensitiveLike:key]) {
                _currentProfile = obj;
                *stop = YES;
            }
        }];
        
        if (!_currentProfile) {
            NSLog(@"Using default platform profile");
            _currentProfile = [profiles objectForKey:@"Default"];
        }
    }
    else NSLog(@"Using default platform profile");
}

- (id)init;
{
    self = [super init];
    
    if (self) {
        _sensors = [[NSMutableArray alloc] init];
        _keys = [[NSMutableDictionary alloc] init];
        _sensorsLock = [[NSLock alloc] init];
        
        _bundle = [NSBundle mainBundle];
        
        [self assignPlatformProfile];
    }
    
    return self;
}

- (id)initWithBundle:(NSBundle*)bundle;
{
    self = [self init];
    
    if (self) {
        _bundle = bundle;
    }
    
    return self;
}

- (void)dealloc
{
    if (_connection) {
        SMCClose(_connection);
        _connection = 0;
    }
    
}

- (void)addSensorsFromGroup:(HWSensorGroup)group withKeysList:(NSArray*)keys
{
    [_currentProfile enumerateObjectsUsingBlock:^(id item, NSUInteger index, BOOL *stop) {
        NSString *prefix = nil;
        
        switch (group) {
            case kHWSensorGroupTemperature:
                prefix = @"T";
                break;
                
            case kHWSensorGroupMultiplier:
                prefix = @"M";
                break;
                
            case kHWSensorGroupFrequency:
                prefix = @"C";
                break;
                
            case kHWSensorGroupVoltage:
                prefix = @"V";
                break;
                
            case kHWSensorGroupCurrent:
                prefix = @"I";
                break;
                
            case kHWSensorGroupPower:
                prefix = @"P";
                break;
                
            default:
                *stop = YES;
                break;
        }
        
        NSString *key = [item objectAtIndex:0];
        
        if ([key hasPrefix:prefix]) {
            NSString *title = [item objectAtIndex:1];
            
            NSRange formater = [key rangeOfString:@":"];

            if (formater.length) {
                // key and title should be formatted by index
                unsigned int start = 0;
                [[NSScanner scannerWithString:[key substringWithRange:NSMakeRange(formater.location + 1, 1)]] scanHexInt:&start];
                unsigned int count = 0;
                [[NSScanner scannerWithString:[key substringWithRange:NSMakeRange(formater.location + 2, 1)]] scanHexInt:&count];
                unsigned int shift = 0;
                [[NSScanner scannerWithString:[key substringWithRange:NSMakeRange(formater.location + 3, 1)]] scanHexInt:&shift];
                
                NSString *keyFormat = [NSString stringWithFormat:@"%@%%X%@", [key substringToIndex:formater.location], [key substringFromIndex:formater.location + formater.length + 3]];
                
                for (NSUInteger index = 0; index < count; index++) {
                    NSString *formattedKey = [NSString stringWithFormat:keyFormat, start + index];

                    if ([keys indexOfObject:formattedKey] != NSNotFound) {
                        [self addSensorWithKey:formattedKey title:[NSString stringWithFormat:GetLocalizedString(title), shift + index] group:group];
                    }
                }
            }
            else if ([keys indexOfObject:key] != NSNotFound) {
                [self addSensorWithKey:key title:GetLocalizedString(title) group:group];
            }
        }
        
    }];
}

- (void)rebuildSensorsList
{
    [_sensorsLock lock];
    
    [_sensors removeAllObjects];
    [_keys removeAllObjects];
    
    if (_connection || kIOReturnSuccess == SMCOpen(&_connection)) {

        SMCVal_t val;
        
        SMCReadKey(_connection, "#KEY", &val);
        UInt32 count =  [HWMonitorSensor decodeNumericData:[NSData dataWithBytes:val.bytes length:val.dataSize] ofType:                         [NSString stringWithCString:val.dataType encoding:NSASCIIStringEncoding]];
        
        NSMutableArray *list = [[NSMutableArray alloc] initWithCapacity:count];
        
        for (UInt32 index = 0; index < count; index++) {
            SMCKeyData_t  inputStructure;
            SMCKeyData_t  outputStructure;
            
            memset(&inputStructure, 0, sizeof(SMCKeyData_t));
            memset(&outputStructure, 0, sizeof(SMCKeyData_t));
            memset(&val, 0, sizeof(SMCVal_t));
            
            inputStructure.data8 = SMC_CMD_READ_INDEX;
            inputStructure.data32 = index;
            
            if (kIOReturnSuccess == SMCCall(_connection, KERNEL_INDEX_SMC, &inputStructure, &outputStructure)) {
                [list addObject:[NSString stringWithFormat:@"%c%c%c%c",
                                 (unsigned int) outputStructure.key >> 24,
                                 (unsigned int) outputStructure.key >> 16,
                                 (unsigned int) outputStructure.key >> 8,
                                 (unsigned int) outputStructure.key]];
            }
        }
        
        //Temperatures
        
        [self addSensorsFromGroup:kHWSensorGroupTemperature withKeysList:list];
        
        if ((_smartDrives = [ATAGenericDrive discoverDrives])) {
            for (ATAGenericDrive * drive in _smartDrives) {
                // Hard Drive Temperatures
                [self addSmartSensorWithGenericDisk:drive group:kSMARTGroupTemperature];
                
                if (![drive isRotational]) {
                    // SSD Remaining Life
                    [self addSmartSensorWithGenericDisk:drive group:kSMARTGroupRemainingLife];
                    // SSD Remaining Blocks
                    [self addSmartSensorWithGenericDisk:drive group:kSMARTGroupRemainingBlocks];
                }
            }
        }
        
        // Multipliers
        [self addSensorsFromGroup:kHWSensorGroupMultiplier withKeysList:list];
        
        // Frequency
        [self addSensorsFromGroup:kHWSensorGroupFrequency withKeysList:list];
        
        // Fans
        for (int i=0; i<0xf; i++) {
            NSString *key = [NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i];
            
            if ([list indexOfObject:key] != NSNotFound) {
                NSData *info = [self getSmcKeyInfoForKey:key];
                NSString *type = [HWMonitorEngine getTypeStringFromSmcKeyInfo:info];
                
                if ([type isEqualToString:@TYPE_CH8]) {
                    NSString * caption = [[NSString alloc] initWithData:[HWMonitorEngine getValueDataFromSmcKeyInfo:info] encoding: NSUTF8StringEncoding];
                    
                    if ([caption length] == 0)
                        caption = [[NSString alloc] initWithFormat:GetLocalizedString(@"Fan %X"),i + 1];
                    
                    if (![caption hasPrefix:@"GPU "])
                        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i] title:GetLocalizedString(caption) group:kHWSensorGroupTachometer];
                }
                else if ([type isEqualToString:@TYPE_FDS]) {
                    FanTypeDescStruct *fds = (FanTypeDescStruct*)[[HWMonitorEngine getValueDataFromSmcKeyInfo:info] bytes];
                    
                    if (fds) {
                        NSString *caption = [[NSString stringWithCString:fds->strFunction encoding:NSASCIIStringEncoding] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
                        
                        if ([caption length] == 0)
                            caption = [[NSString alloc] initWithFormat:@"Fan %X", i + 1];
                        
                        switch (fds->type) {
                            case GPU_FAN_RPM:
                            case GPU_FAN_PWM_CYCLE:
                                // Add it later as GPU sensors
                                break;
                                
                            default:
                                [self addSensorWithKey:[NSString stringWithFormat:@KEY_FORMAT_FAN_SPEED,i] title:GetLocalizedString(caption) group:kHWSensorGroupTachometer];
                                break;
                        }
                    }
                }
            }
        }
        
        // GPU Fans
        for (int i=0; i < 0xf; i++) {
            NSString *key = [NSString stringWithFormat:@KEY_FORMAT_FAN_ID,i];
            
            if ([list indexOfObject:key] != NSNotFound) {
                NSData *info = [self getSmcKeyInfoForKey:key];
                NSString *type = [HWMonitorEngine getTypeStringFromSmcKeyInfo:info];
                
                if ([type isEqualToString:@TYPE_CH8]) {
                    NSString * caption = [[NSString alloc] initWithData:[HWMonitorEngine getValueDataFromSmcKeyInfo:info] encoding: NSUTF8StringEncoding];
                    
                    if ([caption hasPrefix:@"GPU "]) {
                        UInt8 cardIndex = [[caption substringFromIndex:4] intValue] - 1;
                        NSString *title = cardIndex == 0 ? GetLocalizedString(@"GPU Fan") : [NSString stringWithFormat:GetLocalizedString(@"GPU %X Fan"), cardIndex + 1];
                        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i] title:title group:kHWSensorGroupTachometer];
                    }
                }
                else if ([type isEqualToString:@TYPE_FDS]) {
                    FanTypeDescStruct *fds = (FanTypeDescStruct*)[[HWMonitorEngine getValueDataFromSmcKeyInfo:info] bytes];
                    
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
        }
        
        // Voltages
        [self addSensorsFromGroup:kHWSensorGroupVoltage withKeysList:list];
        
        // Currents
        [self addSensorsFromGroup:kHWSensorGroupCurrent withKeysList:list];
        
        // Powers
        [self addSensorsFromGroup:kHWSensorGroupPower withKeysList:list];
        
        // Batteries
        if ((_bluetoothDevices = [GenericBatteryDevice discoverDevices])) {
            for (GenericBatteryDevice * device in _bluetoothDevices) {
                [self addBluetoothSensorWithGenericDevice:device group:kBluetoothGroupBattery];
            }
        }

    }
    
    [_sensorsLock unlock];
}

- (NSArray*)updateSmartSensors
{
    [_sensorsLock lock];
    
    NSMutableArray *updated = [[NSMutableArray alloc] init];
    
    for (HWMonitorSensor *sensor in _sensors) {
        if ([sensor genericDevice] && [[sensor genericDevice] isKindOfClass:[ATAGenericDrive class]]) {
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
                [updated addObject:sensor];
            }
        }
    }
    
    [_sensorsLock unlock];
    
    return updated;
}

- (void)updateSensor:(HWMonitorSensor*)sensor addToArray:(NSMutableArray*)updated
{
    if (![sensor genericDevice]) {
        SMCVal_t val;
        UInt32Char_t name;
        
        strncpy(name, [[sensor name] cStringUsingEncoding:NSASCIIStringEncoding], 5);
        
        if (kIOReturnSuccess == SMCReadKey(_connection, name, &val)) {
            [sensor setType:[NSString stringWithCString:val.dataType encoding:NSASCIIStringEncoding]];
            [sensor setData:[NSData dataWithBytes:val.bytes length:val.dataSize]];
            
            if ([sensor valueHasBeenChanged]) {
                [updated addObject:sensor];
            }
        }
    }
    else if ([[sensor genericDevice] isKindOfClass:[GenericBatteryDevice class]]) {
        [sensor setData:[[sensor genericDevice] getBatteryLevel]];
        
        if ([sensor valueHasBeenChanged]) {
            [updated addObject:sensor];
        }
    }
}

- (NSArray*)updateSensors
{
    [_sensorsLock lock];
    
    NSMutableArray *updated = [[NSMutableArray alloc] init];
    
    if (_connection || kIOReturnSuccess == SMCOpen(&_connection)) {
        for (HWMonitorSensor *sensor in _sensors) {
            [self updateSensor:sensor addToArray:updated];
            
            [NSThread sleepForTimeInterval:1 / _sensors.count];
        }
    }
    else if (_connection) {
        SMCClose(_connection);
        _connection = 0;
    }
    
    [_sensorsLock unlock];
    
    return updated;
}

-(NSArray*)updateSensorsList:(NSArray *)sensors
{
    if (!sensors) return nil; // [self updateSmcSensors];
    
    [_sensorsLock lock];
    
    NSMutableArray *updated = [[NSMutableArray alloc] init];
    
    if (_connection || kIOReturnSuccess == SMCOpen(&_connection)) {
        for (id object in sensors) {
            if ([object isKindOfClass:[HWMonitorSensor class]] && [_sensors containsObject:object]) {
                [self updateSensor:object addToArray:updated];
            }
        }
    }
    
    [_sensorsLock unlock];
    
    return updated;
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
