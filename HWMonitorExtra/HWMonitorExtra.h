//
//  HWMonitorExtra.h
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SystemUIPlugin.h"
#import "HWMonitorSensor.h"
#import "HWMonitorView.h"

#define kATASmartVendorSpecific1AttributesCount         30
#define kATASmartVendorSpecific1TemperatureAttribute    0xC2

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

@interface HWMonitorExtra : NSMenuExtra

{
    HWMonitorView *     view;
    IBOutlet NSMenu *   menu;
    
    int                 menusCount;
    int                 lastMenusCount;
    
    NSMutableArray *    sensorsList;
    NSDictionary *      driveTemperatures;
    
    NSFont *            statusBarFont;
    NSFont *            statusMenuFont;
    NSDictionary*       statusMenuAttributes;
}

- (HWMonitorSensor *)   addSensorWithKey:(NSString *)key andCaption:(NSString *)caption intoGroup:(SensorGroup)group;
- (void)                insertFooterAndTitle:(NSString *)title;
- (void)                updateDrivesTemperatures;
- (void)                updateTitles:(BOOL)forced;
- (void)                updateTitlesForced;
- (void)                updateTitlesDefault;



- (void)                menuItemClicked:(id)sender;

@end
