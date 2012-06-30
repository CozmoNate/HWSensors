//
//  HWMonitorItem.m
//  HWSensors
//
//  Created by kozlek on 28.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "NSString+TruncateToWidth.h"

#import "HWMonitorItem.h"
#import "HWMonitorGroup.h"

@implementation HWMonitorItem

@synthesize title = _title;

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
    _title = [title stringByTruncatingToWidth:145 withFont:[_group menuFont]];
}

-(NSString *)title
{
    return _title;
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
