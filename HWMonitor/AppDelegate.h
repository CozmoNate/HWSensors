//
//  AppDelegate.h
//  HWMonitor
//
//  Created by kozlek on 22.06.12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.


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
    NSUInteger _lastSelectedView;
    
    BOOL _menuExtraConnectionActive;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSView *preferencesView;
@property (assign) IBOutlet NSView *graphsView;
@property (assign) IBOutlet NSToolbar *prefsToolbar;
@property (assign) IBOutlet NSMenu *menu;
@property (assign) IBOutlet HWMonitorSensorsController *sensorsController;
@property (assign) IBOutlet NSArrayController *graphsController;
@property (assign) IBOutlet NSTextField *versionLabel;
@property (assign) IBOutlet NSButton *toggleMenuButton;

@property (assign) IBOutlet NSTextField *sensorsUpdateRateText;
@property (assign) IBOutlet NSSlider *sensorsUpdateRateSlider;
@property (assign) IBOutlet NSTextField *SMARTUpdateRateText;
@property (assign) IBOutlet NSSlider *SMARTUpdateRateSlider;

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
- (IBAction)updateRateChanged:(id)sender;

- (IBAction)toggleGraphSmoothing:(id)sender;

- (IBAction)graphsTableViewClicked:(id)sender;
- (IBAction)prefsToolbarClicked:(id)sender;

@end
