//
//  HWMonitorGraphsView.m
//  HWSensors
//
//  Created by kozlek on 07.07.12.
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


#import "HWMonitorGraphsView.h"
#import "HWMonitorSensor.h"
#import "HWMonitorDefinitions.h"
#import "HWMonitorBezierSpline.h"

@implementation HWMonitorGraphsView

@synthesize graphs = _content;
@synthesize group = _group;
@synthesize useFahrenheit = _useFahrenheit;

-(void)setGroup:(NSUInteger)group
{
    _group = group;
    
    if (_group & kHWSensorGroupTemperature || _group & kSMARTSensorGroupTemperature) {
        _legendFormat = @"%1.0f°";
    }
    else if (_group & kHWSensorGroupFrequency) {
        _legendFormat = @"%1.0fMHz";
    }
    else if (_group & kHWSensorGroupTachometer) {
        _legendFormat = @"%1.0frpm";
    }
    else if (_group & kHWSensorGroupVoltage) {
        _legendFormat = @"%1.3fV";
    }
    
    [self calculateGraphBoundsLoopExtremes:YES];
}

-(NSUInteger)group
{
    return _group;
}

-(id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    
    if (!self)
        return nil;
    
    _graphs = [[NSMutableDictionary alloc] init];
    
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
    
    return self;
}

