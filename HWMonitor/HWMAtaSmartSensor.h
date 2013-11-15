//
//  HWMAtaSmartSensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "HWMSensor.h"

#define kATASMARTVendorSpecificAttributesCount     30

#define kATASMARTAttributeTemperature               0xC2
#define kATASMARTAttributeTemperature2              0xE7
#define kATASMARTAttributeTemperature3              0xBE
#define kATASMARTAttributeEndurance                 0xE8
#define kATASMARTAttributeEndurance2                0xE7
#define kATASMARTAttributeUnusedReservedBloks       0xB4

#define kHWMAtaSmartSensorTypeNone                  0
#define kHWMAtaSmartSensorTypeTemperature           1
#define kHWMAtaSmartSensorTypeLife                  2
#define kHWMAtaSmartSensorTypeBlocks                3

typedef struct {
    UInt8 			attributeId;
    UInt16			flag;
    UInt8 			current;
    UInt8 			worst;
    UInt8 			rawvalue[6];
    UInt8 			reserv;
}  __attribute__ ((packed)) ATASMARTAttribute;

typedef struct {
    UInt16 					revisonNumber;
    ATASMARTAttribute		vendorAttributes [kATASMARTVendorSpecificAttributesCount];
} __attribute__ ((packed)) ATASmartVendorSpecificData;

@interface HWMAtaSmartSensor : HWMSensor
{
    BOOL exceeded;
    NSDate * updated;
    ATASmartVendorSpecificData _smartData;
}

@property (nonatomic, retain) NSNumber * selector;
@property (nonatomic, retain) NSNumber * service;

-(void)doUpdateValue;

@end
