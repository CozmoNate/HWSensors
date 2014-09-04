//
//  AppDelegate.h
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
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
#import <Growl/Growl.h>

#import "PopupController.h"
#import "PopoverController.h"
#import "GraphsController.h"
#import "HWMEngine.h"

@interface AppController : NSWindowController <NSApplicationDelegate, NSTableViewDataSource, NSTableViewDelegate, PopupControllerDelegate, HWMEngineDelegate, GrowlApplicationBridgeDelegate, NSPopoverDelegate>
{
    NSView *_previousView;
}

@property (readonly) IBOutlet HWMEngine *monitorEngine;

@property (nonatomic, strong) IBOutlet NSMutableArray *themePreview;
@property (nonatomic, strong) IBOutlet NSMutableIndexSet *themeSelectionIndexes;

@property (nonatomic, strong) IBOutlet NSMutableArray *drivePreview;
@property (nonatomic, strong) IBOutlet NSMutableIndexSet *driveSelectionIndexes;

@property (assign) IBOutlet PopupController *popupController;
@property (assign) IBOutlet PopoverController *popoverController;
@property (assign) IBOutlet GraphsController *graphsController;
@property (assign) IBOutlet SUUpdater *sharedUpdater;

@property (assign) IBOutlet NSTableView *favoritesTableView;
@property (assign) IBOutlet NSTableView *sensorsTableView;

@property (readonly) BOOL hasDraggedFavoriteItem;
@property (atomic, assign) NSDragOperation currentItemDragOperation;

@property (readonly) NSArray *sensorsAndGroupsCollectionSnapshot;
@property (readonly) NSArray *favoritesCollectionSnapshot;

- (IBAction)checkForUpdates:(id)sender;
- (IBAction)rebuildSensorsList:(id)sender;

@end
