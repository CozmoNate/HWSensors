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
#import "NSSmartReporter.h"

@interface HWMonitorExtra : NSMenuExtra

{
    HWMonitorView *     view;
    IBOutlet NSMenu *   menu;
    
    int                 menusCount;
    int                 lastMenusCount;
    
    NSMutableArray *    sensorsList;
    
    NSSmartReporter *   smartReporter;
    NSDictionary *      driveTemperatures;
    NSDictionary *      driveRemainingLifes;
    
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
