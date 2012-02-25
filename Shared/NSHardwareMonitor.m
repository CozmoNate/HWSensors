//
//  NSHWMonitor.m
//  HWSensors
//
//  Created by Natan Zalkin on 23/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#import "NSHardwareMonitor.h"

#include "FakeSMCDefinitions.h"

@implementation NSHardwareMonitor

#define GetLocalizedString(key) \
[bundle localizedStringForKey:(key) value:@"" table:nil]

@synthesize sensors;

+ (NSHardwareMonitor*)hardwareMonitor
{
    NSHardwareMonitor *me = [[NSHardwareMonitor alloc] init];
    
    if (me) {
        [me rebuildSensorsList];
    }
    
    return me;
}

+ (NSDictionary*)populateValues;
{
    NSDictionary * values = NULL;
    
    io_service_t service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (service) { 
        CFTypeRef message = (CFTypeRef) CFStringCreateWithCString(kCFAllocatorDefault, "magic", kCFStringEncodingASCII);
        
        if (kIOReturnSuccess == IORegistryEntrySetCFProperty(service, CFSTR(kFakeSMCDevicePopulateValues), message))
            values = (__bridge_transfer NSDictionary *)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
        
        CFRelease(message);
        IOObjectRelease(service);
    }
    
    return values;
    
}

+ (NSData*)populateValueForKey:(NSString*)key
{
    NSData * value = NULL;
    
    io_service_t service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (service) {
        CFTypeRef message = (CFTypeRef) CFStringCreateWithCString(kCFAllocatorDefault, [key cStringUsingEncoding:NSASCIIStringEncoding], kCFStringEncodingASCII);
        
        if (kIOReturnSuccess == IORegistryEntrySetCFProperty(service, CFSTR(kFakeSMCDeviceUpdateKeyValue), message)) 
        {
            NSDictionary * values = (__bridge_transfer NSDictionary *)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
            
            if (values)
                value = [values objectForKey:key];
        }
        
        CFRelease(message);
        IOObjectRelease(service);
    }
    
    return value;
}

- (NSHardwareMonitorSensor*)addSensorWithKey:(NSString*)key caption:(NSString*)caption group:(NSUInteger)group
{
    NSHardwareMonitorSensor *sensor = nil;
    NSData * value = nil;
    
    switch (group) {
        case kHWSMARTTemperatureGroup:
            
            break;
        case kHWSMARTRemainingLifeGroup:
            
            break;
            
        default:
            value = [NSHardwareMonitor populateValueForKey:key];
            
            if (!value)
                return nil;
            
            break;
    }
    
    sensor = [NSHardwareMonitorSensor sensor];
    
    [sensor setKey:key];
    [sensor setCaption:caption];
    [sensor setValue:value];
    [sensor setGroup:group];
    
    [sensors addObject:sensor];
    [keys setObject:sensor forKey:key];
    
    return sensor;
}

