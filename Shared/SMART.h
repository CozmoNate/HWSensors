//
//  SMART.h
//  HWSensors
//
//  Created by Natan Zalkin on 18/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#ifndef HWSensors_SMART_h
#define HWSensors_SMART_h

#define kATASMARTVendorSpecific1AttributesCount     30

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

typedef struct ATASMARTVendorSpecific1Data
{
    UInt16 					revisonNumber;
    ATASMARTAttribute		vendorAttributes [kATASMARTVendorSpecific1AttributesCount];
} __attribute__ ((packed)) ATASMARTVendorSpecific1Data;

#endif
