//
//  HWMonitorEngine.m
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "HWMonitorEngine.h"

#include "FakeSMCDefinitions.h"

@implementation HWMonitorEngine

#define GetLocalizedString(key) \
[_bundle localizedStringForKey:(key) value:@"" table:nil]

@synthesize bundle = _bundle;

@synthesize sensors = _sensors;
@synthesize keys = _keys;

@synthesize useFahrenheit = _useFahrenheit;
@synthesize showBSDNames = _showBSDNames;

+ (HWMonitorEngine*)engineWithBundle:(NSBundle*)bundle;
{
    HWMonitorEngine *me = [[HWMonitorEngine alloc] init];
    
    if (me) {
        [me setBundle:bundle];
        [me rebuildSensorsList];
    }
    
    return me;
}

+ (NSArray*)populateInfoForKey:(NSString*)key
{
    NSArray * info = NULL;
    
    io_service_t service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (service) {
        if (kIOReturnSuccess == IORegistryEntrySetCFProperty(service, CFSTR(kFakeSMCDeviceUpdateKeyValue), (__bridge CFTypeRef)key)) {
            NSDictionary *values = (__bridge_transfer NSDictionary *)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
            
            if (values)
                info = [values objectForKey:key];
        }
        
        IOObjectRelease(service);
    }
    
    return info;
}

+ (NSString*)copyTypeFromKeyInfo:(NSArray*)info
{
    if (info && [info count] == 2) 
        return [NSString stringWithString:(NSString*)[info objectAtIndex:0]];
    
    return nil;
}

+ (NSData*)copyValueFromKeyInfo:(NSArray*)info
{
    if (info && [info count] == 2)
        return [NSData dataWithData:(NSData *)[info objectAtIndex:1]];
    
    return nil;
}

-(void)setShowBSDNames:(BOOL)showBSDNames
{
    if (_showBSDNames != showBSDNames) {
        _showBSDNames = showBSDNames;
        
        for (HWMonitorSensor *sensor in [self sensors])
            if ([sensor disk])
                [sensor setTitle:_showBSDNames ? [[sensor disk] bsdName] : [[[sensor disk] productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]]];
    }
}

-(BOOL)showBSDNames
{
    return _showBSDNames;
}

