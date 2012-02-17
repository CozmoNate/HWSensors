//
//  NSSensor.m
//  HWSensors
//
//  Created by mozo on 22.10.11.
//  Copyright (c) 2011 mozo. All rights reserved.
//

#import "HWMonitorSensor.h"

#include "FakeSMCDefinitions.h"

@implementation HWMonitorSensor

@synthesize key;
@synthesize group;
@synthesize caption;
@synthesize object;
@synthesize favorite;

+ (unsigned int)swapBytes:(unsigned int) value
{
    return ((value & 0xff00) >> 8) | ((value & 0xff) << 8);
}

+ (NSDictionary *)populateValues;
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

+ (NSData *)populateValueForKey:(NSString *)key
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

/*+ (NSData *) readValueForKey:(NSString *)key
{
    NSData * value = NULL;
    
    io_service_t service = IOServiceGetMatchingService(0, IOServiceMatching(kFakeSMCDeviceService));
    
    if (service) {
        NSDictionary * values = (__bridge_transfer NSDictionary *)IORegistryEntryCreateCFProperty(service, CFSTR(kFakeSMCDeviceValues), kCFAllocatorDefault, 0);
        
        if (values) 
            value = [values objectForKey:key];

        IOObjectRelease(service);
    }
    
    return value;
}*/

- (HWMonitorSensor *)initWithKey:(NSString *)aKey andGroup:(NSUInteger)aGroup withCaption:(NSString *)aCaption
{
    key = aKey;
    group = aGroup;
    caption = aCaption;
    
    return self;
}

- (NSString *) formateValue:(NSData *)value
{
    if (value != NULL) {
        switch (group) {
            case TemperatureSensorGroup:
            case SMARTTemperatureSensorGroup:
            {
                unsigned int t = 0;
                
                bcopy([value bytes], &t, 2);
                
                //t = [NSSensor swapBytes:t] >> 8;
                
                return [[NSString alloc] initWithFormat:@"%d°",t];
                
            } break;
                
            case VoltageSensorGroup:
            {
                unsigned int encoded = 0;
                
                bcopy([value bytes], &encoded, 2);
                
                encoded = [HWMonitorSensor swapBytes:encoded];
                
                float v = ((encoded & 0xc000) >> 14) + ((encoded & 0x3fff) >> 4) / 1000.0;
                
                return [[NSString alloc] initWithFormat:@"%1.3fV",v];
            } break;
                
            case TachometerSensorGroup:
            {
                unsigned int rpm = 0;
                
                bcopy([value bytes], &rpm, 2);
                
                rpm = [HWMonitorSensor swapBytes:rpm] >> 2;
                
                return [[NSString alloc] initWithFormat:@"%drpm",rpm];
                
            } break;
                
            case MultiplierSensorGroup:
            {
                unsigned int mlt = 0;
                
                bcopy([value bytes], &mlt, 2);
                
                return [[NSString alloc] initWithFormat:@"x%1.1f",(float)mlt / 10.0];
                
            } break;
        }
    }
    
    return [[NSString alloc] initWithString:@"-"];
}

@end
