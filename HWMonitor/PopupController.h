//
//  PopupController.h
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

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
 
}

@property (assign) IBOutlet id <PopupControllerDelegate> delegate;

@property (readonly) IBOutlet HWMEngine *monitorEngine;

@property (readonly) NSStatusItem *statusItem;
@property (readonly) StatusItemView *statusItemView;

@property (assign) IBOutlet NSMenu *mainMenu;
@property (assign) IBOutlet NSView *toolbarView;
@property (assign) IBOutlet NSScrollView *scrollView;
@property (assign) IBOutlet NSTableView *tableView;

@property (assign) IBOutlet NSWindowController *appController;
@property (assign) IBOutlet NSWindowController *graphsController;
@property (assign) IBOutlet NSWindowController *aboutController;

@property (assign) IBOutlet NSLayoutConstraint *tableHeightConstraint;

@property (atomic, assign) NSDragOperation currentItemDragOperation;
@property (readonly) BOOL hasDraggedFavoriteItem;

@property (readonly) NSArray *sensorsAndGroupsCollectionSnapshot;

- (IBAction)togglePanel:(id)sender;
- (IBAction)showAboutPanel:(id)sender;
- (IBAction)openPreferences:(id)sender;
- (IBAction)showGraphsWindow:(id)sender;

@end