- (HWMonitorSensor*)addSensorWithKey:(NSString*)key title:(NSString*)title group:(NSUInteger)group
{
    HWMonitorSensor *sensor = nil;
    NSString *type = nil;
    NSData *value = nil;
    BOOL smartSensor = FALSE;
    
    switch (group) {
        case kSMARTSensorGroupTemperature:
        case kSMARTSensorGroupRemainingLife:
        case kSMARTSensorGroupRemainingBlocks:
            smartSensor = TRUE;
            break;
            
        default: {
            NSArray *info = [HWMonitorEngine populateInfoForKey:key];
            
            type = [HWMonitorEngine copyTypeFromKeyInfo:info];
            value = [HWMonitorEngine copyValueFromKeyInfo:info];
            
            if (!type || !value)
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
        case kSMARTSensorGroupTemperature:
            value = [disk getTemperature];

            UInt16 t = 0;
            
            [value getBytes:&t length:2];
            
            // Don't add sensor if value is insane
            if (t > 100) 
                return nil;
            
            break;
            
        case kSMARTSensorGroupRemainingLife:
            value = [disk getRemainingLife];
            break;
            
        case kSMARTSensorGroupRemainingBlocks:
            value = [disk getRemainingBlocks];
            break;
    }
    
    if (value) {
        HWMonitorSensor *sensor = [self addSensorWithKey:[NSString stringWithFormat:@"%@%lx", [disk serialNumber], group] title:_showBSDNames ? [disk bsdName] : [[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] group:group];
        
        [sensor setData:value];
        [sensor setDisk:disk];
        if ([disk isExceeded]) [sensor setLevel:kHWSensorLevelExceeded];
        
        return sensor;
    }
    
    return nil;
}

- (id)init;
{
    _service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (!_service) 
        return nil;
    
    _smartReporter = [NSATASmartReporter smartReporterByDiscoveringDrives];
    _sensors = [[NSMutableArray alloc] init];
    _keys = [[NSMutableDictionary alloc] init];
    _bundle = [NSBundle mainBundle];
    
    return self;
}

- (id)initWithBundle:(NSBundle*)mainBundle;
{
    _service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (!_service) 
        return nil;
    
    _smartReporter = [NSATASmartReporter smartReporterByDiscoveringDrives];
    _sensors = [[NSMutableArray alloc] init];
    _keys = [[NSMutableDictionary alloc] init];
    _bundle = mainBundle;
    
    return self;
}

- (void)dealloc
{
    if (_service) 
        IOObjectRelease(_service);
}

- (void)rebuildSensorsList
{
    [_sensors removeAllObjects];
    [_keys removeAllObjects];
    
    //Temperatures
    
    for (int i=0; i<0xA; i++)
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_CPU_DIODE_TEMPERATURE,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU Core %X"),i + 1] group:kHWSensorGroupTemperature];
    
    [self addSensorWithKey:@KEY_CPU_HEATSINK_TEMPERATURE title:GetLocalizedString(@"CPU Heatsink") group:kHWSensorGroupTemperature];
    [self addSensorWithKey:@KEY_CPU_PROXIMITY_TEMPERATURE title:GetLocalizedString(@"CPU Proximity") group:kHWSensorGroupTemperature];
    [self addSensorWithKey:@KEY_NORTHBRIDGE_TEMPERATURE title:GetLocalizedString(@"Northbridge") group:kHWSensorGroupTemperature];
    [self addSensorWithKey:@KEY_PCH_DIE_TEMPERATURE title:GetLocalizedString(@"Platform Controller Hub") group:kHWSensorGroupTemperature];
    [self addSensorWithKey:@KEY_AMBIENT_TEMPERATURE title:GetLocalizedString(@"Ambient") group:kHWSensorGroupTemperature];
    
    for (int i=0; i<0xA; i++) {
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_DIODE_TEMPERATURE,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Diode"),i + 1] group:kHWSensorGroupTemperature];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_HEATSINK_TEMPERATURE,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Heatsink"),i + 1] group:kHWSensorGroupTemperature];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X"),i + 1] group:kHWSensorGroupTemperature];
    }
    
    if ([_smartReporter drives]) {
        for (int i = 0; i < [[_smartReporter drives] count]; i++) {
            ATAGenericDisk * disk = [[_smartReporter drives] objectAtIndex:i];
            
            if (disk) { 
                // Hard Drive Temperatures
                [self addSMARTSensorWithGenericDisk:disk group:kSMARTSensorGroupTemperature];
                
                if (![disk isRotational]) {
                    // SSD Remaining Life
                    [self addSMARTSensorWithGenericDisk:disk group:kSMARTSensorGroupRemainingLife];
                    // SSD Remaining Blocks
                    [self addSMARTSensorWithGenericDisk:disk group:kSMARTSensorGroupRemainingBlocks];
                }
            }
        }
    }
    
    //Frequencies
    for (int i=0; i<0xA; i++) {
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FAKESMC_FORMAT_CPU_MULTIPLIER,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU Core %X Multiplier"),i + 1] group:kHWSensorGroupMultiplier];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FAKESMC_FORMAT_CPU_FREQUENCY,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU Core %X"),i + 1] group:kHWSensorGroupFrequency];
    }
    
    [self addSensorWithKey:@KEY_FAKESMC_CPU_PACKAGE_MULTIPLIER title:GetLocalizedString(@"CPU Package Multiplier") group:kHWSensorGroupMultiplier];
    [self addSensorWithKey:@KEY_FAKESMC_CPU_PACKAGE_FREQUENCY title:GetLocalizedString(@"CPU Package") group:kHWSensorGroupFrequency];
    
    for (int i=0; i<0xA; i++) {
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FAKESMC_FORMAT_GPU_FREQUENCY,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Core"),i + 1] group:kHWSensorGroupFrequency];
        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FAKESMC_FORMAT_GPU_SHADER_FREQUENCY,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Shaders"),i + 1] group:kHWSensorGroupFrequency];
        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FAKESMC_FORMAT_GPU_ROP_FREQUENCY,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X ROP"),i + 1] group:kHWSensorGroupFrequency];
        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FAKESMC_FORMAT_GPU_MEMORY_FREQUENCY,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Memory"),i + 1] group:kHWSensorGroupFrequency];
    }
    
    // Fans
    for (int i=0; i<10; i++) {
        NSString * caption = [[NSString alloc] initWithData:[HWMonitorEngine copyValueFromKeyInfo:[HWMonitorEngine populateInfoForKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_ID,i]]] encoding: NSUTF8StringEncoding];
        
        if ([caption length] == 0)
            caption = [[NSString alloc] initWithFormat:@"Fan %X",i + 1];
        
        if (![caption hasPrefix:@"GPU "])
            [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i] title:GetLocalizedString(caption) group:kHWSensorGroupTachometer];
    }
    
    // GPU Fans
    for (int i=0; i<10; i++) {
        NSString * caption = [[NSString alloc] initWithData:[HWMonitorEngine copyValueFromKeyInfo:[HWMonitorEngine populateInfoForKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_ID,i]]] encoding: NSUTF8StringEncoding];
        
        if ([caption hasPrefix:@"GPU "]) {
            UInt8 cardIndex = [[caption substringFromIndex:5] intValue];
            
            [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FAKESMC_FORMAT_FAN_PWM,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X PWM"),i + 1] group:kHWSensorGroupPWM];
            
            [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X"),cardIndex + 1] group:kHWSensorGroupTachometer];
        }
    }
    
    // Voltages
    for (int i = 0; i <= 0xf; i++)        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_CPU_VOLTAGE,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU %X"),i + 1] group:kHWSensorGroupVoltage];
    
    [self addSensorWithKey:@KEY_MEMORY_VOLTAGE title:GetLocalizedString(@"Memory") group:kHWSensorGroupVoltage];
    [self addSensorWithKey:@KEY_MAIN_3V3_VOLTAGE title:GetLocalizedString(@"Main 3.3V") group:kHWSensorGroupVoltage];
    [self addSensorWithKey:@KEY_AUXILIARY_3V3V_VOLTAGE title:GetLocalizedString(@"Auxiliary 3.3V") group:kHWSensorGroupVoltage];
    [self addSensorWithKey:@KEY_MAIN_5V_VOLTAGE title:GetLocalizedString(@"Main 5V") group:kHWSensorGroupVoltage];
    [self addSensorWithKey:@KEY_STANDBY_5V_VOLTAGE title:GetLocalizedString(@"Standby 5V") group:kHWSensorGroupVoltage];
    [self addSensorWithKey:@KEY_MAIN_12V_VOLTAGE title:GetLocalizedString(@"Main 12V") group:kHWSensorGroupVoltage];
    [self addSensorWithKey:@KEY_PCIE_12V_VOLTAGE title:GetLocalizedString(@"PCIe 12V") group:kHWSensorGroupVoltage];
    [self addSensorWithKey:@KEY_POWERBATTERY_VOLTAGE title:GetLocalizedString(@"Power/Battery") group:kHWSensorGroupVoltage];
    
    for (int i = 0; i <= 0xf; i++)        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"VRM Supply %X"),i + 1] group:kHWSensorGroupVoltage];
    
    for (int i = 0; i <= 0xf; i++)        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_POWERSUPPLY_VOLTAGE,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"Power Supply %X"),i + 1] group:kHWSensorGroupVoltage];
    
    for (int i = 0; i <= 0xf; i++)        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_VOLTAGE,i] title:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X"),i + 1] group:kHWSensorGroupVoltage];
}

