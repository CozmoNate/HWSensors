//
//  AppDelegate.h
//  HWMonitor
//
//  Created by kozlek on 22.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "BundleUserDefaults.h"
#import "HWMonitorEngine.h"
#import "HWMonitorSensorsController.h"
#import "HWMonitorGraphsView.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    BundleUserDefaults *_defaults;
    NSMutableDictionary *_icons;
    NSMutableArray *_favorites;
    NSMutableDictionary *_sensors;
    NSMutableArray *_globalColors;
    NSUInteger _globalColorIndex;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSView *preferencesView;
@property (assign) IBOutlet NSView *menubarView;
@property (assign) IBOutlet NSView *graphsView;
@property (assign) IBOutlet NSToolbar *prefsToolbar;
@property (assign) IBOutlet NSMenu *menu;
@property (assign) IBOutlet NSTokenField *favoritesTokens;
@property (assign) IBOutlet HWMonitorSensorsController *sensorsController;
@property (assign) IBOutlet NSArrayController *graphsController;
@property (assign) IBOutlet NSTextField *versionLabel;
@property (assign) IBOutlet NSButton *toggleMenuButton;

@property (assign) IBOutlet HWMonitorGraphsView *temperatureGraph;
@property (assign) IBOutlet HWMonitorGraphsView *frequencyGraph;
@property (assign) IBOutlet HWMonitorGraphsView *tachometerGraph;
@property (assign) IBOutlet HWMonitorGraphsView *voltageGraph;

@property (readwrite, assign) IBOutlet NSObject *userInterfaceEnabled;

- (void)recieveItems:(NSNotification*)aNotification;
- (void)valuesChanged:(NSNotification*)aNotification;

- (IBAction)toggleMenu:(id)sender;

- (IBAction)favoritesChanged:(id)sender;
- (IBAction)useFahrenheitChanged:(id)sender;
- (IBAction)useBigFontChanged:(id)sender;
- (IBAction)useShadowEffectChanged:(id)sender;
- (IBAction)useBSDNamesChanged:(id)sender;
- (IBAction)showVolumeNamesChanged:(id)sender;

- (IBAction)graphsTableViewClicked:(id)sender;
- (IBAction)prefsToolbarClicked:(id)sender;

@end
