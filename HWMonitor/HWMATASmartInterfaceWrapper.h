//
//  HWMATASmartInterfaceWrapper.h
//  HWMonitor
//
//  Created by Natan Zalkin on 16/04/2017.
//  Copyright © 2017 kozlek. All rights reserved.
//

//
//  HWMAtaSmartSensor.h
//  HWMonitor
//
//  Created by Kozlek on 15/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

// The parts of this code is based on http://smartmontools.sourceforge.net

/*
 * Home page of code is: http://smartmontools.sourceforge.net
 *
 * Copyright (C) 2002-10 Bruce Allen <smartmontools-support@lists.sourceforge.net>
 * Copyright (C) 2008-10 Christian Franke <smartmontools-support@lists.sourceforge.net>
 * Copyright (C) 1999-2000 Michael Cornwell <cornwell@acm.org>
 * Copyright (C) 2000 Andre Hedrick <andre@linux-ide.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * You should have received a copy of the GNU General Public License
 * (for example COPYING); if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This code was originally developed as a Senior Thesis by Michael Cornwell
 * at the Concurrent Systems Laboratory (now part of the Storage Systems
 * Research Center), Jack Baskin School of Engineering, University of
 * California, Santa Cruz. http://ssrc.soe.ucsc.edu/
 *
 */

#import <Foundation/Foundation.h>
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

@interface HWMATASmartInterfaceWrapper : NSObject
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
@property (nonatomic, strong) NSString * bsdName;
@property (nonatomic, strong) NSString * product;
@property (nonatomic, strong) NSString * firmware;
@property (assign) BOOL isRotational;

@property (readonly) NSArray * attributes;
@property (readonly) NSDictionary * overrides;

+(NSDictionary*)getAttributeOverrideForProduct:(NSString*)product firmware:(NSString*)firmware;

+(HWMATASmartInterfaceWrapper*)wrapperWithService:(io_service_t)service productName:(NSString*)productName firmware:(NSString*)firmware bsdName:(NSString*)bsdName isRotational:(BOOL)rotational;
+(HWMATASmartInterfaceWrapper*)getWrapperForBsdName:(NSString*)name;
+(void)destroyAllWrappers;

@end
