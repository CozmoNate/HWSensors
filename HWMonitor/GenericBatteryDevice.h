//
//  BluetoothReporter.h
//  HWMonitor
//
//  Created by kozlek on 12.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

typedef enum
{
    kGenericBatteryDeviceTypeNone,
    kPrimaryBatteryTypeInternal,
    kBluetoothBatteryTypeMouse,
    kBluetoothBatteryTypeKeyboard,
    kBluetoothBatteryTypeTrackpad,
} GenericBatteryDeviceType;

@interface GenericBatteryDevice : NSObject

@property (nonatomic, assign) io_service_t service;
@property (nonatomic, assign) GenericBatteryDeviceType deviceType;
@property (nonatomic, strong) NSString *productName;
@property (nonatomic, strong) NSString *serialNumber;

+ (NSArray*)discoverDevices;

+ (GenericBatteryDevice*)genericBatteryDeviceWithService:(io_service_t)service ofType:(GenericBatteryDeviceType)type;

- (NSData*)getBatteryLevel;

@end
