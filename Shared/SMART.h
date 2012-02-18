//
//  SMART.h
//  HWSensors
//
//  Created by Natan Zalkin on 18/02/12.
//  Copyright (c) 2012 natan.zalkin@gmail.com. All rights reserved.
//

#ifndef HWSensors_SMART_h
#define HWSensors_SMART_h

#define kATASmartVendorSpecific1AttributesCount 30
#define kATASmartVendorSpecific1Temperature     0xC2
#define kATASmartVendorSpecific1Temperature2    0xE7

typedef struct ATASmartAttribute
{
    UInt8 			attributeId;
    UInt16			flag;  
    UInt8 			current;
    UInt8 			worst;
    UInt8 			rawvalue[6];
    UInt8 			reserv;
}  __attribute__ ((packed)) ATASmartAttribute;

typedef struct ATASmartVendorSpecific1Data
{
    UInt16 					revisonNumber;
    ATASmartAttribute		vendorAttributes [kATASmartVendorSpecific1AttributesCount];
} __attribute__ ((packed)) ATASmartVendorSpecific1Data;

#endif
