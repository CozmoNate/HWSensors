//
//  AppDelegate.h
//  HWMonitor
//
//  Created by Kozlek on 22.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "BundleUserDefaults.h"
#import "HWMonitorEngine.h"
#import "HWMonitorArrayController.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    BundleUserDefaults *_defaults;
    NSMutableDictionary *_icons;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet HWMonitorEngine *engine;
@property (assign) IBOutlet HWMonitorArrayController *arrayController;
@property (assign) IBOutlet NSButton *toggleMenuButton;

- (void)loadIconNamed:(NSString*)name;
- (HWMonitorIcon*)getIconByName:(NSString*)name;
- (HWMonitorIcon*)getIconByGroup:(NSUInteger)group;
- (void)addAvailableItemsFromGroup:(NSUInteger)group;
- (void)rebuildSensors;

- (IBAction)toggleMenu:(id)sender;

- (IBAction)favoritesChanged:(id)sender;
- (IBAction)useFahrenheitChanged:(id)sender;
- (IBAction)useBigFontChanged:(id)sender;
- (IBAction)useShadowEffectChanged:(id)sender;
- (IBAction)showHiddenSensorsChanged:(id)sender;
- (IBAction)showBSDNamesChanged:(id)sender;

@end
