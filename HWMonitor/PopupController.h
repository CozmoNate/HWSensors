//
//  PopupController.h
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//  

#import "StatusItemView.h"
#import "ColorTheme.h"
#import "OBMenuBarWindow.h"

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
}

@property (assign) IBOutlet id <PopupControllerDelegate> delegate;

@property (readonly) NSStatusItem *statusItem;
@property (readonly) StatusItemView *statusItemView;

@property (nonatomic, assign) BOOL showVolumeNames;

@property (nonatomic, setter = setColorTheme:) ColorTheme *colorTheme;

@property (assign) IBOutlet NSView *toolbarView;
@property (assign) IBOutlet NSTableView *tableView;
@property (assign) IBOutlet NSTextField *titleField;


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
