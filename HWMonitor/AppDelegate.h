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
    
    IBOutlet NSMenu *statusMenu;
    NSFont *statusMenuFont;
    NSDictionary *statusMenuAttributes;
    
    HWMonitorEngine *monitor;
    
    BOOL isMenuVisible;
    
    NSImage *gemClear;
    NSImage *gemGreen;
    NSImage *gemRed;
    NSImage *gemYellow;
}

- (void)setGemForMenuItem:(NSMenuItem*)menuItem Sensor:(HWMonitorSensor*)sensor;

- (void)insertMenuGroupWithTitle:(NSString *)title sensors:(NSArray*)list;

- (void)updateSMARTData;
- (void)updateTitles;

- (void)menuItemClicked:(id)sender;

@end
