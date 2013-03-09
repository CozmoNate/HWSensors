//
//  HWMonitorGraphsView.m
//  HWSensors
//
//  Created by kozlek on 07.07.12.
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


#import "GraphsView.h"
#import "HWMonitorDefinitions.h"

@implementation GraphsView

#define GraphScale 6.0

#define LeftViewMargin 5
#define TopViewMargin 5
#define RightViewMargin 8
#define BottomViewMargin 8

-(id)init
{
    self = [super init];
    
    if (self) {
        NSShadow *shadow = [[NSShadow alloc] init];
        
        [shadow setShadowColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.55]];
        [shadow setShadowOffset:CGSizeMake(0, -1.0)];
        [shadow setShadowBlurRadius:1.0];
        
        _legendAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSFont systemFontOfSize:9.0], NSFontAttributeName,
                             [NSColor yellowColor], NSForegroundColorAttributeName,
                             shadow, NSShadowAttributeName,
                             nil];
        
        _legendFormat = @"%1.0f";
    }
    
    return self;
}

- (NSArray*)addItemsFromList:(NSArray*)itemsList forSensorGroup:(HWSensorGroup)sensorsGroup;
{
    if (sensorsGroup & (kHWSensorGroupTemperature | kSMARTGroupTemperature)) {
        _legendFormat = @"%1.0f°";
    }
    else if (sensorsGroup & kHWSensorGroupFrequency) {
        _legendFormat = @"%1.0fMHz";
    }
    else if (sensorsGroup & kHWSensorGroupTachometer) {
        _legendFormat = @"%1.0frpm";
    }
    else if (sensorsGroup & kHWSensorGroupVoltage) {
        _legendFormat = @"%1.3fV";
    }
    else if (sensorsGroup & kHWSensorGroupAmperage) {
        _legendFormat = @"%1.3fA";
    }
    else if (sensorsGroup & kHWSensorGroupPower) {
        _legendFormat = @"%1.3fW";
    }
    
    if (!_items) {
        _items = [[NSMutableArray alloc] init];
    }
    else {
        [_items removeAllObjects];
    }
    
    if (!_graphs) {
        _graphs = [[NSMutableDictionary alloc] init];
    }
    else {
        [_graphs removeAllObjects];
    }
 
    for (HWMonitorItem *item in itemsList) {
        [_items addObject:item];
        [_graphs setObject:[[NSMutableArray alloc] init] forKey:[[item sensor] name]];
    }
    
    [self calculateGraphBoundsFindExtremes:YES];
    
    return _items;
}

- (void)captureDataToHistoryNow;
{
    for (HWMonitorItem *item in _items) {
        HWMonitorSensor *sensor = [item sensor];
        NSMutableArray *history = [_graphs objectForKey:[sensor name]];

        if ([sensor rawValue]) {
            [history addObject:[sensor rawValue]];
            
            if ([history count] > _maxPoints + 8) {
                [history removeObjectAtIndex:0];
            }
        }
    }
    
    [self calculateGraphBoundsFindExtremes:YES];
    
    [self setNeedsDisplay:YES];
}

- (void)calculateGraphBoundsFindExtremes:(BOOL)findExtremes
{
    if (findExtremes) {
        _maxY = 0, _minY = MAXFLOAT;
        
        for (HWMonitorItem *item in _items) {

            if ([_graphsController checkItemIsHidden:item])
                continue;
            
            HWMonitorSensor *sensor = [item sensor];
            NSArray *points = [_graphs objectForKey:[sensor name]];
            
            if (points) {
                for (NSNumber *point in points) {
                    if ([point doubleValue] < _minY) {
                        _minY = [point doubleValue];
                    }
                    else if ([point doubleValue] > _maxY)
                    {
                        _maxY = [point doubleValue];
                    }
                }
            }
        }
    }

    _maxPoints = [self bounds].size.width / GraphScale;
    
    if ((_maxY == 0 && _minY == MAXFLOAT)) {
        _graphBounds = NSMakeRect(0, 0, _maxPoints, 100);
    }
    else if (_minY >= _maxY) {
        _graphBounds = NSMakeRect(0, _minY, _maxPoints, _minY + 100);
    }
    else {

        double minY = _minY <= 0 ? _minY : _minY - _minY * 0.05;
        double maxY = _maxY + _maxY * 0.05;
        
        _graphBounds = NSMakeRect(0, minY, _maxPoints, maxY - minY);
    }
}

- (NSPoint)graphPointToView:(NSPoint)point
{
    double graphScaleX = ([self bounds].size.width - LeftViewMargin - RightViewMargin) / _graphBounds.size.width;
    double graphScaleY = ([self bounds].size.height - TopViewMargin - BottomViewMargin) / _graphBounds.size.height;
    double x = LeftViewMargin + (point.x - _graphBounds.origin.x) * graphScaleX;
    double y = BottomViewMargin + (point.y - _graphBounds.origin.y) * graphScaleY;
    
    return NSMakePoint(x, y);
}

