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
#import "HWMonitorEngine.h"

@interface HWMonitorExtra : NSMenuExtra

{
    HWMonitorView *view;
    IBOutlet NSMenu *menu;
    
    NSFont *statusBarFont;
    NSFont *statusMenuFont;
    NSDictionary *statusMenuAttributes;
    
    HWMonitorEngine *monitor;
    
    NSImage *gemClear;
    NSImage *gemGreen;
    NSImage *gemRed;
    NSImage *gemYellow;
}

- (void)setGemForMenuItem:(NSMenuItem*)menuItem Sensor:(HWMonitorSensor*)sensor;

- (void)insertMenuGroupWithTitle:(NSString*)title sensors:(NSArray*)list;

- (void)updateSMARTData;
- (void)updateTitlesForceAllSensors:(BOOL)allSensors;
- (void)updateTitlesForced;
- (void)updateTitlesDefault;

- (void)menuItemClicked:(id)sender;

@end
