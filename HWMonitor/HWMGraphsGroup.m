//
//  HWMGraphsGroup.m
//  HWMonitor
//
//  Created by Kozlek on 27/11/13.
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

#import "HWMGraphsGroup.h"
#import "HWMConfiguration.h"
#import "HWMGraph.h"
#import "HWMIcon.h"
#import "HWMSensorsGroup.h"
#import "HWMSensor.h"

NSString * const HWMGraphsGroupHistoryHasBeenChangedNotification = @"HWMGraphsGroupHistoryHasBeenChangedNotification";

@implementation HWMGraphsGroup

@dynamic configuration;
@dynamic graphs;
@dynamic icon;

@synthesize selectors;

@synthesize maxGraphsValue = _maxGraphsValue;
@synthesize minGraphsValue = _minGraphsValue;

-(void)addGraphsObject:(HWMGraph *)value
{
    [[self mutableOrderedSetValueForKey:@"graphs"] addObject:value];
}

- (void)moveGraphsObject:(HWMGraph *)value toIndex:(NSUInteger)toIndex
{
    if (toIndex > self.graphs.count) {
        toIndex = self.graphs.count;
    }
    
    NSUInteger fromIndex = [self.graphs indexOfObject:value];
    
    [[self mutableOrderedSetValueForKey:@"graphs"] moveObjectsAtIndexes:[NSIndexSet indexSetWithIndex:fromIndex] toIndex:fromIndex < toIndex || toIndex == self.graphs.count ? toIndex - 1 : toIndex];
}

-(void)captureSensorValuesToGraphsHistorySetLimit:(NSUInteger)limit
{
    @synchronized (self) {
        
        _maxGraphsValue = nil;
        _minGraphsValue = nil;
        
        for (HWMGraph *graph in self.graphs) {
            
            [graph captureSensorValueToHistorySetLimit:limit];
            
            if (graph.hidden.boolValue)
                continue;
            
            if (!_maxGraphsValue || (graph.historyMaxValue && [_maxGraphsValue isLessThan:graph.historyMaxValue])) {
                _maxGraphsValue = [graph.historyMaxValue copy];
            }
            
            if (!_minGraphsValue || (graph.historyMinValue && [_minGraphsValue isGreaterThan:graph.historyMinValue])) {
                _minGraphsValue = [graph.historyMinValue copy];
            }
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:HWMGraphsGroupHistoryHasBeenChangedNotification object:self];
}

@end
