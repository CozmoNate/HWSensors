//
//  HWMonitorGroup.m
//  HWSensors
//
//  Created by kozlek on 28.06.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "NSString+TruncateToWidth.h"

#import "HWMonitorGroup.h"
#import "HWMonitorEngine.h"

#import "HWMonitorDefinitions.h"

@implementation HWMonitorGroup

@synthesize sensorGroup = _group;
@synthesize menuFont = _font;

@synthesize separatorItem = _separatorItem;
@synthesize titleMenuItem = _titleMenuItem;

@synthesize items = _items;

@synthesize isFirst = _isFirst;

+ (HWMonitorGroup*)groupWithEngine:(HWMonitorEngine*)engine sensorGroup:(enum HWSensorGroup)sensorGroup menu:(NSMenu*)menu font:(NSFont*)font title:(NSString*)title image:(NSImage*)image
{
    HWMonitorGroup* group = [[HWMonitorGroup alloc] initWithEngine:engine sensorGroup:sensorGroup menu:menu font:font title:title image:image];
    
    if (!group)
        return nil;
        
    return group;
}

- (HWMonitorGroup*)initWithEngine:(HWMonitorEngine*)engine sensorGroup:(enum HWSensorGroup)sensorGroup menu:(NSMenu*)menu font:(NSFont*)font title:(NSString*)title image:(NSImage*)image
{
    _engine = engine;
    _group = sensorGroup;
    _font = font;
    _items = [[NSMutableArray alloc] init];
    
    NSMutableAttributedString *attributedTitle = [[NSMutableAttributedString alloc] initWithString:[title stringByTruncatingToWidth:190 withFont:_font]];
    
    [attributedTitle addAttribute:NSForegroundColorAttributeName value:[NSColor controlShadowColor] range:NSMakeRange(0, [attributedTitle length])];
    [attributedTitle addAttribute:NSFontAttributeName value:_font range:NSMakeRange(0, [attributedTitle length])];

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

- (void)checkVisibility;
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
}

@end