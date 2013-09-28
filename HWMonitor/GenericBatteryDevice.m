//
//  BluetoothReporter.m
//  HWMonitor
//
//  Created by kozlek on 12.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "GenericBatteryDevice.h"

@implementation GenericBatteryDevice

+ (NSArray*)discoverDevices
{
    NSMutableArray *devices = [[NSMutableArray alloc] init];
    BatteryDeviceType type;
    int count = 0;

    do {
        CFDictionaryRef matching;
        
        switch (count++) {
            case 0:
                //                matching = IOServiceMatching("AppleSmartBattery");
                matching = IOServiceMatching("IOPMPowerSource");
                type = kInternalBatteryType;
                break;

            case 1:
                matching = IOServiceMatching("BNBMouseDevice");
                type = kBluetoothDeviceTypeMouse;
                break;
                
            case 2:
                matching = IOServiceMatching("AppleBluetoothHIDKeyboard");
                type = kBluetoothDeviceTypeKeyboard;
                break;
                
            case 3:
                matching = IOServiceMatching("BNBTrackpadDevice");
                type = kBluetoothDeviceTypeTrackpad;
                break;
                
            default:
                matching = MACH_PORT_NULL;
                type = kGenericBatteryDeviceTypeNone;
                break;
        }

        if (MACH_PORT_NULL != matching) {
            
            io_iterator_t iterator = IO_OBJECT_NULL;
            
            if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
                if (IO_OBJECT_NULL != iterator) {
                    
                    io_service_t service = MACH_PORT_NULL;
                    
                    while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                        GenericBatteryDevice* device = [GenericBatteryDevice genericBatteryDeviceWithService:service ofType:type];
                        
                        if (device) {
                            [devices addObject:device];
                        }
                    }
                    
                    IOObjectRelease(iterator);
                }
            }
        }
    } while (type != kGenericBatteryDeviceTypeNone);
    
    
    return devices;
}

+ (GenericBatteryDevice*)genericBatteryDeviceWithService:(io_service_t)service ofType:(BatteryDeviceType)type;
{
    return [[GenericBatteryDevice alloc] initWithService:service ofType:type];
}

- (void)dealloc
{
    if (MACH_PORT_NULL != _service) {
        IOObjectRelease(_service);
    }
}

- (GenericBatteryDevice*)initWithService:(io_service_t)service ofType:(BatteryDeviceType)type
{
    self = [super init];
    
    if (self) {
        _service = service;
        _deviceType = type;
        
        if (![self getBatteryLevel]) {
            return nil;
        }
        
        switch (type) {
            case kInternalBatteryType:
                _productName = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("DeviceName"), kCFAllocatorDefault, 0);
                _serialNumber = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("BatterySerialNumber"), kCFAllocatorDefault, 0);
                
                if (!_serialNumber)
                    _serialNumber = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("SerialNumber"), kCFAllocatorDefault, 0);
                break;
                
            default:
                _productName = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("Product"), kCFAllocatorDefault, 0);
                _serialNumber = (__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("SerialNumber"), kCFAllocatorDefault, 0);
                break;
        }
    }
    
    return self;
}

- (NSData*)getBatteryLevel
{
    NSData *result = nil;
    
    if (MACH_PORT_NULL != _service) {
        NSNumber *level = nil;
        
        switch (_deviceType) {
            case kInternalBatteryType: {
                NSNumber *max = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(_service, CFSTR("MaxCapacity"), kCFAllocatorDefault, 0);
                NSNumber *current = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(_service, CFSTR("CurrentCapacity"), kCFAllocatorDefault, 0);
                
                if (max && current && [max doubleValue] > 0) {
                    double percent = (([current doubleValue] / [max doubleValue]) + 0.005) * 100;
                    level = [NSNumber numberWithDouble:percent];
                }
                
                break;
            }
                
            default:
                level = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(_service, CFSTR("BatteryPercent"), kCFAllocatorDefault, 0);
                break;
        }
        
        if (level) {
            NSUInteger bytes = [level unsignedIntegerValue];
            result = [NSData dataWithBytes:&bytes length:sizeof(NSUInteger)];
        }
    }
    
    return result;
}

@end
