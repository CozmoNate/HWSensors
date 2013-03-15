//
//  ATASmartReporter.h
//  HWSensors
//
//  Based on ISPSmartController from iStatPro by Buffy (c) 2007
//  ISPSmartController licensed under GPL
//
//  Created by kozlek on 19/02/12.
//

#import <Foundation/Foundation.h>

#include <IOKit/storage/ata/ATASMARTLib.h>

#define kATASMARTVendorSpecificAttributesCount     30

#define kATASMARTAttributeTemperature               0xC2
#define kATASMARTAttributeTemperature2              0xE7
#define kATASMARTAttributeTemperature3              0xBE
#define kATASMARTAttributeEndurance                 0xE8
#define kATASMARTAttributeEndurance2                0xE7
#define kATASMARTAttributeUnusedReservedBloks       0xB4


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

// NSATAGenericDisk

@interface ATAGenericDisk : NSObject
{
@private
    io_service_t                _service;
    ATASmartVendorSpecificData  _data;
    NSDate                      *lastUpdated;
}

@property (readonly) NSString   *productName;
@property (readonly) NSString   *bsdName;
@property (readonly) NSString   *volumesNames;
@property (readonly) NSString   *serialNumber;
@property (readonly) BOOL       isRotational;
@property (readonly) BOOL       isExceeded;

+(ATAGenericDisk*)genericDiskWithService:(io_service_t)ioservice productName:(NSString*)name bsdName:(NSString*)bsd volumesNames:(NSString*)volumes serialNumber:(NSString*)serial isRotational:(BOOL)rotational;

-(BOOL)readSMARTData;
-(ATASMARTAttribute*)getAttributeByIdentifier:(UInt8)identifier;
-(NSData*)getTemperature;
-(NSData*)getRemainingLife;
-(NSData*)getRemainingBlocks;

@end

// NSATASmartReporter

@interface ATASmartReporter : NSObject
{
    NSArray *_drives;
}

@property (readonly) NSArray *drives;

+(ATASmartReporter*)smartReporterByDiscoveringDrives;

-(void)diskoverDrives;

@end
