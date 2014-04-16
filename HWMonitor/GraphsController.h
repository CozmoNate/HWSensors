//
//  GraphsController.h
//  HWMonitor
//
//  Created by kozlek on 24.02.13.
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

@class GraphsView;
@class HWMonitorItem;

@class HWMEngine;

@interface GraphsController : NSWindowController <NSWindowDelegate, NSTableViewDelegate, NSTableViewDataSource, NSCollectionViewDelegate>
{
    NSMutableArray *_graphViews;
}

@property (readonly) IBOutlet HWMEngine *monitorEngine;

@property (readonly) id selectedItem;

@property (assign) IBOutlet NSScrollView *graphsScrollView;
@property (assign) IBOutlet NSTableView *graphsTableView;
@property (assign) IBOutlet NSCollectionView *graphsCollectionView;

@property (atomic, assign) NSDragOperation currentItemDragOperation;

@property (readonly) NSArray *graphsAndGroupsCollectionSnapshot;

-(IBAction)setNeedDisplayGraphs:(id)sender;

@end
