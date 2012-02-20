//
//  NSSmartReporter.h
//  HWSensors
//
//  Created by Natan Zalkin on 19/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <IOKit/storage/ata/ATASMARTLib.h>

#define kATASMARTVendorSpecificAttributesCount     30

#define kATASMARTAttributeTemperature               0xC2
#define kATASMARTAttributeTemperature2              0xE7

typedef struct ATASMARTAttribute
{
    UInt8 			attributeId;
    UInt16			flag;  
    UInt8 			current;
    UInt8 			worst;
    UInt8 			rawvalue[6];
    UInt8 			reserv;
}  __attribute__ ((packed)) ATASMARTAttribute;

typedef struct ATASMARTVendorSpecificData
{
    UInt16 					revisonNumber;
    ATASMARTAttribute		vendorAttributes [kATASMARTVendorSpecificAttributesCount];
} __attribute__ ((packed)) ATASmartVendorSpecificData;

@interface NSGenericDisk : NSObject
{
@private
    io_service_t service;
    struct ATASMARTVendorSpecificData data;
}
+(NSGenericDisk*)genericDiskWithService:(io_service_t) s;

-(BOOL)readSMARTData;
-(ATASMARTAttribute*)getSMARTAttributeByIdentifier:(UInt8) identifier;

@end

@interface NSSmartReporter : NSObject
{
@private
    NSDictionary * services;
    
@public
    
}

- (NSDictionary*)diskoverDrives;

@end
