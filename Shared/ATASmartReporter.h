//
//  NSSmartReporter.h
//  HWSensors
//
//  Based on code by OldNavi
//
//  Created by kozlek on 19/02/12.
//  Copyright (c) 2012 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <IOKit/storage/ata/ATASMARTLib.h>

#define kATASMARTVendorSpecificAttributesCount     30

#define kATASMARTAttributeTemperature               0xC2
#define kATASMARTAttributeTemperature2              0xE7
#define kATASMARTAttributeEndurance                 0xE8
#define kATASMARTAttributeUnusedReservedBloks       0xB4


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

@interface ATAGenericDisk : NSObject
{
@private
    io_service_t service;
    struct ATASMARTVendorSpecificData data;
    
    NSDate      *lastUpdate;
    
    NSString    *productName;
    NSString    *serialNumber;
    BOOL        isRotational;
    BOOL        isExceeded;
}

@property (readonly) NSString   *productName;
@property (readonly) NSString   *serialNumber;
@property (readonly) BOOL       isRotational;
@property (readonly) BOOL       isExceeded;

+(ATAGenericDisk*)genericDiskWithService:(io_service_t)ioservice productName:(NSString*)name serialNumber:(NSString*)serial isRotational:(BOOL)rotational;

-(BOOL)readSMARTData;
-(ATASMARTAttribute*)getAttributeByIdentifier:(UInt8)identifier;
-(NSData*)getTemperature;
-(NSData*)getRemainingLife;
-(NSData*)getRemainingBlocks;

@end

// NSATASmartReporter

@interface NSATASmartReporter : NSObject
{
@private
    NSArray *drives;
    
@public
}

@property (readonly) NSArray *drives;

+(NSATASmartReporter*)smartReporterByDiscoveringDrives;

-(void)diskoverDrives;

@end
