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
@synthesize historyMaxCount = _historyMaxCount;
@synthesize historyMinValue = _historyMinValue;
@synthesize historyMinCount = _historyMinCount;

+(void)addColor:(NSColor*)color toArray:(NSMutableArray*)array
{
    double red = [color redComponent];
    double green = [color greenComponent];
    double blue = [color blueComponent];
    double intensity =  0.3 * red + 0.5 * green + 0.2 * blue;
    BOOL blackAndWhite = red == green && red == blue && green == blue;

    //if (idx > 7 && !blackAndWhite)
    if (intensity >= 0.4 && intensity <= 0.90 && !blackAndWhite)
        [array insertObject:color atIndex:0];
}

+(const NSArray*)graphColors
{
    if (!gHWMGraphsGroupColors) {

        NSMutableArray *colors = [[NSMutableArray alloc] init];

        NSColorList *list = [NSColorList colorListNamed:@"Crayons"];

        NSArray *colorNames = [list allKeys];

        [colorNames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {

                [HWMGraph addColor:[list colorWithKey:obj] toArray:colors];

        }];

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
        _historyMaxCount = 1;
    }
    else if ([_historyMaxValue isEqualToNumber:self.sensor.value]) {
        _historyMaxCount++;
    }
    
    if (!_historyMinValue || [_historyMinValue isGreaterThan:self.sensor.value]) {
        _historyMinValue = [self.sensor.value copy];
        _historyMinCount = 1;
    }
    else if ([_historyMinValue isEqualToNumber:self.sensor.value]) {
        _historyMinCount++;
    }
    
    [_history addObject:[self.sensor.value copy]];

    if (_history.count > limit) {
        NSIndexSet *range = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _history.count - limit - 1)];

        NSArray *objectsToRemove = [_history objectsAtIndexes:range];

        NSArray *removedMaximums = [objectsToRemove filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"floatValue == %f", _historyMaxValue.floatValue]];
        NSArray *removedMinmums = [objectsToRemove filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"floatValue == %f", _historyMinValue.floatValue]];

        [_history removeObjectsAtIndexes:range];

        _historyMaxCount -= removedMaximums.count;

        if (_historyMaxCount == 0) {
            _historyMaxValue = [_history valueForKeyPath:@"@max.floatValue"];
            _historyMaxCount = [_history filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"floatValue == %f", _historyMaxValue.floatValue]].count;
            //NSLog(@"%@ max updated!", self.sensor.name);
        }

        _historyMinCount -= removedMinmums.count;

        if (_historyMinCount == 0) {
            _historyMinValue = [_history valueForKeyPath:@"@min.floatValue"];
            _historyMinCount = [_history filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"floatValue == %f", _historyMinValue.floatValue]].count;
            //NSLog(@"%@ min updated!", self.sensor.name);
        }
    }
}

@end
