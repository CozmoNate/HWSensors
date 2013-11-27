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

@property (nonatomic, retain) NSString * productName;
@property (nonatomic, retain) NSString * bsdName;
@property (nonatomic, retain) NSString * volumeNames;
@property (nonatomic, retain) NSString * serialNumber;
@property (nonatomic, retain) NSNumber * rotational;
@property (nonatomic, retain) NSNumber * exceeded;

+(NSArray*)discoverDrives;

-(void)doUpdateValue;

@end
