//
//  BluetoothReporter.m
//  HWMonitor
//
//  Created by kozlek on 12.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "BluetoothGenericDevice.h"

@implementation BluetoothGenericDevice

+ (BluetoothGenericDevice*)bluetoothGenericDeviceByDiscoveringDeviceType:(BluetoothDeviceType)type
{
    BluetoothGenericDevice *me = [[BluetoothGenericDevice alloc] init];
    
    if (me)
        [me discoverDeviceType:type];
    
    return me;
}

- (NSData*)getBatteryLevel
{
    NSData *result = nil;
    
    CFDictionaryRef matching = MACH_PORT_NULL;
    
    switch (_deviceType) {
        case kBluetoothDeviceTypeMouse:
            matching = IOServiceMatching("BNBMouseDevice");
            break;
            
        case kBluetoothDeviceTypeKeyboard:{
            matching = IOServiceMatching("AppleBluetoothHIDKeyboard");
            break;
        }
            
        default:
            break;
    }

    if (MACH_PORT_NULL != matching) {
        io_iterator_t iterator = IO_OBJECT_NULL;
        
        if (kIOReturnSuccess == IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator)) {
            if (IO_OBJECT_NULL != iterator) {
                
                io_service_t service = MACH_PORT_NULL;
                
                while (MACH_PORT_NULL != (service = IOIteratorNext(iterator))) {
                    CFStringRef batteryLevel = (CFStringRef)IORegistryEntryCreateCFProperty(service, CFSTR("BatteryPercent"), kCFAllocatorDefault, 0);
                    
                    if (batteryLevel != IO_OBJECT_NULL) {
                        SInt32 bytes = CFStringGetIntValue(batteryLevel);
                        result = [NSData dataWithBytes:&bytes length:sizeof(SInt32)];
                        CFRelease(batteryLevel);
                    }
                    
                    IOObjectRelease(service);
                }
                
                IOObjectRelease(iterator);
            }
        }
    }
    
    return result;
}

- (void)discoverDeviceType:(BluetoothDeviceType)type
{
    switch (type) {
        case kBluetoothDeviceTypeMouse:
        case kBluetoothDeviceTypeKeyboard:
            _deviceType = type;
            
            if ([self getBatteryLevel] < 0)
                _deviceType = kBluetoothDeviceTypeNone;
            
            break;
            
        default:
            _deviceType = kBluetoothDeviceTypeNone;
            break;
    }
}

@end
