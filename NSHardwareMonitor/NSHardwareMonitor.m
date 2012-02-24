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

- (id)init
{
    service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (!service) 
        return nil;
    
    smartReporter = [NSATASmartReporter smartReporterByDiscoveringDrives];
    sensors = [NSMutableArray array];
    keys = [NSMutableDictionary dictionary];
    
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
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"TC%XD",i] caption:[[NSString alloc] initWithFormat:@"CPU %X",i] group:kHWTemperatureGroup];
    
    [self addSensorWithKey:@"Th0H" caption:@"CPU Heatsink" group:kHWTemperatureGroup];
    [self addSensorWithKey:@"TN0P" caption:@"Motherboard" group:kHWTemperatureGroup];
    [self addSensorWithKey:@"TA0P" caption:@"Ambient" group:kHWTemperatureGroup];
    
    for (int i=0; i<0xA; i++) {
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"TG%XD",i] caption:[[NSString alloc] initWithFormat:@"GPU %X Core",i] group:kHWTemperatureGroup];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"TG%XH",i] caption:[[NSString alloc] initWithFormat:@"GPU %X Board",i] group:kHWTemperatureGroup];
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"TG%XP",i] caption:[[NSString alloc] initWithFormat:@"GPU %X Proximity",i] group:kHWTemperatureGroup];
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
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"MC%XC",i] caption:[[NSString alloc] initWithFormat:@"CPU %X",i] group:kHWMultiplierGroup];
    
    [self addSensorWithKey:@"MPkC" caption:@"CPU Package" group:kHWMultiplierGroup];
    
    // Fans
    
    for (int i=0; i<10; i++) {
        NSString * caption = [[NSString alloc] initWithData:[NSHardwareMonitor populateValueForKey:[[NSString alloc] initWithFormat:@"F%XID",i] ]encoding: NSUTF8StringEncoding];
        if ([caption length]<=0) 
            caption = [[NSString alloc] initWithFormat:@"Fan %d",i];
        
        [self addSensorWithKey:[[NSString alloc] initWithFormat:@"F%XAc",i] caption:caption group:kHWTachometerGroup];
    }
    
    // Voltages
    
    [self addSensorWithKey:@"VC0C" caption:@"CPU" group:kHWVoltageGroup];
    [self addSensorWithKey:@"VM0R" caption:@"DIMM Modules" group:kHWVoltageGroup];
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
