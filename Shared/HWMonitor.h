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
#import "BundleUserDefaults.h"

@interface HWMonitor : NSObject

{
    BOOL _isMenuExtra;
    NSStatusItem* _statusItem;
    NSBundle* _bundle;
    BundleUserDefaults* _defaults;
    HWMonitorEngine* _monitor;
    HWMonitorView*  _view;
    NSMenu* _mainMenu;
    NSMenu* _prefsMenu;
    NSMenuItem *_celsiusItem;
    NSMenuItem *_fahrenheitItem;
    NSImage *_favoriteIcon;
    NSImage *_disabledIcon;
    NSImage *_temperaturesIcon;
    NSImage *_hddtemperaturesIcon;
    NSImage *_ssdlifeIcon;
    NSImage *_multipliersIcon;
    NSImage *_frequenciesIcon;
    NSImage *_tachometersIcon;
    NSImage *_voltagesIcon;
    NSImage *_prefsIcon;
    NSFont *_menuFont;
    NSDictionary *_menuAttributes;
    NSDictionary *_blackColorAttribute;
    NSDictionary *_orangeColorAttribute;
    NSDictionary *_redColorAttribute;
    NSMutableArray *_favorites;
}

@property (readwrite, retain) NSMenu* menu;

- (id)initWithStatusItem:(NSStatusItem*)item bundle:(NSBundle*)bundle;

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

- (void)systemWillSleep:(NSNotification*)aNotification;
- (void)systemDidWake:(NSNotification*)aNotification;

@end
