//
//  HWMonitorGroup.h
//  HWSensors
//
//  Created by kozlek on 28.06.12.
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


#import "HWMonitorItem.h"

@interface HWMonitorGroup : NSObject
{
    HWMonitorEngine* _engine;
}

@property (readwrite, assign) enum HWSensorGroup sensorGroup;

@property (readwrite, retain) NSFont* titleFont;
@property (readwrite, retain) NSFont* textFont;

@property (readwrite, retain) NSMenuItem* separatorItem;
@property (readwrite, retain) NSMenuItem* titleMenuItem;

@property (readwrite, retain) NSMutableArray* items;

@property (readwrite, assign, setter = setFirst:) BOOL isFirst;

+ (HWMonitorGroup*)groupWithEngine:(HWMonitorEngine*)engine sensorGroup:(enum HWSensorGroup)sensorGroup menu:(NSMenu*)menu titleFont:(NSFont*)titleFont textFont:(NSFont*)textFont title:(NSString*)title image:(NSImage*)image;

- (HWMonitorGroup*)initWithEngine:(HWMonitorEngine*)engine sensorGroup:(enum HWSensorGroup)sensorGroup menu:(NSMenu*)menu titleFont:(NSFont*)titleFont textFont:(NSFont*)textFont title:(NSString*)title image:(NSImage*)image;

- (BOOL)checkVisibility;

@end