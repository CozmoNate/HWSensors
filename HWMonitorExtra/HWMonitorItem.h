//
//  HWMonitorItem.h
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

#import "HWMonitorSensor.h"

@class HWMonitorGroup;

@interface HWMonitorItem : NSObject
{
    HWMonitorGroup* _group;
}

@property (readwrite, retain) NSString* title;
@property (readwrite, retain) NSString* subTitle;

@property (readwrite, retain) HWMonitorSensor* sensor;
@property (readwrite, retain) NSMenuItem* menuItem;

@property (readwrite, assign, setter = setVisible:) BOOL isVisible;
@property (readwrite, assign, setter = setFavorite:) BOOL isFavorite;

+ (HWMonitorItem*)itemWithGroup:(HWMonitorGroup*)group sensor:(HWMonitorSensor*)sensor menuItem:(NSMenuItem*)menuItem;

- (HWMonitorItem*)initWithGroup:(HWMonitorGroup*)group sensor:(HWMonitorSensor*)sensor menuItem:(NSMenuItem*)menuItem;

@end