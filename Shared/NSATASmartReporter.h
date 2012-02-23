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

// NSATAGenericDisk

@interface NSATAGenericDisk : NSObject
{
@private
    io_service_t service;
    struct ATASMARTVendorSpecificData data;

    NSString    *productName;
    NSString    *serialNumber;
    BOOL        isRotational;
}

@property (readonly) NSString   *productName;
@property (readonly) NSString   *serialNumber;
@property (readonly) BOOL       isRotational;

+(NSATAGenericDisk*)genericDiskWithService:(io_service_t)ioservice productName:(NSString*)name serialNumber:(NSString*)serial isRotational:(BOOL)rotational;

-(BOOL)readSMARTData;
-(ATASMARTAttribute*)getSMARTAttributeByIdentifier:(UInt8)identifier;

@end

// NSATASmartReporter

@interface NSATASmartReporter : NSObject
{
@private
    NSDictionary *drives;
    
@public
}

@property (readonly) NSDictionary *drives;

+(NSATASmartReporter*)smartReporterByDiscoveringDrives;

-(void)diskoverDrives;

@end
