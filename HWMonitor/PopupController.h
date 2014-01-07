//
//  PopupController.h
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//

#import <Sparkle/SUUpdater.h>

#import "StatusItemView.h"
#import "OBMenuBarWindow.h"

@protocol PopupControllerDelegate <NSObject>
@optional
- (void)popupWillOpen:(id)sender;
- (void)popupDidOpen:(id)sender;
- (void)popupWillClose:(id)sender;
- (void)popupDidClose:(id)sender;
@end

@class HWMEngine;

@interface PopupController : NSWindowController <NSApplicationDelegate, NSWindowDelegate, NSTableViewDelegate, NSTableViewDataSource>
{
    NSMutableArray *_sensorsAndGroupsCollectionSnapshot;
}

@property (assign) IBOutlet id <PopupControllerDelegate> delegate;

@property (assign) IBOutlet HWMEngine *monitorEngine;

@property (readonly) NSStatusItem *statusItem;
@property (readonly) StatusItemView *statusItemView;

@property (nonatomic, assign) BOOL showVolumeNames;

@property (assign) IBOutlet NSMenu *mainMenu;
@property (assign) IBOutlet NSView *toolbarView;
@property (assign) IBOutlet NSScrollView *scrollView;
@property (assign) IBOutlet NSTableView *tableView;

@property (assign) IBOutlet NSWindowController *appController;
@property (assign) IBOutlet NSWindowController *graphsController;
@property (assign) IBOutlet NSWindowController *aboutController;

@property (atomic, assign) NSDragOperation currentItemDragOperation;
@property (readonly) BOOL hasDraggedFavoriteItem;

- (IBAction)togglePanel:(id)sender;
- (IBAction)showAboutPanel:(id)sender;
- (IBAction)openPreferences:(id)sender;
- (IBAction)showGraphsWindow:(id)sender;

@end
