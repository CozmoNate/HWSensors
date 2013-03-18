//
//  BluetoothReporter.m
//  HWMonitor
//
//  Created by kozlek on 12.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "BluetoothGenericDevice.h"

@implementation BluetoothGenericDevice

+ (NSArray*)discoverDevices
{
    NSMutableArray *devices = [[NSMutableArray alloc] init];
    
    for (int index = 0; index < 3; index++) {
        
        BluetoothDeviceType type = kBluetoothDeviceTypeNone;
        CFDictionaryRef matching = MACH_PORT_NULL;
        
        switch (index) {
            case 0:
                matching = IOServiceMatching("BNBMouseDevice");
                type = kBluetoothDeviceTypeMouse;
                break;
                
            case 1:
                matching = IOServiceMatching("AppleBluetoothHIDKeyboard");
                type = kBluetoothDeviceTypeKeyboard;
                break;
                
            case 2:
                matching = IOServiceMatching("BNBTrackpadDevice");
                type = kBluetoothDeviceTypeTrackpad;
                break;
                
            default:
                break;
        }

        if (MACH_PORT_NULL != matching) {
            io_iterator_t iterator = IO_OBJECT_NULL;
            
            if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
                if (IO_OBJECT_NULL != iterator) {
                    
                    io_service_t service = MACH_PORT_NULL;
                    
                    while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                        BluetoothGenericDevice* device = [BluetoothGenericDevice bluetoothGenericDeviceWithService:service ofType:type];
                        
                        if (device) {
                            [devices addObject:device];
                        }
                    }
                    
                    IOObjectRelease(iterator);
                }
            }
        }
    }
    
    return devices;
}

+ (BluetoothGenericDevice*)bluetoothGenericDeviceWithService:(io_service_t)service ofType:(BluetoothDeviceType)type;
{
    BluetoothGenericDevice *me = [[BluetoothGenericDevice alloc] init];
    
    if (me) {
        [me setService:service];
        [me setDeviceType:type];
        
        NSData *level = [me getBatteryLevel];
        
        if (level == nil) {
            return nil;
        }
        
        [me setProductName:(__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("Product"), kCFAllocatorDefault, 0)];
        [me setSerialNumber:(__bridge_transfer NSString *)IORegistryEntryCreateCFProperty(service, CFSTR("SerialNumber"), kCFAllocatorDefault, 0)];
    }
    
    return me;
}

-(void)dealloc
{
    if (MACH_PORT_NULL != _service) {
        IOObjectRelease(_service);
    }
}

- (NSData*)getBatteryLevel
{
    NSData *result = nil;
    
    if (MACH_PORT_NULL != _service) {
        NSNumber *level = (__bridge_transfer  NSNumber *)IORegistryEntryCreateCFProperty(_service, CFSTR("BatteryPercent"), kCFAllocatorDefault, 0);
        
        if (level) {
            NSUInteger bytes = [level unsignedIntegerValue];
            result = [NSData dataWithBytes:&bytes length:sizeof(NSUInteger)];
        }
    }
    
    return result;
}

@end
