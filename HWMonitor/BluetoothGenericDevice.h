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
    kBluetoothDeviceTypeKeyboard,
    kBluetoothDeviceTypeTrackpad
} BluetoothDeviceType;

@interface BluetoothGenericDevice : NSObject

@property (nonatomic, assign) io_service_t service;
@property (nonatomic, assign) BluetoothDeviceType deviceType;
@property (nonatomic, strong) NSString *productName;

+ (NSArray*)discoverDevices;

+ (BluetoothGenericDevice*)bluetoothGenericDeviceWithService:(io_service_t)service ofType:(BluetoothDeviceType)type;

- (NSData*)getBatteryLevel;

@end