- (NSArray*)updateSMARTSensorsValues
{
    NSMutableArray *list = [[NSMutableArray alloc] init];
    
    for (HWMonitorSensor *sensor in [self sensors]) {
        if ([sensor disk]) {
            switch ([sensor group]) {
                case kSMARTSensorGroupTemperature:
                    [sensor setData:[[sensor disk] getTemperature]];
                    [list addObject:sensor];
                    break;
                    
                case kSMARTSensorGroupRemainingLife:
                    [sensor setData:[[sensor disk] getRemainingLife]];
                    [list addObject:sensor];
                    break;
                    
                case kSMARTSensorGroupRemainingBlocks:
                    [sensor setData:[[sensor disk] getRemainingBlocks]];
                    [list addObject:sensor];
                    break;
                        
                default:
                    break;
            }
        }
    }
    
    return list;
}

- (NSArray*)updateGenericSensorsValues
{
    NSMutableArray *namesList = [[NSMutableArray alloc] init];
    NSMutableArray *sensorsList = [[NSMutableArray alloc] init];
    
    for (HWMonitorSensor *sensor in [self sensors])
        if (![sensor disk]) {
            [namesList addObject:[sensor name]];
            [sensorsList addObject:sensor];
        }
    
    if (kIOReturnSuccess == IORegistryEntrySetCFProperty(_service, CFSTR(kFakeSMCDevicePopulateValues), (__bridge CFTypeRef)namesList))
    {           
        NSDictionary *values = nil;
        
        if ((values = (__bridge_transfer NSDictionary*)IORegistryEntryCreateCFProperty(_service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0))) {
            
            for (NSString *key in [values allKeys]) {
                
                HWMonitorSensor *sensor = [_keys objectForKey:key];
                
                if (sensor) {
                    NSArray *keyInfo = [values objectForKey:key];
                    
                    [sensor setType:[HWMonitorEngine copyTypeFromKeyInfo:keyInfo]];
                    [sensor setData:[HWMonitorEngine copyValueFromKeyInfo:keyInfo]];
                }
            }
        }
    }
    
    return sensorsList;
}

