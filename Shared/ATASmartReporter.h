//
//  ATASmartReporter.h
//  HWSensors
//
//  Based on code by OldNavi
//
//  Created by kozlek on 19/02/12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
