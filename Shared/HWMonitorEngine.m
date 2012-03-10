//
//  NSHWMonitor.m
//  HWSensors
//
//  Created by kozlek on 23/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#import "HWMonitorEngine.h"

#include "FakeSMCDefinitions.h"

@implementation HWMonitorEngine

#define GetLocalizedString(key) \
[bundle localizedStringForKey:(key) value:@"" table:nil]

@synthesize sensors;

+ (HWMonitorEngine*)hardwareMonitor
{
    HWMonitorEngine *me = [[HWMonitorEngine alloc] init];
    
    if (me) {
        [me rebuildSensorsList];
    }
    
    return me;
}

+ (NSArray*)populateInfoForKey:(NSString*)key
{
    NSArray * info = NULL;
    
    io_service_t service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (service) {
        CFTypeRef message = (CFTypeRef) CFStringCreateWithCString(kCFAllocatorDefault, [key cStringUsingEncoding:NSASCIIStringEncoding], kCFStringEncodingASCII);
        
        if (kIOReturnSuccess == IORegistryEntrySetCFProperty(service, CFSTR(kFakeSMCDeviceUpdateKeyValue), message)) 
        {
            NSDictionary *values = (__bridge_transfer NSDictionary *)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
            
            if (values)
                info = [values objectForKey:key];
        }
        
        CFRelease(message);
        IOObjectRelease(service);
    }
    
    return info;
}

+ (NSString*)getTypeFromKeyInfo:(NSArray*)info
{
    if (info && [info count] == 2) 
        return (NSString*)[info objectAtIndex:0];
        
    return nil;
}

+ (NSData*)getValueFromKeyInfo:(NSArray*)info
{
    if (info && [info count] == 2)
        return (NSData*)[info objectAtIndex:1];
    
    return nil;
}

- (HWMonitorSensor*)addSensorWithKey:(NSString*)key caption:(NSString*)caption group:(NSUInteger)group
{
    HWMonitorSensor *sensor = nil;
    NSString *type = nil;
    NSData *value = nil;
    
    switch (group) {
        case kHWSMARTTemperatureGroup:
            
            break;
        case kHWSMARTRemainingLifeGroup:
            
            break;
            
        default: {
            NSArray *info = [HWMonitorEngine populateInfoForKey:key];
            
            type = [HWMonitorEngine getTypeFromKeyInfo:info];
            value = [HWMonitorEngine getValueFromKeyInfo:info];
            
            if (!type || !value)
                return nil;
            
            break;
        }
    }
    
    sensor = [HWMonitorSensor sensor];
    
    [sensor setKey:key];
    [sensor setType:type];
    [sensor setCaption:caption];
    [sensor setValue:value];
    [sensor setGroup:group];
    
    [sensors addObject:sensor];
    [keys setObject:sensor forKey:key];
    
    return sensor;
}

