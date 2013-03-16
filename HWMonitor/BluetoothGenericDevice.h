//
//  BluetoothReporter.h
//  HWMonitor
//
//  Created by kozlek on 12.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum
{
    kBluetoothDeviceTypeNone,
    kBluetoothDeviceTypeMouse,
    kBluetoothDeviceTypeKeyboard
} BluetoothDeviceType;

@interface BluetoothGenericDevice : NSObject

@property (readonly) BluetoothDeviceType deviceType;

+ (BluetoothGenericDevice*)bluetoothGenericDeviceByDiscoveringDeviceType:(BluetoothDeviceType)type;

- (NSData*)getBatteryLevel;
- (void)discoverDeviceType:(BluetoothDeviceType)type;

@end