- (NSHardwareMonitorSensor*)addSMARTSensorWithGenericDisk:(NSATAGenericDisk*)disk group:(NSUInteger)group
{
    NSData * value = nil;
    
    switch (group) {
        case kHWSMARTTemperatureGroup:
            value = [disk getTemperature];
            if (value) {
                NSHardwareMonitorSensor *sensor = [self addSensorWithKey:[disk serialNumber] caption:[[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] group:kHWSMARTTemperatureGroup];
                [sensor setValue:value];
                [sensor setDisk:disk];
            }
            break;
            
        case kHWSMARTRemainingLifeGroup:
            value = [disk getRemainingLife];
            if (value) {
                NSHardwareMonitorSensor *sensor = [self addSensorWithKey:[disk serialNumber] caption:[[disk productName] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] group:kHWSMARTRemainingLifeGroup];
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
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_CPU_DIODE_TEMPERATURE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU %X"),i] group:kHWTemperatureGroup];
    
    [self addSensorWithKey:@KEY_CPU_HEATSINK_TEMPERATURE caption:GetLocalizedString(@"CPU Heatsink") group:kHWTemperatureGroup];
    [self addSensorWithKey:@KEY_NORTHBRIDGE_TEMPERATURE caption:GetLocalizedString(@"Motherboard") group:kHWTemperatureGroup];
    [self addSensorWithKey:@KEY_AMBIENT_TEMPERATURE caption:GetLocalizedString(@"Ambient") group:kHWTemperatureGroup];
    
    for (int i=0; i<0xA; i++) {
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_DIODE_TEMPERATURE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Core"),i] group:kHWTemperatureGroup];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_BOARD_TEMPERATURE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Board"),i] group:kHWTemperatureGroup];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"GPU %X Proximity"),i] group:kHWTemperatureGroup];
    }
    
    if ([smartReporter drives]) {
        
        // Hard Drive Temperatures
        
        for (int i = 0; i < [[smartReporter drives] count]; i++) {
            NSATAGenericDisk * disk = [[smartReporter drives] objectAtIndex:i];
            
            if (disk) 
                [self addSMARTSensorWithGenericDisk:disk group:kHWSMARTTemperatureGroup];
        }
    
        // SSD Remaining Life
        
        for (int i = 0; i < [[smartReporter drives] count]; i++) {
            NSATAGenericDisk * disk = [[smartReporter drives] objectAtIndex:i];
            
            if (disk && ![disk isRotational]) 
                [self addSMARTSensorWithGenericDisk:disk group:kHWSMARTRemainingLifeGroup];
        }
    }
    
    
    //Multipliers
    
    for (int i=0; i<0xA; i++)
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_NON_APPLE_CPU_MULTIPLIER,i] caption:[[NSString alloc] initWithFormat:GetLocalizedString(@"CPU %X"),i] group:kHWMultiplierGroup];
    
    [self addSensorWithKey:@KEY_NON_APPLE_PACKAGE_MULTIPLIER caption:GetLocalizedString(@"CPU Package") group:kHWMultiplierGroup];
    
    // Fans
    
    for (int i=0; i<10; i++) {
        NSString * caption = [[NSString alloc] initWithData:[NSHardwareMonitor populateValueForKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_ID,i] ]encoding: NSUTF8StringEncoding];
        if ([caption length]<=0) 
            caption = [[NSString alloc] initWithFormat:GetLocalizedString(@"Fan %X"),i];
        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@KEY_FORMAT_FAN_SPEED,i] caption:caption group:kHWTachometerGroup];
    }
    
    // Voltages
    
    [self addSensorWithKey:@KEY_CPU_VOLTAGE caption:GetLocalizedString(@"CPU") group:kHWVoltageGroup];
    [self addSensorWithKey:@KEY_MEMORY_VOLTAGE caption:GetLocalizedString(@"DIMM Modules") group:kHWVoltageGroup];
    [self addSensorWithKey:@KEY_12V_VOLTAGE caption:GetLocalizedString(@"+12V") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_AVCC_VOLTAGE caption:GetLocalizedString(@"AVCC") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_3VCC_VOLTAGE caption:GetLocalizedString(@"+3.3V VCC") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_3VSB_VOLTAGE caption:GetLocalizedString(@"+3.3V VSB") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_VBAT_VOLTAGE caption:GetLocalizedString(@"Battery") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_CPU_VRM_SUPPLY0 caption:GetLocalizedString(@"VRM Supply 0") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_CPU_VRM_SUPPLY1 caption:GetLocalizedString(@"VRM Supply 1") group:kHWVoltageGroup ];
    [self addSensorWithKey:@KEY_CPU_VRM_SUPPLY2 caption:GetLocalizedString(@"VRM Supply 2") group:kHWVoltageGroup ];
}

- (void)updateSMARTSensorsValues
{
    if (sensors) 
        for (int i = 0; i < [sensors count]; i++) {
            NSHardwareMonitorSensor *sensor = [sensors objectAtIndex:i];
            
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
        CFMutableArrayRef list = (CFMutableArrayRef)CFArrayCreateMutable(kCFAllocatorDefault, 0, nil);
        
        for (int i = 0; i < [sensors count]; i++) {
            NSHardwareMonitorSensor *sensor = [sensors objectAtIndex:i];
            
            if ((updateOnlyFavorites && [sensor favorite]) || !updateOnlyFavorites)
                switch ([sensor group]) {
                    case kHWSMARTTemperatureGroup:
                    case kHWSMARTRemainingLifeGroup:
                        break;
                        
                    default: {
                        CFTypeRef name = (CFTypeRef)CFStringCreateWithCString(kCFAllocatorDefault, [[sensor key] cStringUsingEncoding:NSUTF8StringEncoding], kCFStringEncodingUTF8);
                        
                        CFArrayAppendValue(list, name);
                        
                        break;
                    }
                }
        }
        
        if (kIOReturnSuccess == IORegistryEntrySetCFProperty(service, CFSTR(kFakeSMCDevicePopulateList), list)) 
        {           
            NSDictionary *values = (__bridge_transfer NSDictionary*)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
            
            if (values) {
                NSEnumerator *enumerator = [values keyEnumerator];
                
                NSString *key = nil;
                
                while (key = (NSString*)[enumerator nextObject]) {
                    NSHardwareMonitorSensor *sensor = [keys objectForKey:key];
                    
                    if (sensor)
                        [sensor setValue:(NSData*)[values objectForKey:key]];
                }
            }
        }
        
        CFArrayRemoveAllValues(list);
        CFRelease(list);
    }
}

- (NSArray*)getAllSensorsInGroup:(NSUInteger)group
{
    NSMutableArray * list = [[NSMutableArray alloc] init];
    
    for (int i = 0; i < [sensors count]; i++) {
        
        NSHardwareMonitorSensor *sensor = (NSHardwareMonitorSensor*)[sensors objectAtIndex:i];
        
        if ([sensor group] == group)
            [list addObject:sensor];
    }
    
    return [list count] > 0 ? [NSArray arrayWithArray:list] : nil;
}

@end
