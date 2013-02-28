//
//  PopupController.h
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

#import <Cocoa/Cocoa.h>

#import "StatusItemView.h"
#import "PopupView.h"

@class PopupController;

@interface PopupController : NSWindowController <NSWindowDelegate, NSTableViewDelegate, NSTableViewDataSource>
{
    BOOL _hasActivePanel;
    NSMutableArray *_items;
}

@property (readonly) NSStatusItem *statusItem;
@property (readonly) StatusItemView *statusItemView;

@property (nonatomic, assign) BOOL hasActivePanel;
@property (nonatomic, assign) BOOL showVolumeNames;

@property (nonatomic, unsafe_unretained) IBOutlet PopupView *popupView;
@property (nonatomic, unsafe_unretained) IBOutlet NSScrollView *scrollView;
@property (nonatomic, unsafe_unretained) IBOutlet NSTableView *tableView;

@property (nonatomic, unsafe_unretained) IBOutlet NSWindow *prefsWindow;
@property (nonatomic, unsafe_unretained) IBOutlet NSWindow *graphsWindow;

- (void)openPanel;
- (void)closePanel;

- (void) setupWithGroups:(NSArray*)groups;
- (void) reloadData;
- (void) updateValues;

- (IBAction)closeApplication:(id)sender;
- (IBAction)openPreferences:(id)sender;
- (IBAction)showGraphs:(id)sender;

@end