-(NSArray*)updateFavoritesSensorsValues:(NSArray *)favorites
{
    NSMutableArray *nameslist = [[NSMutableArray alloc] init];
    NSMutableArray *sensorsList = [[NSMutableArray alloc] init];
    
    for (id object in favorites)
        if ([object isKindOfClass:[HWMonitorSensor class]] && [[self sensors] containsObject:object] && ![object disk]) {
            [nameslist addObject:[object name]];
            [sensorsList addObject:object];
        }
    
    if (kIOReturnSuccess == IORegistryEntrySetCFProperty(_service, CFSTR(kFakeSMCDevicePopulateValues), (__bridge CFTypeRef)nameslist)) 
    {           
        NSDictionary *values = nil;
        
        if ((values = (__bridge_transfer NSDictionary*)IORegistryEntryCreateCFProperty(_service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0))) {
            
            for (NSString *key in [values allKeys]) {
                
                HWMonitorSensor *sensor = [_keys objectForKey:key];
                
                if (sensor) {
                    NSArray *keyInfo = [values objectForKey:key];
                    
                    [sensor setType:[HWMonitorEngine copyTypeFromKeyInfo:keyInfo]];
                    [sensor setData:[HWMonitorEngine copyValueFromKeyInfo:keyInfo]];
                }
            }
        }
    }
    
    return sensorsList;
}

- (NSArray*)getAllSensorsInGroup:(NSUInteger)group
{
    NSMutableArray * list = [[NSMutableArray alloc] init];
    
    for (HWMonitorSensor *sensor in [self sensors])
        if (group & [sensor group])
            [list addObject:sensor];
    
    return [list count] > 0 ? [NSArray arrayWithArray:list] : nil;
}

@end
