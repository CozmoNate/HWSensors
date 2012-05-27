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

@interface HWMonitorExtra : NSMenuExtra

{
    BundleUserDefaults *defaults;
    
    HWMonitorView *view;
    NSMenu *menu;
    
    NSMenu *prefsMenu;
    
    NSMenuItem *celsiusItem;
    NSMenuItem *fahrenheitItem;
    
    NSFont *statusMenuFont;
    NSDictionary *statusMenuAttributes;
    NSDictionary *blackColorAttribute;
    NSDictionary *orangeColorAttribute;
    NSDictionary *redColorAttribute;
    
    HWMonitorEngine *monitor;
    NSMutableArray *favorites;
    
    NSImage *favoriteIcon;
    NSImage *disabledIcon;
    NSImage *temperaturesIcon;
    NSImage *hddtemperaturesIcon;
    NSImage *ssdlifeIcon;
    NSImage *multipliersIcon;
    NSImage *frequenciesIcon;
    NSImage *tachometersIcon;
    NSImage *voltagesIcon;
    NSImage *prefsIcon;
}

- (void)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Icon:(NSImage*)image;
- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(NSImage*)image Sensors:(NSArray*)list;
- (NSMenuItem*)insertPrefsItemWithTitle:(NSString*)title icon:(NSImage*)image state:(NSUInteger)state action:(SEL)aSelector keyEquivalent:(NSString *)charCode;

- (void)updateSMARTData;
- (void)updateSMARTDataThreaded;
- (void)updateData;
- (void)updateDataThreaded;

- (void)updateTitlesForceAllSensors:(BOOL)allSensors;
- (void)updateTitlesForced;
- (void)updateTitlesDefault;
- (void)rebuildSensors;

- (void)sensorItemClicked:(id)sender;
- (void)degreesItemClicked:(id)sender;
- (void)showHiddenSensorsItemClicked:(id)sender;
- (void)showBSDNamesItemClicked:(id)sender;
- (void)favoritesInRowItemClicked:(id)sender;
- (void)useShadowEffectItemClicked:(id)sender;

- (void)sleepNoteReceived:(NSNotification*)note;
- (void)wakeNoteReceived:(NSNotification*)note;

@end