- (HWMonitorSensor*)addSMARTSensorWithGenericDisk:(ATAGenericDisk*)disk group:(NSUInteger)group
{
    NSData * value = nil;
    
    switch (group) {
        case kHWSMARTTemperatureGroup:
            value = [disk getTemperature];
            if (value) {
                HWMonitorSensor *sensor = [self addSensorWithKey:[disk serialNumber] caption:[[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] group:kHWSMARTTemperatureGroup];
                [sensor setValue:value];
                [sensor setDisk:disk];
            }
            break;
            
        case kHWSMARTRemainingLifeGroup:
            value = [disk getRemainingLife];
            if (value) {
                HWMonitorSensor *sensor = [self addSensorWithKey:[disk serialNumber] caption:[[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] group:kHWSMARTRemainingLifeGroup];
                [sensor setValue:value];
                [sensor setDisk:disk];
            }
            break;
            
        default:           
            break;
    }
    
    return nil;
}

- (id)init;
{
    service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (!service) 
        return nil;
    
    smartReporter = [NSATASmartReporter smartReporterByDiscoveringDrives];
    sensors = [NSMutableArray array];
    keys = [NSMutableDictionary dictionary];
    bundle = [NSBundle mainBundle];
    
    return self;
}

- (id)initWithBundle:(NSBundle*)mainBundle;
{
    service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (!service) 
        return nil;
    
    smartReporter = [NSATASmartReporter smartReporterByDiscoveringDrives];
    sensors = [NSMutableArray array];
    keys = [NSMutableDictionary dictionary];
    bundle = mainBundle;
    
    return self;
}

- (void)dealloc
{
    if (service) 
        IOObjectRelease(service);
}

- (void)rebuildSensorsList
{
    [sensors removeAllObjects];
    
    //Temperatures
    
    for (int i=0; i<0xA; i++)
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_CPU_DIODE_TEMPERATURE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU Core %X"),i + 1] group:kHWTemperatureGroup];
    
    [self addSensorWithKey:@KEY_CPU_HEATSINK_TEMPERATURE caption:GetLocalizedString(@"CPU Heatsink") group:kHWTemperatureGroup];
    //[self addSensorWithKey:@KEY_NORTHBRIDGE_TEMPERATURE caption:GetLocalizedString(@"Northbridge") group:kHWTemperatureGroup];
    [self addSensorWithKey:@KEY_NORTHBRIDGE_TEMPERATURE caption:GetLocalizedString(@"System Chipset") group:kHWTemperatureGroup];
    [self addSensorWithKey:@KEY_AMBIENT_TEMPERATURE caption:GetLocalizedString(@"Ambient") group:kHWTemperatureGroup];
    
    for (int i=0; i<0xA; i++) {
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_DIODE_TEMPERATURE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Core"),i + 1] group:kHWTemperatureGroup];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_BOARD_TEMPERATURE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Board"),i + 1] group:kHWTemperatureGroup];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Proximity"),i + 1] group:kHWTemperatureGroup];
    }
    
    if ([smartReporter drives]) {
        
        // Hard Drive Temperatures
        
        for (int i = 0; i < [[smartReporter drives] count]; i++) {
            ATAGenericDisk * disk = [[smartReporter drives] objectAtIndex:i];
            
            if (disk) 
                [self addSMARTSensorWithGenericDisk:disk group:kHWSMARTTemperatureGroup];
        }
    
        // SSD Remaining Life
        
        for (int i = 0; i < [[smartReporter drives] count]; i++) {
            ATAGenericDisk * disk = [[smartReporter drives] objectAtIndex:i];
            
            if (disk && ![disk isRotational]) 
                [self addSMARTSensorWithGenericDisk:disk group:kHWSMARTRemainingLifeGroup];
        }
    }
    
    
    //Multipliers
    
    for (int i=0; i<0xA; i++)
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_NON_APPLE_CPU_MULTIPLIER,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU Core %X"),i + 1] group:kHWMultiplierGroup];
    
    [self addSensorWithKey:@KEY_NON_APPLE_CPU_PACKAGE_MULTIPLIER caption:GetLocalizedString(@"CPU Package") group:kHWMultiplierGroup];
    
    // Fans
    
    for (int i=0; i<10; i++) {
        NSString * caption = [[NSString alloc] initWithData:[HWMonitorEngine getValueFromKeyInfo:[HWMonitorEngine populateInfoForKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_ID,i]]] encoding: NSUTF8StringEncoding];
        if ([caption length]<=0) 
            caption = [[NSString alloc] initWithFormat:GetLocalizedString(@"Fan %X"),i + 1];
        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i] caption:caption group:kHWTachometerGroup];
    }
    
    // Voltages
    for (int i = 0; i <= 0xf; i++)        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_CPU_VOLTAGE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU %X"),i + 1] group:kHWVoltageGroup];
    
    [self addSensorWithKey:@KEY_MEMORY_VOLTAGE caption:GetLocalizedString(@"DIMM Modules") group:kHWVoltageGroup];
    [self addSensorWithKey:@KEY_DCIN_12V_S0_VOLTAGE caption:GetLocalizedString(@"+12V") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_DCIN_3V3_S5_VOLTAGE caption:GetLocalizedString(@"AVCC") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_POWERBATTERY_VOLTAGE caption:GetLocalizedString(@"Power/Battery") group:kHWVoltageGroup ];
    
    for (int i = 0; i <= 0xf; i++)        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_CPU_VRMSUPPLY_VOLTAGE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"VRM Supply %X"),i + 1] group:kHWVoltageGroup];
    
    for (int i = 0; i <= 0xf; i++)        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_POWERSUPPLY_VOLTAGE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"Power Supply %X"),i + 1] group:kHWVoltageGroup];
}

- (void)updateSMARTSensorsValues
{
    if (sensors) 
        for (int i = 0; i < [sensors count]; i++) {
            HWMonitorSensor *sensor = [sensors objectAtIndex:i];
            
            switch ([sensor group]) {
                case kHWSMARTTemperatureGroup:
                    if ([sensor disk]) [sensor setValue:[[sensor disk] getTemperature]];                    
                    break;

                case kHWSMARTRemainingLifeGroup: {
                    if ([sensor disk]) [sensor setValue:[[sensor disk] getRemainingLife]];                   
                    break;
                }
                    
                default:
                    break;
            }
        }
}

- (void)updateGenericSensorsValuesButOnlyFavorits:(BOOL)updateOnlyFavorites
{
    if (sensors) {
        NSMutableArray *list = [[NSMutableArray alloc] init];
        
        for (int i = 0; i < [sensors count]; i++) {
            HWMonitorSensor *sensor = [sensors objectAtIndex:i];
            
            if ((updateOnlyFavorites && [sensor favorite]) || !updateOnlyFavorites)
                switch ([sensor group]) {
                    case kHWSMARTTemperatureGroup:
                    case kHWSMARTRemainingLifeGroup:
                        break;
                        
                    default: {
                        [list addObject:[sensor key]];
                        break;
                    }
                }
        }
        
        if (kIOReturnSuccess == IORegistryEntrySetCFProperty(service, CFSTR(kFakeSMCDevicePopulateValues), (__bridge CFTypeRef)list)) 
        {           
            NSDictionary *values = (__bridge_transfer NSDictionary*)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
            
            if (values) {
                NSEnumerator *enumerator = [values keyEnumerator];
                
                NSString *key = nil;
                
                while (key = (NSString*)[enumerator nextObject]) {
                    HWMonitorSensor *sensor = [keys objectForKey:key];
                    
                    if (sensor) {
                        NSArray *keyInfo = [values objectForKey:key];
                        
                        [sensor setType:[HWMonitorEngine getTypeFromKeyInfo:keyInfo]];
                        [sensor setValue:[HWMonitorEngine getValueFromKeyInfo:keyInfo]];
                    }
                }
            }
        }
    }
}

- (NSArray*)getAllSensorsInGroup:(NSUInteger)group
{
    NSMutableArray * list = [[NSMutableArray alloc] init];
    
    for (int i = 0; i < [sensors count]; i++) {
        
        HWMonitorSensor *sensor = (HWMonitorSensor*)[sensors objectAtIndex:i];
        
        if ([sensor group] == group)
            [list addObject:sensor];
    }
    
    return [list count] > 0 ? [NSArray arrayWithArray:list] : nil;
}

@end
