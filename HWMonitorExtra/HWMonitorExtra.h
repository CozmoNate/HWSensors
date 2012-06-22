//
//  HWMonitorExtra.h
//  HWSensors
//
//  Created by kozlek on 03/02/12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "SystemUIPlugin.h"
#import "BundleUserDefaults.h"
#import "HWMonitorIcon.h"
#import "HWMonitorEngine.h"
#import "HWMonitorView.h"

@interface HWMonitorExtra : NSMenuExtra
{
    BundleUserDefaults* _defaults;
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
    
    NSMenu *_mainMenu;
}

@property (readwrite, retain) IBOutlet HWMonitorEngine* engine;
@property (readwrite, retain) IBOutlet NSArray* favorites;

- (void)loadIconNamed:(NSString*)name;
- (HWMonitorIcon*)getIconByName:(NSString*)name;
- (HWMonitorIcon*)getIconByGroup:(NSUInteger)group;
- (NSMenuItem*)insertTitleItemWithMenu:(NSMenu*)someMenu Title:(NSString*)title Image:(NSImage*)image;
- (void)insertMenuGroupWithTitle:(NSString*)title Icon:(HWMonitorIcon*)icon Sensors:(NSArray*)list;

- (void)updateSMARTData;
- (void)updateSMARTDataThreaded;
- (void)updateData;
- (void)updateDataThreaded;

- (void)updateTitlesForceAllSensors:(BOOL)allSensors;
- (void)updateTitlesForced;
- (void)updateTitlesDefault;

- (void)openPreferences:(id)sender;

- (void)rebuildSensors;

- (void)systemWillSleep:(NSNotification*)aNotification;
- (void)systemDidWake:(NSNotification*)aNotification;

- (void)favoritesChanged:(NSNotification*)aNotification;
- (void)useFahrenheitChanged:(NSNotification*)aNotification;
- (void)useBigFontChanged:(NSNotification*)aNotification;
- (void)useShadowEffectChanged:(NSNotification*)aNotification;
- (void)showHiddenSensorsChanged:(NSNotification*)aNotification;
- (void)showBSDNamesChanged:(NSNotification*)aNotification;

@end
