//
//  HWMGraph.m
//  HWMonitor
//
//  Created by Kozlek on 24/11/13.
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

#import "HWMGraph.h"
#import "HWMSensor.h"

const NSArray * gHWMGraphsGroupColors;

@implementation HWMGraph

@dynamic color;
@dynamic identifier;
@dynamic hidden;
@dynamic sensor;
@dynamic group;

@synthesize historyMaxValue = _historyMaxValue;
@synthesize historyMinValue = _historyMinValue;

+(const NSArray*)graphColors
{
    if (!gHWMGraphsGroupColors) {

        NSMutableArray *colors = [[NSMutableArray alloc] init];

        NSColorList *list = [NSColorList colorListNamed:@"Crayons"];

        for (NSUInteger i = [[list allKeys] count] - 1; i != 0; i--) {
            NSString *key = [[list allKeys] objectAtIndex:i];
            NSColor *color = [list colorWithKey:key];
            double intensity = (color.redComponent + color.blueComponent + color.greenComponent) / 3.0;
            double red = [color redComponent];
            double green = [color greenComponent];
            double blue = [color blueComponent];
            BOOL blackAndWhite = red == green && red == blue && green == blue;

            if (intensity >= 0.335 && intensity <=0.900 && !blackAndWhite)
                [colors addObject:color];
        }

        gHWMGraphsGroupColors = [colors copy];
    }
    
    return gHWMGraphsGroupColors;
}

-(NSArray *)history
{
    return _history;
}

- (void)captureSensorValueToHistorySetLimit:(NSUInteger)limit
{
    if (!self.sensor.value) {
        return;
    }

    if (!_history) {
        _history = [[NSMutableArray alloc] init];
    }

    if (!_historyMaxValue || [_historyMaxValue isLessThan:self.sensor.value]) {
        _historyMaxValue = [self.sensor.value copy];
    }
    
    if (!_historyMinValue || [_historyMinValue isGreaterThan:self.sensor.value]) {
        _historyMinValue = [self.sensor.value copy];
    }
    
    [_history addObject:[self.sensor.value copy]];

    if (_history.count > limit) {
        [_history removeObjectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _history.count - limit - 1)]];
    }
}

@end
