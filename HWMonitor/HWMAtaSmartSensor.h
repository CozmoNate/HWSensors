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

#define kATASMARTAttributesCount     30

typedef struct {
    UInt8 			attributeId;
    UInt16			flag;
    UInt8 			current;
    UInt8 			worst;
    UInt8 			rawvalue[6];
    UInt8 			reserv;
}  __attribute__ ((packed)) ATASMARTAttribute;

// 0: Prefailure bit

// From SFF 8035i Revision 2 page 19: Bit 0 (pre-failure/advisory bit)
// - If the value of this bit equals zero, an attribute value less
// than or equal to its corresponding attribute threshold indicates an
// advisory condition where the usage or age of the device has
// exceeded its intended design life period. If the value of this bit
// equals one, an attribute value less than or equal to its
// corresponding attribute threshold indicates a prefailure condition
// where imminent loss of data is being predicted.
#define ATTRIBUTE_FLAGS_PREFAILURE(x) (x & 0x01)

// 1: Online bit

//  From SFF 8035i Revision 2 page 19: Bit 1 (on-line data collection
// bit) - If the value of this bit equals zero, then the attribute
// value is updated only during off-line data collection
// activities. If the value of this bit equals one, then the attribute
// value is updated during normal operation of the device or during
// both normal operation and off-line testing.
#define ATTRIBUTE_FLAGS_ONLINE(x) (x & 0x02)


// The following are (probably) IBM's, Maxtors and  Quantum's definitions for the
// vendor-specific bits:
// 2: Performance type bit
#define ATTRIBUTE_FLAGS_PERFORMANCE(x) (x & 0x04)

// 3: Errorrate type bit
#define ATTRIBUTE_FLAGS_ERRORRATE(x) (x & 0x08)

// 4: Eventcount bit
#define ATTRIBUTE_FLAGS_EVENTCOUNT(x) (x & 0x10)

// 5: Selfpereserving bit
#define ATTRIBUTE_FLAGS_SELFPRESERVING(x) (x & 0x20)

// 6-15: Reserved for future use
#define ATTRIBUTE_FLAGS_OTHER(x) ((x) & 0xffc0)

typedef struct {
    UInt16 					revisonNumber;
    ATASMARTAttribute		vendorAttributes [kATASMARTAttributesCount];
} __attribute__ ((packed)) ATASmartVendorSpecificData;

typedef struct {
    UInt8             attributeId;
    UInt8             ThresholdValue;
    UInt8             Reserved[10];
} __attribute__ ((packed)) ATASmartThresholdAttribute;

typedef struct {
    UInt16                          revisonNumber;
    ATASmartThresholdAttribute      ThresholdEntries [kATASMARTAttributesCount];
} __attribute__ ((packed)) ATASmartVendorSpecificDataThresholds;

@interface HWMSmartPluginInterfaceWrapper : NSObject
{
    ATASmartVendorSpecificData _vendorSpecificData;
    ATASmartVendorSpecificDataThresholds _vendorSpecificThresholds;

    NSArray *_attributes;

    NSDictionary *_overrides;

    NSString *_product;
    NSString *_firmware;
    NSString *_bsdName;
    BOOL _rotational;
}

@property (nonatomic, assign) IOCFPlugInInterface** pluginInterface;
@property (nonatomic, assign) IOATASMARTInterface** smartInterface;

@property (readonly) NSArray * attributes;

+(HWMSmartPluginInterfaceWrapper*)wrapperWithService:(io_service_t)service productName:(NSString*)productName firmware:(NSString*)firmware bsdName:(NSString*)bsdName isRotational:(BOOL)rotational;
+(HWMSmartPluginInterfaceWrapper*)getWrapperForBsdName:(NSString*)name;
+(void)destroyAllWrappers;

-(HWMSmartPluginInterfaceWrapper*)initWithPluginInterface:(IOCFPlugInInterface**)pluginInterface smartInterface:(IOATASMARTInterface**)smartInterface productName:(NSString*)productName firmware:(NSString*)firmware bsdName:(NSString*)bsdName isRotational:(BOOL)rotational;

@end

typedef enum : NSUInteger {
    kHWMDriveNameProduct,
    kHWMDriveNamePartitions,
    kHWMDriveNameBSD,
} HWMDriveNameSelector;

@interface HWMAtaSmartSensor : HWMSensor
{
    NSInteger _temperatureAttributeIndex;
    NSInteger _remainingLifeAttributeIndex;
}

@property (nonatomic, retain) NSString * bsdName;
@property (nonatomic, retain) NSString * productName;
@property (nonatomic, retain) NSString * volumeNames;
@property (nonatomic, retain) NSString * revision;
@property (nonatomic, retain) NSString * serialNumber;
@property (nonatomic, retain) NSNumber * rotational;

@property (readonly) NSArray *attributes;

+(void)updatePartitionsList;

+(void)startWatchingForBlockStorageDevicesWithEngine:(HWMEngine*)engine;
+(void)stopWatchingForBlockStorageDevices;

-(void)updateVolumeNames;

@end
