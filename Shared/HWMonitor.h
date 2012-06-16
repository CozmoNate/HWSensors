//
//  HWMonitor.h
//  HWSensors
//
//  Created by kozlek on 06.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "HWMonitorEngine.h"
#import "HWMonitorView.h"
#import "HWMonitorIcon.h"
#import "HWMonitorArrayController.h"

#import "BundleUserDefaults.h"
#import "SystemUIPlugin.h"

@interface HWMonitor : NSObject

{
    BOOL _isMenuExtra;
    NSStatusItem* _statusItem;
    NSBundle* _bundle;
    BundleUserDefaults* _defaults;
    NSMenu* _prefsMenu;
    NSFont *_menuFont;
    NSDictionary *_menuAttributes;
    NSDictionary *_blackColorAttribute;
    NSDictionary *_orangeColorAttribute;
    NSDictionary *_redColorAttribute;
    NSImage *_favoriteIcon;
    NSImage *_disabledIcon;
    NSImage *_prefsIcon;
    NSMutableDictionary *_icons;
    NSMutableArray *_favorites;
}

@property (readwrite, retain) IBOutlet NSMenu* menu;
@property (readwrite, retain) IBOutlet HWMonitorEngine* engine;
@property (readwrite, retain) IBOutlet HWMonitorView* view;
@property (readwrite, retain) IBOutlet NSArray* favorites;

@property (assign) IBOutlet NSWindow *prefsWindow;
@property (assign) IBOutlet HWMonitorArrayController *arrayController;

- (IBAction)preferencesDidChanged:(id)sender;
- (IBAction)forceRebuildSensors:(id)sender;

- (id)initWithStatusItem:(NSStatusItem*)item bundle:(NSBundle*)bundle;

- (void)loadIconNamed:(NSString*)name;
- (HWMonitorIcon*)getIconByName:(NSString*)name;
- (HWMonitorIcon*)getIconByGroup:(NSUInteger)group;
- (NSMenuItem*)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Image:(NSImage*)image;
- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(HWMonitorIcon*)icon Sensors:(NSArray*)list;
//- (NSMenuItem*)insertPrefsItemWithTitle:(NSString*)title icon:(NSImage*)image state:(NSUInteger)state action:(SEL)aSelector keyEquivalent:(NSString *)charCode;

- (void)updateSMARTData;
- (void)updateSMARTDataThreaded;
- (void)updateData;
- (void)updateDataThreaded;

- (void)updateTitlesForceAllSensors:(BOOL)allSensors;
- (void)updateTitlesForced;
- (void)updateTitlesDefault;

- (void)rebuildSensors;

- (void)prefsItemClicked:(id)sender;

- (void)systemWillSleep:(NSNotification*)aNotification;
- (void)systemDidWake:(NSNotification*)aNotification;

@end
