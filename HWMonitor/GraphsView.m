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
#import "Localizer.h"

#import "HWMGraph.h"
#import "HWMGraphsGroup.h"
#import "HWMSensorsGroup.h"
#import "HWMSensor.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"

#define LeftViewMargin      1
#define TopViewMargin       2
#define RightViewMargin     1
#define BottomViewMargin    2

@implementation GraphsView

@synthesize graphsGroup = _graphsGroup;

-(void)setGraphsGroup:(HWMGraphsGroup *)group
{
    if (group != _graphsGroup) {
        
        _graphsGroup = group;

        if ([_graphsGroup.selectors containsObject:@kHWMGroupTemperature] ||
            [_graphsGroup.selectors containsObject:@kHWMGroupSmartTemperature]) {
            _legendFormat = @"%1.0f°";
            _isTemperatureGroup = YES;
        }
        else if ([_graphsGroup.selectors containsObject:@kHWMGroupFrequency]) {
            _legendFormat = GetLocalizedString(@"%1.0f MHz");
        }
        else if ([_graphsGroup.selectors containsObject:@kHWMGroupTachometer]) {
            _legendFormat = GetLocalizedString(@"%1.0f rpm");
        }
        else if ([_graphsGroup.selectors containsObject:@kHWMGroupVoltage]) {
            _legendFormat = GetLocalizedString(@"%1.3f V");
        }
        else if ([_graphsGroup.selectors containsObject:@kHWMGroupCurrent]) {
            _legendFormat = GetLocalizedString(@"%1.3f A");
        }
        else if ([_graphsGroup.selectors containsObject:@kHWMGroupPower]) {
            _legendFormat = GetLocalizedString(@"%1.3f W");
        }
        else if ([_graphsGroup.selectors containsObject:@kHWMGroupBattery]) {
            _legendFormat = @"%1.0f%";
        }

        [self calculateGraphBoundsFindExtremes:YES];
    }
}

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

- (void)captureDataToHistoryNow;
{
    if (_graphsGroup) {

        for (HWMGraph *graph in _graphsGroup.graphs) {
            [graph captureValueToHistorySetLimit:_maxPoints];
        }

        [self calculateGraphBoundsFindExtremes:YES];

        [self setNeedsDisplay:YES];
    }
}

