//
//  HWMonitorExtra.h
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SystemUIPlugin.h"
#import "BundleUserDefaults.h"
#import "HWMonitorView.h"
#import "HWMonitorEngine.h"

#define kHWMonitorUseFahrenheitKey  "useFahrenheit"

@interface HWMonitorExtra : NSMenuExtra

{
    BundleUserDefaults *defaults;
    
    HWMonitorView *view;
    NSMenu *menu;
    
    NSMenuItem *celsiusItem;
    NSMenuItem *fahrenheitItem;
    
    NSFont *statusMenuFont;
    NSDictionary *statusMenuAttributes;
    NSDictionary *blackColorAttribute;
    NSDictionary *orangeColorAttribute;
    NSDictionary *redColorAttribute;
    
    HWMonitorEngine *monitor;
    
    NSImage *favoriteIcon;
    NSImage *disabledIcon;
    NSImage *temperaturesIcon;
    NSImage *hddtemperaturesIcon;
    NSImage *ssdlifeIcon;
    NSImage *multipliersIcon;
    NSImage *frequenciesIcon;
    NSImage *tachometersIcon;
    NSImage *voltagesIcon;
    
    IBOutlet NSWindow *prefsWindow;
}

- (void)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Icon:(NSImage*)image;
- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(NSImage*)image Sensors:(NSArray*)list;

- (void)updateSMARTData;
- (void)updateTitlesForceAllSensors:(BOOL)allSensors;
- (void)updateTitlesForced;
- (void)updateTitlesDefault;
- (void)rebuildSensors;

- (void)sensorItemClicked:(id)sender;
- (void)celsiusItemClicked:(id)sender;
- (void)farenheitItemClicked:(id)sender;

@end
