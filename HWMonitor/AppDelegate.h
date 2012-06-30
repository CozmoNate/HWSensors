//
//  AppDelegate.h
//  HWMonitor
//
//  Created by kozlek on 22.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "BundleUserDefaults.h"
#import "HWMonitorEngine.h"
#import "HWMonitorArrayController.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    BundleUserDefaults *_defaults;
    NSMutableDictionary *_icons;
    NSMutableArray *_favorites;
    NSMutableDictionary *_sensors;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet HWMonitorArrayController *arrayController;
@property (assign) IBOutlet NSTextField *versionLabel;
@property (assign) IBOutlet NSButton *toggleMenuButton;

@property (readwrite, assign) IBOutlet NSObject *userInterfaceEnabled;

- (void)recieveItems:(NSNotification*)aNotification;

- (IBAction)toggleMenu:(id)sender;

- (IBAction)favoritesChanged:(id)sender;
- (IBAction)useFahrenheitChanged:(id)sender;
- (IBAction)useBigFontChanged:(id)sender;
- (IBAction)useShadowEffectChanged:(id)sender;
- (IBAction)showBSDNamesChanged:(id)sender;

@end
