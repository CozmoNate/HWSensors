//
//  HWMonitorGraphsView.h
//  HWSensors
//
//  Created by kozlek on 07.07.12.
//
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.


@interface HWMonitorGraphsView : NSView
{
    NSRect _graphBounds;
    NSMutableDictionary *_graphs;
    double _minY;
    double _maxY;
    //double _viewPoints;
    double _maxPoints;
    NSDictionary *_legendAttributes;
    NSString *_legendFormat;
}

@property (assign) IBOutlet NSArrayController *graphs;
@property (readwrite, assign) NSUInteger group;
@property (readwrite, assign) BOOL useFahrenheit;
@property (readwrite, assign) BOOL useSmoothing;

- (void)captureDataToHistoryFromDictionary:(NSDictionary*)info;

@end
