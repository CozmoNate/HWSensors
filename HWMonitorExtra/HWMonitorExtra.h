//
//  HWMonitorExtra.h
//  HWSensors
//
//  Created by mozo on 03/02/12.
//  Copyright (c) 2012 mozodojo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SystemUIPlugin.h"
#import "HWMonitorView.h"
#import "NSHardwareMonitor.h"

@interface HWMonitorExtra : NSMenuExtra

{
    HWMonitorView *view;
    IBOutlet NSMenu *menu;
    
    NSFont *statusBarFont;
    NSFont *statusMenuFont;
    NSDictionary *statusMenuAttributes;
    
    NSHardwareMonitor *monitor;
}

- (void)insertMenuGroupWithTitle:(NSString*)title  sensors:(NSArray*)list;

- (void)updateSMARTData;
- (void)updateTitles;

- (void)menuItemClicked:(id)sender;

@end
