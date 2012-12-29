//
//  HWMonitorGroup.m
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

#import "HWMonitorGroup.h"
#import "HWMonitorEngine.h"

#import "HWMonitorDefinitions.h"

@implementation HWMonitorGroup

@synthesize sensorGroup = _group;
@synthesize titleFont = _titleFont;
@synthesize textFont = _textFont;

@synthesize separatorItem = _separatorItem;
@synthesize titleMenuItem = _titleMenuItem;

@synthesize items = _items;

@synthesize isFirst = _isFirst;

+ (HWMonitorGroup*)groupWithEngine:(HWMonitorEngine*)engine sensorGroup:(enum HWSensorGroup)sensorGroup menu:(NSMenu*)menu titleFont:(NSFont*)titleFont textFont:(NSFont*)textFont title:(NSString*)title image:(NSImage*)image
{
    HWMonitorGroup* group = [[HWMonitorGroup alloc] initWithEngine:engine sensorGroup:sensorGroup menu:menu titleFont:titleFont textFont:textFont title:title image:image];
    
    if (!group)
        return nil;
        
    return group;
}

- (HWMonitorGroup*)initWithEngine:(HWMonitorEngine*)engine sensorGroup:(enum HWSensorGroup)sensorGroup menu:(NSMenu*)menu titleFont:(NSFont*)titleFont textFont:(NSFont*)textFont title:(NSString*)title image:(NSImage*)image
{
    self = [super init];
    
    _engine = engine;
    _group = sensorGroup;
    _titleFont = titleFont;
    _textFont = textFont;
    _items = [[NSMutableArray alloc] init];
    
    NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:[title stringByTruncatingToWidth:[[NSAttributedString alloc] initWithString:@"H" attributes:[NSDictionary dictionaryWithObject:_titleFont forKey:NSFontAttributeName]].size.width * (kHWMonitorMenuTitleWidth + kHWMonitorMenuTabWidth + kHWMonitorMenuValueWidth) withFont:_titleFont]];
    
    [attributedTitle addAttribute:NSForegroundColorAttributeName value:[NSColor controlShadowColor] range:NSMakeRange(0, [attributedTitle length])];
    [attributedTitle addAttribute:NSFontAttributeName value:_textFont range:NSMakeRange(0, [attributedTitle length])];

    _separatorItem = [NSMenuItem separatorItem];
    
    [menu addItem:_separatorItem];
    
    _titleMenuItem = [menu addItemWithTitle:@"" action:nil keyEquivalent:@""];
    
    [_titleMenuItem setAttributedTitle:attributedTitle];
    [_titleMenuItem setImage:image];
    [_titleMenuItem setEnabled:NO];
    
    NSArray* sensors = [_engine getAllSensorsInGroup:sensorGroup];
    
    if (sensors) {
        NSArray *favorites = [[NSUserDefaults standardUserDefaults] arrayForKey:kHWMonitorFavoritesList];
        
        for (HWMonitorSensor *sensor in sensors) {
            
            NSMenuItem *menuItem = [menu addItemWithTitle:@"" action:nil keyEquivalent:@""];
            
            [menuItem setEnabled:NO];
            
            HWMonitorItem *monitorItem = [HWMonitorItem itemWithGroup:self sensor:sensor menuItem:menuItem];

            [monitorItem setFavorite:favorites ? [favorites containsObject:[sensor key]] : NO];
            
            [_items addObject:monitorItem];
        }
    }
    
    return self;
}

- (BOOL)checkVisibility;
{
    int visibleCount = 0;
    
    for (HWMonitorItem *item in _items)
        if ([item isVisible])
            visibleCount++;
    
    if (visibleCount == 0) {
        [_separatorItem setHidden:YES];
        [_titleMenuItem setHidden:YES];
    }
    else {
        [_separatorItem setHidden:_isFirst];
        [_titleMenuItem setHidden:NO];
    }
    
    return visibleCount > 0;
}

@end