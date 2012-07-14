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

@synthesize graphs = _content;
@synthesize group = _group;
@synthesize useFahrenheit = _useFahrenheit;

#define kHWMonitorGraphsHistoryPoints  120

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
    
    _legendAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                         [NSFont systemFontOfSize:9.0], NSFontAttributeName,
                         [NSColor yellowColor], NSForegroundColorAttributeName,
                         nil];
    
    _legendFormat = @"%1.0f";
    
    return self;
}

- (void)calculateGraphBounds
{
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
    
    if (_maxY == 0 && _minY == MAXFLOAT) {
        _graphBounds = NSMakeRect(0, 0, kHWMonitorGraphsHistoryPoints, 100);
    }
    else {

        double minY = _minY - (_maxY - _minY) * 0.15, maxY = _maxY * 1.15;
        
        if (_group & kHWSensorGroupTemperature || _group & kSMARTSensorGroupTemperature) {
            minY = minY < 25 ? minY : 25;
            maxY = maxY < 25 ? 25 : maxY;
        }
        else if (_group & kHWSensorGroupFrequency) {
            //minY = 0;
        }
        else if (_group & kHWSensorGroupTachometer) {
            minY = minY < 1000 ? minY : 1000;
            maxY = maxY < 1000 ? 1000 : maxY;
        }
        else if (_group & kHWSensorGroupVoltage) {
            //minY = 0;
        }
        
        _graphBounds = NSMakeRect(0, minY, kHWMonitorGraphsHistoryPoints, maxY - minY);
    }
    
    NSPoint topLeft = [self graphPointToView:NSMakePoint(_graphBounds.origin.x, _graphBounds.origin.y)];
    NSPoint bottomRight = [self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width, _graphBounds.origin.y + _graphBounds.size.height)];
    
   [NSBezierPath clipRect:NSMakeRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y)];
}

#define LeftViewMargin 5
#define TopViewMargin 5
#define RightViewMargin 5
#define BottomViewMargin 5

- (NSPoint)graphPointToView:(NSPoint)point
{
    double graphScaleX = ([self bounds].size.width - LeftViewMargin - RightViewMargin) / _graphBounds.size.width;
    double graphScaleY = ([self bounds].size.height - TopViewMargin - BottomViewMargin) / _graphBounds.size.height;
    double x = LeftViewMargin + (point.x - _graphBounds.origin.x) * graphScaleX;
    double y = BottomViewMargin + (point.y - _graphBounds.origin.y) * graphScaleY;
    
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
    //[[NSColor colorWithCalibratedRed:0.85 green:0.9 blue:0.95 alpha:1.0] setFill];
    [[NSColor blackColor] setFill];
    [NSBezierPath fillRect:[self bounds]];
    
    double x, y;

    NSGraphicsContext* context = [NSGraphicsContext currentContext];
    
    [context saveGraphicsState];
    
    
    // Draw marks
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
    
    // Draw extremes
    [context setShouldAntialias:NO];
    
    [path removeAllPoints];
    [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x,_maxY)]];
    [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width,_maxY)]];
    [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x,_minY)]];
    [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.origin.x + _graphBounds.size.width,_minY)]];
    CGFloat pattern[2] = { 4.0, 4.0 };
    [path setLineDash:pattern count:2 phase:1.0];
    [[NSColor lightGrayColor] set];
    [path setLineWidth:0.5];
    [path stroke];
    CGFloat resetPattern[1] = { 0 };
    [path setLineDash:resetPattern count:0 phase:0];
    
    [context setShouldAntialias:YES];
    
    NSAttributedString *title = [[NSAttributedString alloc]
                                 initWithString:[NSString stringWithFormat:_legendFormat, ((_group & kHWSensorGroupTemperature || _group & kSMARTSensorGroupTemperature) && _useFahrenheit ? _minY * (9.0f / 5.0f) + 32.0f : _minY )]
                                 attributes:_legendAttributes];
    [title drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _minY)].y - [title size].height - 2)];
    
    title = [[NSAttributedString alloc]
             initWithString:[NSString stringWithFormat:_legendFormat, ((_group & kHWSensorGroupTemperature || _group & kSMARTSensorGroupTemperature) && _useFahrenheit ? _maxY * (9.0f / 5.0f) + 32.0f : _maxY )]
             attributes:_legendAttributes];
    [title drawAtPoint:NSMakePoint(LeftViewMargin + 2, [self graphPointToView:NSMakePoint(0, _maxY)].y + 2)];
    
    
    // Draw graphs
    
    [context setShouldAntialias:YES];
    [path setLineJoinStyle:NSMiterLineJoinStyle];
        
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
                    NSColor *itemColor = [item objectForKey:kHWMonitorKeyColor];
                    color = [NSColor colorWithCalibratedRed:itemColor.redComponent green:itemColor.greenComponent blue:itemColor.blueComponent alpha:1.00];
                    [path setLineWidth:2];
                }
                
                [path removeAllPoints];
                
                [color set];
                                
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