- (void)drawRect:(NSRect)rect
{
    [self calculateGraphBoundsFindExtremes:NO];
    
    [[[NSGradient alloc]
      initWithStartingColor:[NSColor colorWithCalibratedWhite:0.15 alpha:0.75]
                endingColor:[NSColor colorWithCalibratedWhite:0.25 alpha:0.75]]
        drawInRect:self.bounds angle:270];
    
    //double x, y;

    NSGraphicsContext* context = [NSGraphicsContext currentContext];
    
    [context saveGraphicsState];
    
    // Draw marks
    [context setShouldAntialias:NO];
    
    NSBezierPath *path = [[NSBezierPath alloc] init];
    
    if (_minY < _maxY) {
        // Draw extremums
        [context setShouldAntialias:NO];
        
        [path removeAllPoints];
        [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x,_maxY)]];
        [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width,_maxY)]];
        [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x,_minY)]];
        [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width,_minY)]];
        CGFloat pattern[2] = { 4.0, 4.0 };
        [path setLineDash:pattern count:2 phase:1.0];
        [[NSColor lightGrayColor] set];
        [path setLineWidth:0.25];
        [path stroke];
        CGFloat resetPattern[1] = { 0 };
        [path setLineDash:resetPattern count:0 phase:0];
    }
    
    // Draw graphs
    
    [context setShouldAntialias:YES];
    
    for (HWMonitorItem *item in _items) {
        
        if ([_graphsController checkItemIsHidden:item])
            continue;

        HWMonitorSensor *sensor = [item sensor];
        NSArray *values = [_graphs objectForKey:[sensor name]];
        
        if (!values || [values count] < 2)
            continue;
        
        [path removeAllPoints];
        [path setLineJoinStyle:NSRoundLineJoinStyle];
        
        if (_useSmoothing) {
            CGFloat startOffset = _graphBounds.size.width - [values count] + 1;
            
            NSPoint lastPoint = NSMakePoint(startOffset, [[values objectAtIndex:0] doubleValue]);
            
            [path moveToPoint:[self graphPointToView:lastPoint]];
            
            for (NSUInteger index = 1; index < [values count]; index++) {
                NSPoint nextPoint = NSMakePoint(startOffset + index, [[values objectAtIndex:index] doubleValue]);
                NSPoint controlPoint1 = NSMakePoint(lastPoint.x + (nextPoint.x - lastPoint.x) * 0.7, lastPoint.y + (nextPoint.y - lastPoint.y) * 0.35);
                NSPoint controlPoint2 = NSMakePoint(lastPoint.x + (nextPoint.x - lastPoint.x) * 0.3, lastPoint.y + (nextPoint.y - lastPoint.y) * 0.65);
                
                [path curveToPoint:[self graphPointToView:nextPoint]
                     controlPoint1:[self graphPointToView:controlPoint1]
                     controlPoint2:[self graphPointToView:controlPoint2]];
                
                lastPoint = nextPoint;
            }
        }
        else {
            CGFloat startOffset = _graphBounds.size.width - [values count] + 1;
            
            [path moveToPoint:[self graphPointToView:NSMakePoint(startOffset, [[values objectAtIndex:0] doubleValue])]];
            
            for (NSUInteger index = 1; index < [values count]; index++) {
                NSPoint p1 = NSMakePoint(startOffset + index, [[values objectAtIndex:index] doubleValue]);
                [path lineToPoint:[self graphPointToView:p1]];
            }
        }
        
        if (item == [_graphsController selectedItem]) {
            [[[item color] highlightWithLevel:0.8] set];
            [path setLineWidth:3.0];
        }
        else {
            [[item color] set];
            [path setLineWidth:2.0];
        }
        
        [path stroke];
    }
    
    // Draw extreme values
    if (_minY < _maxY) {
        [context setShouldAntialias:YES];

        NSAttributedString *maxExtremeTitle = [[NSAttributedString alloc]
                                               initWithString:[NSString stringWithFormat:_legendFormat, ((group & kHWSensorGroupTemperature || group & kSMARTGroupTemperature) && _useFahrenheit ? _maxY * (9.0f / 5.0f) + 32.0f : _maxY )]
                                               attributes:_legendAttributes];

        NSAttributedString *minExtremeTitle = [[NSAttributedString alloc]
                                     initWithString:[NSString stringWithFormat:_legendFormat, ((group & kHWSensorGroupTemperature || group & kSMARTGroupTemperature) && _useFahrenheit ? _minY * (9.0f / 5.0f) + 32.0f : _minY )]
                                     attributes:_legendAttributes];

        if ([self graphPointToView:NSMakePoint(0, _maxY)].y + 2 + [maxExtremeTitle size].height > [self graphPointToView:NSMakePoint(0, _graphBounds.origin.y + _graphBounds.size.height)].y || [self graphPointToView:NSMakePoint(0, _minY)].y - [minExtremeTitle size].height < [self graphPointToView:_graphBounds.origin].y) {
            [maxExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _maxY)].y - [maxExtremeTitle size].height)];
            [minExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _minY)].y + 2)];
        }
        else {
            [maxExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _maxY)].y + 2)];
            [minExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _minY)].y - [minExtremeTitle size].height)];
        }
    }
    
    [context restoreGraphicsState];
}

@end
