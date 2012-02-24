//
//  AppDelegate.h
//  HWMonitor
//
//  Created by mozo on 20.10.11.
//  Copyright (c) 2011 mozodojo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NSATASmartReporter.h"

#import "NSHardwareMonitor.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    NSStatusItem *statusItem;
    NSFont *statusItemFont;
    NSDictionary *statusItemAttributes;
    
    IBOutlet NSMenu *statusMenu;
    NSFont *statusMenuFont;
    NSDictionary *statusMenuAttributes;
    
    NSHardwareMonitor *monitor;
    
    BOOL isMenuVisible;
}

- (void)insertMenuGroupWithTitle:(NSString *)title sensors:(NSArray*)list;

- (void)updateSMARTData;
- (void)updateTitles;

- (void)menuItemClicked:(id)sender;

@end
