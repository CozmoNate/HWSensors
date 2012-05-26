//
//  AppDelegate.h
//  HWMonitor
//
//  Created by mozo on 20.10.11.
//  Copyright (c) 2011 mozodojo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ATASmartReporter.h"

#import "HWMonitorEngine.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    NSStatusItem *statusItem;
    NSFont *statusItemFont;
    NSDictionary *statusItemAttributes;
    NSShadow *statusItemShadow;
    
    IBOutlet NSMenu *statusMenu;
    NSFont *statusMenuFont;
    NSDictionary *statusMenuAttributes;
    
    NSDictionary * blackColorAttribute;
    NSDictionary * orangeColorAttribute;
    NSDictionary * redColorAttribute;
    
    HWMonitorEngine *monitor;
    
    BOOL isMenuVisible;
    
    NSImage *favoriteIcon;
    NSImage *altFavoriteIcon;
    NSImage *disabledIcon;
    NSImage *altDisabledIcon;
    
    NSMenu *prefsMenu;
    
    NSMenuItem *celsiusItem;
    NSMenuItem *fahrenheitItem;
}

- (void)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Icon:(NSImage*)image;
- (void)insertMenuGroupWithTitle:(NSString *)title Icon:(NSImage*)image Sensors:(NSArray*)list;
- (NSMenuItem*)insertPrefsItemWithTitle:(NSString*)title icon:(NSImage*)image state:(NSUInteger)state action:(SEL)aSelector keyEquivalent:(NSString *)charCode;

- (void)updateSMARTData;
- (void)updateSMARTDataThreaded;
- (void)updateData;
- (void)updateDataThreaded;
- (void)updateTitles;

- (void)rebuildSensors;

- (void)sensorItemClicked:(id)sender;
- (void)degreesItemClicked:(id)sender;
- (void)showHiddenSensorsItemClicked:(id)sender;
- (void)showBSDNamesItemClicked:(id)sender;

- (void)sleepNoteReceived:(NSNotification*)note;
- (void)wakeNoteReceived:(NSNotification*)note;

@end