- (void)calculateGraphBoundsFindExtremes:(BOOL)findExtremes
{
    if (findExtremes) {
        _maxY = 0, _minY = MAXFLOAT;
        
        for (HWMGraph *graph in _graphsGroup.graphs) {

            if (graph.hidden.boolValue)
                continue;

            if (graph.history) {
                for (NSNumber *point in graph.history) {
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

    _maxPoints = self.window.windowNumber > 0 ? self.bounds.size.width / self.graphsController.monitorEngine.configuration.graphsScaleValue.doubleValue : 100;
    
    if ((_maxY == 0 && _minY == MAXFLOAT)) {
        _graphBounds = NSMakeRect(0, 0, _maxPoints, 100);
    }
    else if (_minY >= _maxY) {
        _graphBounds = NSMakeRect(0, _minY, _maxPoints, _minY + 100);
    }
    else {

        double minY = _minY <= 0 ? _minY : _minY - _minY * 0.1;
        double maxY = _maxY + _maxY * 0.1;
        
        _graphBounds = NSMakeRect(0, minY, _maxPoints, maxY - minY);
    }
}

- (NSPoint)graphPointToView:(NSPoint)point
{
    double graphScaleX = self.graphsController.monitorEngine.configuration.graphsScaleValue.doubleValue; //([self bounds].size.width - LeftViewMargin - RightViewMargin) / _graphBounds.size.width;
    double graphScaleY = ([self bounds].size.height - TopViewMargin - BottomViewMargin) / _graphBounds.size.height;

    double x = LeftViewMargin + (point.x - _graphBounds.origin.x) * graphScaleX;
    double y = BottomViewMargin + (point.y - _graphBounds.origin.y) * graphScaleY;
    
    return NSMakePoint(x, y);
}

- (void)drawRect:(NSRect)rect
{
    [self calculateGraphBoundsFindExtremes:NO];
    
    NSGraphicsContext* context = [NSGraphicsContext currentContext];
    
    [context saveGraphicsState];
    
    // Clipping rect
    [NSBezierPath clipRect:NSMakeRect(LeftViewMargin, TopViewMargin, self.bounds.size.width - LeftViewMargin - RightViewMargin, self.bounds.size.height - TopViewMargin - BottomViewMargin)];
    
    [[[NSGradient alloc]
      initWithStartingColor:[NSColor colorWithCalibratedWhite:0.15 alpha:0.85]
                endingColor:[NSColor colorWithCalibratedWhite:0.25 alpha:0.85]]
        drawInRect:self.bounds angle:270];
    
    // Draw marks
    [context setShouldAntialias:NO];
    
    __block NSBezierPath *path = [[NSBezierPath alloc] init];
    
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
    
    for (HWMGraph *graph in _graphsGroup.graphs) {
        
        if (graph.hidden.boolValue)
            continue;

        if (!graph.history || graph.history.count < 2)
            continue;
        
        [path removeAllPoints];
        [path setLineJoinStyle:NSRoundLineJoinStyle];
        
        CGFloat startOffset = /*[values count] > _maxPoints ?*/ _maxPoints - graph.history.count /*: _graphBounds.size.width - _maxPoints*/;

        __block CGFloat offset = startOffset;

        if (self.graphsController.monitorEngine.configuration.useGraphSmoothing.boolValue) {

            __block NSPoint lastPoint = NSMakePoint(startOffset, [[graph.history objectAtIndex:0] doubleValue]);
            
            [path moveToPoint:[self graphPointToView:lastPoint]];

            [graph.history enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                if (idx > 0) {
                    NSPoint nextPoint = NSMakePoint(offset + idx, [obj doubleValue]);
                    NSPoint controlPoint1 = NSMakePoint(lastPoint.x + (nextPoint.x - lastPoint.x) * 0.7, lastPoint.y + (nextPoint.y - lastPoint.y) * 0.35);
                    NSPoint controlPoint2 = NSMakePoint(lastPoint.x + (nextPoint.x - lastPoint.x) * 0.3, lastPoint.y + (nextPoint.y - lastPoint.y) * 0.65);

                    [path curveToPoint:[self graphPointToView:nextPoint]
                         controlPoint1:[self graphPointToView:controlPoint1]
                         controlPoint2:[self graphPointToView:controlPoint2]];

                    lastPoint = nextPoint;
                }
            }];
        }
        else {
            [path moveToPoint:[self graphPointToView:NSMakePoint(startOffset, [[graph.history objectAtIndex:0] doubleValue])]];
            
            [graph.history enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                if (idx > 0) {
                    NSPoint p1 = NSMakePoint(offset + idx, [obj doubleValue]);
                    [path lineToPoint:[self graphPointToView:p1]];
                }
            }];
        }
        
        if (graph == _graphsController.selectedItem) {
            [[[graph color] highlightWithLevel:0.8] set];
            [path setLineWidth:3.0];
        }
        else {
            [[graph color] set];
            [path setLineWidth:1.5];
        }
        
        [path stroke];
    }
    
    // Draw extreme values
    if (_minY < _maxY) {
        [context setShouldAntialias:YES];

        NSAttributedString *maxExtremeTitle = [[NSAttributedString alloc]
                                               initWithString:[NSString stringWithFormat:_legendFormat, _isTemperatureGroup && self.graphsController.monitorEngine.configuration.useFahrenheit.boolValue ? _maxY * (9.0f / 5.0f) + 32.0f : _maxY]
                                               attributes:_legendAttributes];

        NSAttributedString *minExtremeTitle = [[NSAttributedString alloc]
                                     initWithString:[NSString stringWithFormat:_legendFormat, _isTemperatureGroup && self.graphsController.monitorEngine.configuration.useFahrenheit.boolValue ? _minY * (9.0f / 5.0f) + 32.0f : _minY]
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
