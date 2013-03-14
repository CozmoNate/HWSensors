//
//  PopupView.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//  Based on code by Vadim Shpanovski <https://github.com/shpakovski/Popup>
//  Popup is licensed under the BSD license.
//  Copyright (c) 2013 Vadim Shpanovski, Natan Zalkin. All rights reserved.
//

#import "PopupView.h"
#import "HWMonitorDefinitions.h"

@implementation PopupView

-(void)setColorTheme:(ColorTheme*)colorTheme
{
    if (_colorTheme != colorTheme) {
        _colorTheme = colorTheme;
        _headerPath = nil;
        _headerGradient = nil;
        _contentPath = nil;
        [self setNeedsDisplay:YES];
    }
}

- (void)setArrowPosition:(CGFloat)arrowPosition
{
    if (_arrowPosition != arrowPosition) {
        _arrowPosition = arrowPosition + LINE_THICKNESS / 2.0;
        _headerPath = nil;
        _headerGradient = nil;
        _contentPath = nil;
        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    [NSGraphicsContext saveGraphicsState];
    
    NSRect contentRect = NSInsetRect([self bounds], LINE_THICKNESS, LINE_THICKNESS);
    
    if (!_headerPath || !_headerGradient || !_contentPath || !NSEqualRects(contentRect, _contentRect)) {
        
        _contentRect = contentRect;
        
        // Header
        
        NSRect headerRect = contentRect;
        headerRect.size.height = kHWMonitorToolbarHeight + ARROW_HEIGHT - LINE_THICKNESS * 3; // toolbar row height
        headerRect.origin.y = contentRect.size.height - headerRect.size.height;
        
        _headerPath = [NSBezierPath bezierPath];
        
        [_headerPath setLineWidth:LINE_THICKNESS];
        [_headerPath setLineJoinStyle:NSRoundLineJoinStyle];
        
        [_headerPath moveToPoint:NSMakePoint(_arrowPosition - ARROW_WIDTH / 2.0f, NSMaxY(headerRect) - ARROW_HEIGHT)];
        [_headerPath lineToPoint:NSMakePoint(_arrowPosition, NSMaxY(headerRect))];
        [_headerPath lineToPoint:NSMakePoint(_arrowPosition + ARROW_WIDTH / 2.0f, NSMaxY(headerRect) - ARROW_HEIGHT)];
        
        [_headerPath lineToPoint:NSMakePoint(NSMaxX(headerRect) - CORNER_RADIUS, NSMaxY(headerRect) - ARROW_HEIGHT)];
        
        NSPoint topRightCorner = NSMakePoint(NSMaxX(headerRect) - CORNER_RADIUS, NSMaxY(headerRect) - ARROW_HEIGHT - CORNER_RADIUS);
        [_headerPath appendBezierPathWithArcWithCenter:topRightCorner radius:CORNER_RADIUS startAngle:90 endAngle:0 clockwise:YES];
        
        [_headerPath lineToPoint:NSMakePoint(NSMaxX(headerRect), NSMinY(headerRect) + CORNER_RADIUS)];
        
        [_headerPath lineToPoint:NSMakePoint(NSMaxX(headerRect), NSMinY(headerRect))];
        [_headerPath lineToPoint:NSMakePoint(NSMinX(headerRect), NSMinY(headerRect))];
        
        [_headerPath lineToPoint:NSMakePoint(NSMinX(headerRect), NSMaxY(headerRect) - ARROW_HEIGHT - CORNER_RADIUS)];
        
        NSPoint topLeftCorner = NSMakePoint(NSMinX(headerRect) + CORNER_RADIUS, NSMaxY(headerRect) - ARROW_HEIGHT - CORNER_RADIUS);
        [_headerPath appendBezierPathWithArcWithCenter:topLeftCorner radius:CORNER_RADIUS startAngle:180 endAngle:90 clockwise:YES];
        
        [_headerPath closePath];        
        
        // Content
        _contentPath = [NSBezierPath bezierPath];
        
        [_contentPath moveToPoint:NSMakePoint(NSMaxX(headerRect), NSMinY(headerRect))];
        
        [_contentPath lineToPoint:NSMakePoint(NSMaxX(contentRect), NSMinY(contentRect) + CORNER_RADIUS)];
        
        NSPoint bottomRightCorner = NSMakePoint(NSMaxX(contentRect) - CORNER_RADIUS, NSMinY(contentRect) + CORNER_RADIUS);
        [_contentPath appendBezierPathWithArcWithCenter:bottomRightCorner radius:CORNER_RADIUS startAngle:0 endAngle:270 clockwise:YES];
        
        [_contentPath lineToPoint:NSMakePoint(NSMinX(contentRect) + CORNER_RADIUS, NSMinY(contentRect))];
        
        NSPoint bottomLeftCorner = NSMakePoint(NSMinX(contentRect) + CORNER_RADIUS, NSMinY(contentRect) + CORNER_RADIUS);
        [_contentPath appendBezierPathWithArcWithCenter:bottomLeftCorner radius:CORNER_RADIUS startAngle:270 endAngle:180 clockwise:YES];
        
        [_contentPath lineToPoint:NSMakePoint(NSMinX(headerRect), NSMinY(headerRect))];
        
        [_contentPath closePath];
        [_contentPath setLineWidth:LINE_THICKNESS];
        //[_contentPath setFlatness:0.3];
        
        _headerGradient = [[NSGradient alloc]
                           initWithStartingColor:   _colorTheme.barBackgroundStartColor
                           endingColor:             _colorTheme.barBackgroundEndColor];
        
    }
    
    // Draw panel
    
    [_headerGradient drawInBezierPath:_headerPath angle:270];
    
    [_colorTheme.listBackgroundColor setFill];
    [_contentPath fill];
    
    NSBezierPath *clip = [NSBezierPath bezierPathWithRect:[self bounds]];
    [clip appendBezierPath:_headerPath];
    [clip appendBezierPath:_contentPath];
    [clip addClip];
    
    [_colorTheme.barPathColor setStroke];
    [_headerPath stroke];
    
    [_colorTheme.listPathColor setStroke];
    [_contentPath stroke];
    
    [NSGraphicsContext restoreGraphicsState];
}

@end
