//
//  ATASmartReporter.h
//  HWSensors
//
//  Based on code by Navi
//
//  Created by kozlek on 19/02/12.
//

/*
 *  Copyright (c) 2013 Navi, Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

#import <Foundation/Foundation.h>

#include <IOKit/storage/ata/ATASMARTLib.h>

#define kATASMARTVendorSpecificAttributesCount     30

#define kATASMARTAttributeTemperature               0xC2
#define kATASMARTAttributeTemperature2              0xE7
#define kATASMARTAttributeTemperature3              0xBE
#define kATASMARTAttributeEndurance                 0xE8
#define kATASMARTAttributeEndurance2                0xE7
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
