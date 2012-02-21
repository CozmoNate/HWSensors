//
//  AppDelegate.h
//  HWMonitor
//
//  Created by mozo on 20.10.11.
//  Copyright (c) 2011 mozodojo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NSSmartReporter.h"

#include "HWMonitorSensor.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    NSStatusItem *          statusItem;
    NSFont *                statusItemFont;
    NSDictionary*           statusItemAttributes;
    
    NSMutableArray *        sensorsList;
    NSSmartReporter *       smartReporter;
    NSDictionary *          driveTemperatures;
    NSDictionary *          driveRemainingLifes;
    
    BOOL                    isMenuVisible;
    int                     menusCount;
    int                     lastMenusCount;
    int                     smartMenusCount;
    
    IBOutlet NSMenu *       statusMenu;
    NSFont *                statusMenuFont;
    NSDictionary*           statusMenuAttributes;
}

- (HWMonitorSensor *)   addSensorWithKey:(NSString *)key andCaption:(NSString *)caption intoGroup:(SensorGroup)group;
- (void)                insertFooterAndTitle:(NSString *)title;
- (void)                updateSMARTData;
- (void)                updateTitles;

- (void)                menuItemClicked:(id)sender;

@end
