//
//  HWMAtaSmartSensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
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
#import <CoreData/CoreData.h>
#import "HWMSensor.h"

#include <IOKit/storage/ata/ATASMARTLib.h>

@interface HWMSmartPlugInInterfaceWrapper : NSObject

@property (nonatomic, assign) IOCFPlugInInterface** pluginInterface;
@property (nonatomic, assign) IOATASMARTInterface ** smartInterface;

+(HWMSmartPlugInInterfaceWrapper*)wrapperWithService:(io_service_t)service forBsdName:(NSString*)name;
+(HWMSmartPlugInInterfaceWrapper*)getWrapperForBsdName:(NSString*)name;
+(void)destroyAllWrappers;

@end

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
    NSDate * updated;
    ATASmartVendorSpecificData _smartData;
}

@property (nonatomic, retain) NSString * productName;
@property (nonatomic, retain) NSString * bsdName;
@property (nonatomic, retain) NSString * volumeNames;
@property (nonatomic, retain) NSString * serialNumber;
@property (nonatomic, retain) NSNumber * rotational;

@property (readonly) Boolean exceeded;

+(NSArray*)discoverDrives;

@end
