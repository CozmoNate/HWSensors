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
}

- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(NSImage*)image Sensors:(NSArray*)list;

- (void)updateSMARTData;
- (void)updateTitlesForceAllSensors:(BOOL)allSensors;
- (void)updateTitlesForced;
- (void)updateTitlesDefault;
- (void)rebuildSensors;

- (void)menuItemClicked:(id)sender;

@end
