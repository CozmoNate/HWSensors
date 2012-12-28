//
//  HWMonitorItem.m
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


#import "NSString+TruncateToWidth.h"

#import "HWMonitorItem.h"
#import "HWMonitorGroup.h"

#import "HWMonitorDefinitions.h"

@implementation HWMonitorItem

@synthesize title = _title;
@synthesize subTitle = _subTitle;

@synthesize sensor = _sensor;
@synthesize menuItem = _menuItem;

@synthesize isVisible = _isVisible;
@synthesize isFavorite = _isFavorite;

+ (HWMonitorItem*)itemWithGroup:(HWMonitorGroup*)group sensor:(HWMonitorSensor*)sensor menuItem:(NSMenuItem*)menuItem
{
    return [[HWMonitorItem alloc] initWithGroup:group sensor:sensor menuItem:menuItem];
}

-(void)setTitle:(NSString *)title
{
    _title = [title stringByTruncatingToWidth:[[NSAttributedString alloc] initWithString:@"0" attributes:[NSDictionary dictionaryWithObject:[_group textFont] forKey:NSFontAttributeName]].size.width * kHWMonitorMenuTitleWidth withFont:[_group textFont]];
}

-(NSString*)title
{
    return _title;
}

-(void)setSubTitle:(NSString *)subTitle
{
    _subTitle = [subTitle stringByTruncatingToWidth:[[NSAttributedString alloc] initWithString:@"0" attributes:[NSDictionary dictionaryWithObject:[_group textFont] forKey:NSFontAttributeName]].size.width * kHWMonitorMenuTitleWidth withFont:[_group textFont]];
}

-(NSString *)subTitle
{
    return _subTitle;
}

-(void)setVisible:(BOOL)isVisible
{
    _isVisible = isVisible;
    
    [_menuItem setHidden:!isVisible];
}

-(BOOL)isVisible
{
    return _isVisible;
}

- (HWMonitorItem*)initWithGroup:(HWMonitorGroup*)group sensor:(HWMonitorSensor*)sensor menuItem:(NSMenuItem*)menuItem
{
    _group = group;
    _sensor = sensor;
    _menuItem = menuItem;
    
    [_menuItem setRepresentedObject:self];
    [_sensor setRepresentedObject:self];
    
    [self setTitle:[_sensor title]];
    
    _isVisible = YES;
    _isFavorite = NO;
    
    return self;
}

@end
