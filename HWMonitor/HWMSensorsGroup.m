//
//  HWMSensorsGroup.m
//  HWMonitor
//
//  Created by Kozlek on 25/11/13.
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

#import "HWMSensorsGroup.h"
#import "HWMIcon.h"
#import "HWMSensor.h"
#import "HWMConfiguration.h"

@implementation HWMSensorsGroup

@dynamic configuration;
@dynamic selector;
@dynamic icon;
@dynamic sensors;

-(void)addSensorsObject:(HWMSensor *)value
{
    [[self mutableOrderedSetValueForKey:@"sensors"] addObject:value];
}

-(void)moveSensorsObject:(HWMSensor *)value toIndex:(NSUInteger)toIndex
{
    if (toIndex > self.sensors.count) {
        toIndex = self.sensors.count;
    }

    NSUInteger fromIndex = [self.sensors indexOfObject:value];

    [[self mutableOrderedSetValueForKey:@"sensors"] moveObjectsAtIndexes:[NSIndexSet indexSetWithIndex:fromIndex] toIndex:fromIndex < toIndex || toIndex == self.sensors.count ? toIndex - 1 : toIndex];
}

@end
