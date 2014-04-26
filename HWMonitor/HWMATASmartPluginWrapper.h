//
//  HWMATASmartPluginWrapper.h
//  HWMonitor
//
//  Created and modified by Kozlek on 23.04.14.
//

/*

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc.
 may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

 Copyright (C) 2007 Apple Inc. All Rights Reserved.
 
 */

#import <Foundation/Foundation.h>

#include <IOKit/storage/ata/ATASMARTLib.h>

// Notifications
extern NSString * const HWMAtaSmartSensorDidAddBlockStorageDevices;
extern NSString * const HWMAtaSmartSensorDidRemoveBlockStorageDevices;

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

    NSDictionary *_overrides;
}

@property (readonly) IOCFPlugInInterface** pluginInterface;
@property (readonly) IOATASMARTInterface** smartInterface;
@property (nonatomic, strong) NSString* bsdName;
@property (nonatomic, strong) NSString* productName;
@property (nonatomic, strong) NSString* firmwareString;
@property (assign) BOOL isRotational;

@property (readonly) NSArray * attributes;

+(HWMATASmartInterfaceWrapper*)wrapperWithService:(io_service_t)service bsdName:(NSString*)bsdName productName:(NSString*)productName firmware:(NSString*)firmware isRotational:(BOOL)rotational;
+(HWMATASmartInterfaceWrapper*)getWrapperForBsdName:(NSString*)name;
+(void)destroyAllWrappers;

@end
