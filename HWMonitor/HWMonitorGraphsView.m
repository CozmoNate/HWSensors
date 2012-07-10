//
//  HWMonitorGraphsView.m
//  HWSensors
//
//  Created by kozlek on 07.07.12.
//  Copyright (c) 2012 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//
//

#import "HWMonitorGraphsView.h"
#import "HWMonitorSensor.h"
#import "HWMonitorDefinitions.h"

@implementation HWMonitorGraphsView

@synthesize group = _group;
@synthesize content = _content;

#define kHWMonitorGraphsHistoryPoints  120

-(void)setGroup:(NSUInteger)group
{
    _group = group;
    [self calculateGraphBounds];
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
    
    return self;
}

- (void)calculateGraphBounds
{
    double maxY = 0, minY = MAXFLOAT;
    
    for (NSDictionary *item in [_content arrangedObjects]) {
        NSNumber *enabled = [item objectForKey:kHWMonitorKeyEnabled];
        
        if (enabled && [enabled boolValue]) {
            NSString *key = [item objectForKey:kHWMonitorKeyKey];
            NSArray *points = [_graphs objectForKey:key];
            
            if (points) {
                for (NSNumber *point in points) {
                    if ([point doubleValue] < minY) {
                        minY = [point doubleValue];
                    }
                    else if ([point doubleValue] > maxY)
                    {
                        maxY = [point doubleValue];
                    }
                }
            }
        }
    }
    
    if (maxY == 0 && minY == MAXFLOAT) {
        if (_group & kHWSensorGroupTemperature || _group & kSMARTSensorGroupTemperature) {
            _graphBounds = NSMakeRect(0, 20, kHWMonitorGraphsHistoryPoints, 99);
        }
        else if (_group & kHWSensorGroupFrequency) {
            _graphBounds = NSMakeRect(0, 0, kHWMonitorGraphsHistoryPoints, 4000);
        }
        else if (_group & kHWSensorGroupTachometer) {
            _graphBounds =  NSMakeRect(0, 0, kHWMonitorGraphsHistoryPoints, 3000);
        }
        else if (_group & kHWSensorGroupVoltage) {
            _graphBounds = NSMakeRect(0, 0, kHWMonitorGraphsHistoryPoints, 15);
        }
        else {
            _graphBounds = NSMakeRect(0, 0, kHWMonitorGraphsHistoryPoints, 100);
        }
    }
    else {
        double scaleY = maxY - minY;
        
        if (scaleY == 0)
            scaleY = 1;
        
        _graphBounds = NSMakeRect(0, minY - scaleY * 0.35, kHWMonitorGraphsHistoryPoints, scaleY * 1.45);
    }
}

#define ViewMargin 5

- (NSPoint)graphPointToView:(NSPoint)point
{
    double graphScaleX = ([self bounds].size.width - ViewMargin * 2) / _graphBounds.size.width;
    double graphScaleY = ([self bounds].size.height - ViewMargin * 2) / _graphBounds.size.height;
    double x = ViewMargin + (point.x - _graphBounds.origin.x) * graphScaleX;
    double y = ViewMargin + (point.y - _graphBounds.origin.y) * graphScaleY;
    
    return NSMakePoint(x, y);
}

- (CGFloat) bWithIndex:(NSUInteger)i time:(float)t {
    switch (i) {
        case -2:
            return (((-t + 3) * t - 3) * t + 1) / 6;
        case -1:
            return (((3 * t - 6) * t) * t + 4) / 6;
        case 0:
            return (((-3 * t + 3) * t + 3) * t + 1) / 6;
        case 1:
            return (t * t * t) / 6;
    }
    return 0;
}

- (NSPoint) splinePointWithPoints:(NSArray*)points index:(NSUInteger)i time:(float)t {
    CGFloat px = 0;
    CGFloat py = 0;
    for (int j = -2; j <= 1; j++) {
        px += [self bWithIndex:j time:t] * (i + j);
        py += [self bWithIndex:j time:t] * [[points objectAtIndex:i + j] doubleValue];
    }
    return NSMakePoint(px, py);
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor blackColor] set];
    [NSBezierPath fillRect:[self bounds]];
    
    double x, y;

    NSGraphicsContext* context = [NSGraphicsContext currentContext];
    
    [context saveGraphicsState];
    
    [context setShouldAntialias:NO];
    
    NSBezierPath *path = [[NSBezierPath alloc] init];
    
    [path setLineWidth:0.33];
    
    [[NSColor colorWithCalibratedRed:0 green:0.25 blue:0 alpha:0.7] set];
    
    for (x = _graphBounds.origin.x; x < _graphBounds.origin.x + _graphBounds.size.width; x += _graphBounds.size.width / 15) {
        [path removeAllPoints];
        [path moveToPoint:[self graphPointToView:NSMakePoint(x,_graphBounds.origin.y)]];
        [path lineToPoint:[self graphPointToView:NSMakePoint(x,_graphBounds.origin.y + _graphBounds.size.height)]];
        [path stroke];
        
        for (y = _graphBounds.origin.y; y < _graphBounds.origin.y + _graphBounds.size.height; y += _graphBounds.size.height / 7) {
            [path removeAllPoints];
            [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x,y)]];
            [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width,y)]];
            [path stroke];
        }
    }
    
    // Draw graphs
    
    [context setShouldAntialias:YES];
    
    for (NSDictionary *item in [_content arrangedObjects]) {
        NSNumber *enabled = [item objectForKey:kHWMonitorKeyEnabled];
        
        if (enabled && [enabled boolValue]) {
            NSString *key = [item objectForKey:kHWMonitorKeyKey];
            NSArray *points = [_graphs objectForKey:key];
                        
            if (points && [points count] >= 2) {
                NSColor *color = nil;
                
                if ([[_content selectedObjects] containsObject:item]) {
                    color = [NSColor whiteColor];
                    [path setLineWidth:3.5];
                }
                else {
                    color = [item objectForKey:kHWMonitorKeyColor];
                    [path setLineWidth:2];
                    [path setLineJoinStyle:NSRoundLineJoinStyle];
                }
                
                [path removeAllPoints];
                
                [color set];
                
                NSPoint topLeft = [self graphPointToView:NSMakePoint(_graphBounds.origin.x, _graphBounds.origin.y)];
                NSPoint bottomRight = [self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width, _graphBounds.origin.y + _graphBounds.size.height)];
                
                [NSBezierPath clipRect:NSMakeRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y)];
                                
                [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.size.width - [points count], [[points objectAtIndex:0] doubleValue])]];
                                
                for (NSUInteger index = 2; index + 1 < [points count]; index++) {
                    
                    NSPoint p1 = [self splinePointWithPoints:points index:index time:0.33];
                    NSPoint p2 = [self splinePointWithPoints:points index:index time:0.66];
                    NSPoint q2 = [self splinePointWithPoints:points index:index time:1.00];
                    
                    [path curveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.size.width - [points count] + 2 + q2.x, q2.y)]
                         controlPoint1:[self graphPointToView:NSMakePoint(_graphBounds.size.width - [points count] + 2 + p1.x, p1.y)]
                         controlPoint2:[self graphPointToView:NSMakePoint(_graphBounds.size.width - [points count] + 2 + p2.x, p2.y)]];
                }
                
                [path stroke];
            }
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
            
            if ([points count] > kHWMonitorGraphsHistoryPoints + 4)
                [points removeObjectAtIndex:0];
        }
    }
        
    [self calculateGraphBounds];
    
    [self setNeedsDisplay:YES];
}

@end
