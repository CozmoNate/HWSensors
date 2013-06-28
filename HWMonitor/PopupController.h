//
//  PopupController.h
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//  Based on code by Vadim Shpanovski <https://github.com/shpakovski/Popup>
//  Popup is licensed under the BSD license.
//  Copyright (c) 2013 Vadim Shpanovski, Natan Zalkin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "StatusItemView.h"
#import "PopupView.h"
#import "ColorTheme.h"

#define kHWMonitorToolbarHeight 37

@protocol PopupControllerDelegate <NSObject>
@optional
- (void)popupWillOpen:(id)sender;
- (void)popupDidOpen:(id)sender;
- (void)popupWillClose:(id)sender;
- (void)popupDidClose:(id)sender;
@end

@class WindowFilter;

@interface PopupController : NSWindowController <NSWindowDelegate, NSTableViewDelegate, NSTableViewDataSource>
{
    BOOL _hasActivePanel;
    NSMutableArray *_items;
    ColorTheme *_colorTheme;
    WindowFilter *_windowFilter;
}

@property (assign) IBOutlet id <PopupControllerDelegate> delegate;

@property (readonly) NSStatusItem *statusItem;
@property (readonly) StatusItemView *statusItemView;

@property (nonatomic, assign) BOOL showVolumeNames;

@property (nonatomic, setter = setColorTheme:) ColorTheme *colorTheme;

@property (assign) IBOutlet PopupView *popupView;
@property (assign) IBOutlet NSTableView *tableView;

@property (assign) IBOutlet NSWindowController *appController;
@property (assign) IBOutlet NSWindowController *graphsController;
@property (assign) IBOutlet NSWindowController *aboutController;
@property (assign) IBOutlet NSWindowController *updatesController;

- (IBAction)togglePanel:(id)sender;
- (IBAction)showAboutPanel:(id)sender;
- (IBAction)openPreferences:(id)sender;
- (IBAction)showGraphsWindow:(id)sender;
- (IBAction)checkForUpdates:(id)sender;

- (void) setupWithGroups:(NSArray*)groups;
- (void) reloadData;
- (void) updateValuesForSensors:(NSArray*)sensors;

@end
