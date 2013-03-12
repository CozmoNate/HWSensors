//
//  HWMonitorGroup.m
//  HWSensors
//
//  Created by kozlek on 28.06.12.
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

#import "HWMonitorGroup.h"
#import "HWMonitorEngine.h"

#import "HWMonitorDefinitions.h"

@implementation HWMonitorGroup

+ (HWMonitorGroup*)groupWithEngine:(HWMonitorEngine*)engine sensorGroup:(HWSensorGroup)sensorGroup title:(NSString*)title image:(HWMonitorIcon*)icon
{
    HWMonitorGroup* group = [[HWMonitorGroup alloc] initWithEngine:engine sensorGroup:sensorGroup title:title image:icon];
    
    if (!group)
        return nil;
        
    return group;
}

- (HWMonitorGroup*)initWithEngine:(HWMonitorEngine*)engine sensorGroup:(HWSensorGroup)group title:(NSString*)title image:(HWMonitorIcon*)icon
{
    self = [super init];
    
    _engine = engine;
    _group = group;
    _items = [[NSMutableArray alloc] init];

    [self setTitle:title];
    [self setIcon:icon];
    
    NSArray* sensors = [_engine getAllSensorsInGroup:group];
    
    if (sensors) {
        //NSArray *favorites = [[NSUserDefaults standardUserDefaults] arrayForKey:kHWMonitorFavoritesList];
        
        for (HWMonitorSensor *sensor in sensors) {
            HWMonitorItem *monitorItem = [HWMonitorItem itemWithGroup:self sensor:sensor];

            //[monitorItem setFavorite:favorites ? [favorites containsObject:[sensor name]] : NO];
            
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
    
    return visibleCount > 0;
}

@end