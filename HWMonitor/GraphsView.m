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
#import "HWMValueFormatter.h"

#define LeftViewMargin      1
#define TopViewMargin       2
#define RightViewMargin     1
#define BottomViewMargin    2

@implementation GraphsView

@synthesize graphsGroup = _graphsGroup;

-(void)setGraphsGroup:(HWMGraphsGroup *)group
{
    if (group != _graphsGroup) {
        
        if (_graphsGroup) {
            [[NSNotificationCenter defaultCenter] removeObserver:self name:HWMGraphsGroupHistoryHasBeenChangedNotification object:_graphsGroup];
            
            _graphsGroup = nil;
        }
        
        _graphsGroup = group;
        
        if (_graphsGroup) {
            
            [self calculateGraphBounds];
            
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(dataHasBeenCapturedToHistoryNow) name:HWMGraphsGroupHistoryHasBeenChangedNotification object:_graphsGroup];
        }
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
    }
    
    return self;
}

-(void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:HWMGraphsGroupHistoryHasBeenChangedNotification object:_graphsGroup];
}

- (void)dataHasBeenCapturedToHistoryNow
{
    [self calculateGraphBounds];
    [self setNeedsDisplay:YES];
}

- (void)calculateGraphBounds
{
    _maxPoints = self.window.windowNumber > 0 ? self.frame.size.width / self.graphsController.monitorEngine.configuration.graphsScaleValue.doubleValue : 100;
    
    if ((!_graphsGroup.maxGraphsValue && !_graphsGroup.minGraphsValue)) {
        _graphBounds = NSMakeRect(0, 0, _maxPoints, 100);
    }
    else if ([_graphsGroup.minGraphsValue isGreaterThanOrEqualTo:_graphsGroup.maxGraphsValue]) {
        _graphBounds = NSMakeRect(0, _graphsGroup.minGraphsValue.doubleValue, _maxPoints, _graphsGroup.minGraphsValue.doubleValue + 100);
    }
    else {

        double minY = _graphsGroup.minGraphsValue.doubleValue <= 0 ? _graphsGroup.minGraphsValue.doubleValue : _graphsGroup.minGraphsValue.doubleValue - _graphsGroup.minGraphsValue.doubleValue * 0.20;
        double maxY = _graphsGroup.maxGraphsValue.doubleValue + _graphsGroup.maxGraphsValue.doubleValue * 0.10;
        
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
    [self calculateGraphBounds];
    
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
    
    if (_graphsGroup.minGraphsValue && _graphsGroup.maxGraphsValue && [_graphsGroup.minGraphsValue isLessThan:_graphsGroup.maxGraphsValue]) {
        // Draw extremums
        [context setShouldAntialias:NO];
        
        [path removeAllPoints];
        [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x,_graphsGroup.maxGraphsValue.doubleValue)]];
        [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width,_graphsGroup.maxGraphsValue.doubleValue)]];
        [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x,_graphsGroup.minGraphsValue.doubleValue)]];
        [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width,_graphsGroup.minGraphsValue.doubleValue)]];
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
    if (_graphsGroup.minGraphsValue && _graphsGroup.maxGraphsValue && [_graphsGroup.minGraphsValue isLessThan:_graphsGroup.maxGraphsValue]) {
        [context setShouldAntialias:YES];

        NSAttributedString *maxExtremeTitle = [[NSAttributedString alloc]
                                               initWithString:[HWMValueFormatter formattedValue:_graphsGroup.maxGraphsValue usingRulesOfGroup:[_graphsGroup.selectors objectAtIndex:0] configuration:_graphsGroup.configuration]
                                               attributes:_legendAttributes];

        NSAttributedString *minExtremeTitle = [[NSAttributedString alloc]
                                     initWithString:[HWMValueFormatter formattedValue:_graphsGroup.minGraphsValue usingRulesOfGroup:[_graphsGroup.selectors objectAtIndex:0] configuration:_graphsGroup.configuration]
                                     attributes:_legendAttributes];

        if ([self graphPointToView:NSMakePoint(0, _graphsGroup.maxGraphsValue.doubleValue)].y + 2 + [maxExtremeTitle size].height > [self graphPointToView:NSMakePoint(0, _graphBounds.origin.y + _graphBounds.size.height)].y || [self graphPointToView:NSMakePoint(0, _graphsGroup.minGraphsValue.doubleValue)].y - [minExtremeTitle size].height < [self graphPointToView:_graphBounds.origin].y) {
            [maxExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _graphsGroup.maxGraphsValue.doubleValue)].y - [maxExtremeTitle size].height)];
            [minExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _graphsGroup.minGraphsValue.doubleValue)].y + 2)];
        }
        else {
            [maxExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _graphsGroup.maxGraphsValue.doubleValue)].y + 2)];
            [minExtremeTitle drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _graphsGroup.minGraphsValue.doubleValue)].y - [minExtremeTitle size].height)];
        }
    }
    
    [context restoreGraphicsState];
}

@end
