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

#define GraphHistoryPoints  120

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
            _graphBounds = NSMakeRect(0, 20, GraphHistoryPoints, 99);
        }
        else if (_group & kHWSensorGroupFrequency) {
            _graphBounds = NSMakeRect(0, 0, GraphHistoryPoints, 4000);
        }
        else if (_group & kHWSensorGroupTachometer) {
            _graphBounds =  NSMakeRect(0, 0, GraphHistoryPoints, 3000);
        }
        else if (_group & kHWSensorGroupVoltage) {
            _graphBounds = NSMakeRect(0, 0, GraphHistoryPoints, 15);
        }
        else {
            _graphBounds = NSMakeRect(0, 0, GraphHistoryPoints, 100);
        }
    }
    else {
        double scaleY = maxY - minY;
        
        if (scaleY == 0)
            scaleY = 1;
        
        _graphBounds = NSMakeRect(0, minY - (maxY - minY) * 0.1, GraphHistoryPoints, scaleY * 1.2);
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
                        
            if (points && [points count] > 3) {
                NSColor *color = nil;
                
                if ([[_content selectedObjects] containsObject:item]) {
                    color = [NSColor whiteColor];
                    [path setLineWidth:3];
                }
                else {
                    color = [item objectForKey:kHWMonitorKeyColor];
                    [path setLineWidth:1.5];
                }
                
                [path removeAllPoints];
                
                [color set];
                
                [path moveToPoint:[self graphPointToView:NSMakePoint(_graphBounds.size.width - [points count], [[points objectAtIndex:0] doubleValue])]];
                
                NSUInteger index;
                
                for (index = 1; index < [points count]; index++)
                    [path lineToPoint:[self graphPointToView:NSMakePoint(_graphBounds.size.width - [points count] + index, [[points objectAtIndex:index] floatValue])]];
                
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
            
            if ([points count] > GraphHistoryPoints)
                [points removeObjectAtIndex:0];
        }
    }
        
    [self calculateGraphBounds];
    
    [self setNeedsDisplay:YES];
}

@end
