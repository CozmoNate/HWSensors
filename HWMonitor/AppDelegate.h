//
//  AppDelegate.h
//  HWMonitor
//
//  Created by mozo on 20.10.11.
//  Copyright (c) 2011 mozodojo. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include "HWMonitorSensor.h"

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

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    NSStatusItem *          statusItem;
    NSFont *                statusItemFont;
    NSDictionary*           statusItemAttributes;
    
    NSMutableArray *        sensorsList;
    NSDictionary *          driveTemperatures;
    NSTimer *               updatedriveTemperaturesTimer;
    
    BOOL                    isMenuVisible;
    int                     menusCount;
    int                     lastMenusCount;
    int                     smartMenusCount;
    
    IBOutlet NSMenu *       statusMenu;
    NSFont *                statusMenuFont;
    NSDictionary*           statusMenuAttributes;
}

- (void)updateTitles;
- (HWMonitorSensor *)addSensorWithKey:(NSString *)key andCaption:(NSString *)caption intoGroup:(SensorGroup)group;
- (void)insertFooterAndTitle:(NSString *)title;

- (void)updateDrivesTemperatures;

- (void)menuItemClicked:(id)sender;

@end