- (void)calculateGraphBoundsLoopExtremes:(BOOL)doLoop
{
    if (doLoop) {
        _maxY = 0, _minY = MAXFLOAT;
        
        for (NSDictionary *item in [_content arrangedObjects]) {
            NSNumber *enabled = [item objectForKey:kHWMonitorKeyEnabled];
            
            if (enabled && [enabled boolValue]) {
                NSString *key = [item objectForKey:kHWMonitorKeyKey];
                NSArray *points = [_graphs objectForKey:key];
                
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
    }

    if (_graphBounds.size.width > _maxPoints)
        _maxPoints = _viewPoints;
    
    _viewPoints = [self bounds].size.width / 4;
    
    if ((_maxY == 0 && _minY == MAXFLOAT)) {
        _graphBounds = NSMakeRect(0, 0, _viewPoints, 100);
    }
    else if (_minY >= _maxY) {
        _graphBounds = NSMakeRect(0, _minY, _viewPoints, _minY + 100);
    }
    else {

        double minY = _minY <= 0 ? _minY : _minY - _minY * 0.05;
        double maxY = _maxY + _maxY * 0.05;
        
        _graphBounds = NSMakeRect(0, minY, _viewPoints, maxY - minY);
    }
}

#define LeftViewMargin 5
#define TopViewMargin 5
#define RightViewMargin 8
#define BottomViewMargin 8

- (NSPoint)graphPointToView:(NSPoint)point
{
    double graphScaleX = ([self bounds].size.width - LeftViewMargin - RightViewMargin) / _graphBounds.size.width;
    double graphScaleY = ([self bounds].size.height - TopViewMargin - BottomViewMargin) / _graphBounds.size.height;
    double x = LeftViewMargin + (point.x - _graphBounds.origin.x) * graphScaleX;
    double y = BottomViewMargin + (point.y - _graphBounds.origin.y) * graphScaleY;
    
    return NSMakePoint(x, y);
}

#define GraphBezier 1
#define GraphScale 2.0
#define GraphTension 5.0

- (void)drawRect:(NSRect)dirtyRect
{
    [self calculateGraphBoundsLoopExtremes:NO];
    
    //[[NSColor colorWithCalibratedRed:0.94 green:0.94 blue:0.94 alpha:1.0] set];
    [[NSColor blackColor] set];
    [NSBezierPath fillRect:[self bounds]];
    
    double x, y;

    NSGraphicsContext* context = [NSGraphicsContext currentContext];
    
    [context saveGraphicsState];
    
    // Draw marks
    [context setShouldAntialias:NO];
    
    NSBezierPath *path = [[NSBezierPath alloc] init];
    
    [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x, _graphBounds.origin.y)]];
    [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width, _graphBounds.origin.y)]];
    [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width, _graphBounds.origin.y + _graphBounds.size.height)]];
    [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x, _graphBounds.origin.y + _graphBounds.size.height)]];
    [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x, _graphBounds.origin.y)]];
    [path setClip];
    
    [path removeAllPoints];
    
    [path setLineWidth:0.33];
    
    [[NSColor colorWithCalibratedRed:0 green:0.25 blue:0 alpha:0.7] set];
    
    double xStart = _graphBounds.origin.x + _graphBounds.size.width;
    double yStart = floor(_graphBounds.origin.y / 10) * 10;
    double xInc = 5;
    double yInc = _graphBounds.size.height / 10;
    
    for (x = xStart; x > 0; x -= xInc) {
        [path removeAllPoints];
        [path moveToPoint:[self graphPointToView:NSMakePoint(x,_graphBounds.origin.y)]];
        [path lineToPoint:[self graphPointToView:NSMakePoint(x,_graphBounds.origin.y + _graphBounds.size.height)]];
        [path stroke];
        
        for (y = yStart; y < _graphBounds.origin.y + _graphBounds.size.height; y += yInc) {
            [path removeAllPoints];
            [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x,y)]];
            [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width,y)]];
            [path stroke];
        }
    }
    
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
            
    for (NSDictionary *item in [_content arrangedObjects]) {
        NSNumber *enabled = [item objectForKey:kHWMonitorKeyEnabled];
        
        if (enabled && [enabled boolValue]) {
            NSString *key = [item objectForKey:kHWMonitorKeyKey];
            NSArray *values = [_graphs objectForKey:key];
                        
            if (values && [values count] >= 2) {
                NSColor *color = nil;
                
                if ([[_content selectedObjects] containsObject:item]) {
                    color = [NSColor whiteColor];
                    [path setLineWidth:3.5];
                }
                else {
                    NSColor *itemColor = [item objectForKey:kHWMonitorKeyColor];
                    color = [NSColor colorWithCalibratedRed:itemColor.redComponent green:itemColor.greenComponent blue:itemColor.blueComponent alpha:1.00];
                    [path setLineWidth:2.0];
                }
                
                [path removeAllPoints];
                
                [color set];
                
                NSUInteger pointsCount = [values count];// > kHWMonitorGraphsHistoryPoints ? kHWMonitorGraphsHistoryPoints : [points count];
                CGFloat startOffset = _graphBounds.size.width - pointsCount * GraphScale + 2 * GraphScale;
                
                NSMutableArray *graph = [[NSMutableArray alloc] init];
                
                for (NSUInteger index = 0; index < [values count]; index++)
                    [graph addObject:[NSValue valueWithPoint: NSMakePoint(startOffset + index * GraphScale, [[values objectAtIndex:index] doubleValue])]];
                
                NSArray *spline = [HWMonitorBezierSpline getBezierPointsForGraph:graph withTension:GraphTension];
                
                [path moveToPoint:[self graphPointToView:[[spline objectAtIndex:0] pointValue]]];
                
                for (NSUInteger index = 1; index + 3 < [spline count]; index += 3) {
                    NSPoint p1 = [[spline objectAtIndex:index] pointValue];
                    NSPoint p2 = [[spline objectAtIndex:index + 1] pointValue];
                    NSPoint q2 = [[spline objectAtIndex:index + 2] pointValue];
                    
                    [path curveToPoint:[self graphPointToView:q2]
                         controlPoint1:[self graphPointToView:p1]
                         controlPoint2:[self graphPointToView:p2]];
                }
                
                [path stroke];
            }
        }
    }
    
    // Draw extreme values
    if (_minY < _maxY) {
        [context setShouldAntialias:YES];

        NSAttributedString *maxExtremeTitle = [[NSAttributedString alloc]
                                               initWithString:[NSString stringWithFormat:_legendFormat, ((_group & kHWSensorGroupTemperature || _group & kSMARTSensorGroupTemperature) && _useFahrenheit ? _maxY * (9.0f / 5.0f) + 32.0f : _maxY )]
                                               attributes:_legendAttributes];

        NSAttributedString *minExtremeTitle = [[NSAttributedString alloc]
                                     initWithString:[NSString stringWithFormat:_legendFormat, ((_group & kHWSensorGroupTemperature || _group & kSMARTSensorGroupTemperature) && _useFahrenheit ? _minY * (9.0f / 5.0f) + 32.0f : _minY )]
                                     attributes:_legendAttributes];

        if ([self graphPointToView:NSMakePoint(0, _maxY)].y - [maxExtremeTitle size].height < [self graphPointToView:NSMakePoint(0, _minY)].y + 2 + [minExtremeTitle size].height) {
            [maxExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _maxY)].y + 2)];
            
            [minExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _minY)].y - [minExtremeTitle size].height)];
        }
        else {
            [maxExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _maxY)].y - [maxExtremeTitle size].height)];
            
            [minExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _minY)].y + 2)];
        }
    }
    
    [context restoreGraphicsState];
}

- (void)captureDataToHistoryFromDictionary:(NSDictionary*)info;
{
    if (!info || [info count] == 0)
        return;
    
    for (NSDictionary *item in [_content arrangedObjects]) {
        NSUInteger group = [[item objectForKey:kHWMonitorKeyGroup] longValue];
        NSString *key = [item objectForKey:kHWMonitorKeyKey];
        
        if (group & _group) {
            if (![[_graphs allKeys] containsObject:key])
                [_graphs setObject:[[NSMutableArray alloc] init] forKey:key];
            
            NSMutableArray *points = [_graphs objectForKey:key];
            
            [points addObject:[[info objectForKey:key] objectForKey:kHWMonitorKeyRawValue]];
            
            if ([points count] > _maxPoints + 8)
                [points removeObjectAtIndex:0];
        }
    }
        
    [self calculateGraphBoundsLoopExtremes:YES];
    
    [self setNeedsDisplay:YES];
}

@end
