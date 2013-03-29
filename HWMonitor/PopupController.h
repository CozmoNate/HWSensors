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

#define kHWMonitorToolbarHeight 35

@protocol PopupControllerDelegate <NSObject>
@optional
- (void)popupWillOpen:(id)sender;
- (void)popupDidOpen:(id)sender;
- (void)popupWillClose:(id)sender;
- (void)popupDidClose:(id)sender;
@end

@interface PopupController : NSWindowController <NSWindowDelegate, NSTableViewDelegate, NSTableViewDataSource>
{
    BOOL _hasActivePanel;
    NSMutableArray *_items;
    ColorTheme *_colorTheme;
}

@property (nonatomic, unsafe_unretained) IBOutlet id <PopupControllerDelegate> delegate;

@property (readonly) NSStatusItem *statusItem;
@property (readonly) StatusItemView *statusItemView;

@property (nonatomic, assign) BOOL showVolumeNames;

@property (nonatomic, setter = setColorTheme:) ColorTheme *colorTheme;

@property (nonatomic, unsafe_unretained) IBOutlet PopupView *popupView;
@property (nonatomic, unsafe_unretained) IBOutlet NSTableView *tableView;

@property (nonatomic, unsafe_unretained) IBOutlet NSWindow *prefsWindow;
@property (nonatomic, unsafe_unretained) IBOutlet NSWindow *graphsWindow;

- (IBAction)togglePanel:(id)sender;

- (void)openPanel;
- (void)closePanel;

- (void) setupWithGroups:(NSArray*)groups;
- (void) reloadData;
- (void) updateValuesForSensors:(NSArray*)sensors;

- (IBAction)closeApplication:(id)sender;
- (IBAction)showAboutWindow:(id)sender;
- (IBAction)openPreferences:(id)sender;
- (IBAction)showGraphs:(id)sender;

@end
