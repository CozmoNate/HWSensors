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
        _toolbarPath = nil;
        _toolbarGradient = nil;
        _listPath = nil;
        [self setNeedsDisplay:YES];
    }
}

- (void)setArrowPosition:(CGFloat)arrowPosition
{
    if (_arrowPosition != arrowPosition) {
        _arrowPosition = arrowPosition + LINE_THICKNESS / 2.0;
        _toolbarPath = nil;
        _toolbarGradient = nil;
        _listPath = nil;
        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect popupBounds = NSInsetRect([self bounds], LINE_THICKNESS, LINE_THICKNESS);
    
    if (!_toolbarPath || !_toolbarGradient || !_listPath || !_clipPath || !NSEqualRects(popupBounds, _popupBounds)) {
        
        _popupBounds = popupBounds;
        
        // Toolbar and arrow
        NSRect toolbarRect = popupBounds;
        toolbarRect.size.height = kHWMonitorToolbarHeight + ARROW_HEIGHT - LINE_THICKNESS * 2; // toolbar row height
        toolbarRect.origin.y = popupBounds.size.height - toolbarRect.size.height;
        
        _toolbarPath = [NSBezierPath bezierPath];
                
        [_toolbarPath setLineWidth:LINE_THICKNESS];
        [_toolbarPath setLineJoinStyle:NSRoundLineJoinStyle];
        
        [_toolbarPath moveToPoint:NSMakePoint(NSMinX(toolbarRect), NSMinY(toolbarRect))];
        [_toolbarPath lineToPoint:NSMakePoint(NSMinX(toolbarRect), NSMaxY(toolbarRect) - ARROW_HEIGHT - CORNER_RADIUS)];
        
        NSPoint topLeftCorner = NSMakePoint(NSMinX(toolbarRect) + CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT - CORNER_RADIUS);
        [_toolbarPath appendBezierPathWithArcWithCenter:topLeftCorner radius:CORNER_RADIUS startAngle:180 endAngle:90 clockwise:YES];
        
        [_toolbarPath lineToPoint:NSMakePoint(_arrowPosition - ARROW_WIDTH / 2.0f, NSMaxY(toolbarRect) - ARROW_HEIGHT)];
        [_toolbarPath lineToPoint:NSMakePoint(_arrowPosition, NSMaxY(toolbarRect))];
        [_toolbarPath lineToPoint:NSMakePoint(_arrowPosition + ARROW_WIDTH / 2.0f, NSMaxY(toolbarRect) - ARROW_HEIGHT)];
        
        [_toolbarPath lineToPoint:NSMakePoint(NSMaxX(toolbarRect) - CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT)];
        
        NSPoint topRightCorner = NSMakePoint(NSMaxX(toolbarRect) - CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT - CORNER_RADIUS);
        [_toolbarPath appendBezierPathWithArcWithCenter:topRightCorner radius:CORNER_RADIUS startAngle:90 endAngle:0 clockwise:YES];
        
        [_toolbarPath lineToPoint:NSMakePoint(NSMaxX(toolbarRect), NSMinY(toolbarRect))];
        
        // List
        _listPath = [NSBezierPath bezierPath];
        
        //[_listPath appendBezierPath:_toolbarPath];
         
        [_listPath moveToPoint:NSMakePoint(NSMaxX(toolbarRect), NSMinY(toolbarRect))];
        
        [_listPath lineToPoint:NSMakePoint(NSMaxX(popupBounds), NSMinY(popupBounds) + CORNER_RADIUS)];
        
        NSPoint bottomRightCorner = NSMakePoint(NSMaxX(popupBounds) - CORNER_RADIUS, NSMinY(popupBounds) + CORNER_RADIUS);
        [_listPath appendBezierPathWithArcWithCenter:bottomRightCorner radius:CORNER_RADIUS startAngle:0 endAngle:270 clockwise:YES];
        
        [_listPath lineToPoint:NSMakePoint(NSMinX(popupBounds) + CORNER_RADIUS, NSMinY(popupBounds))];
        
        NSPoint bottomLeftCorner = NSMakePoint(NSMinX(popupBounds) + CORNER_RADIUS, NSMinY(popupBounds) + CORNER_RADIUS);
        [_listPath appendBezierPathWithArcWithCenter:bottomLeftCorner radius:CORNER_RADIUS startAngle:270 endAngle:180 clockwise:YES];
        
        [_listPath lineToPoint:NSMakePoint(NSMinX(toolbarRect), NSMinY(toolbarRect))];
        
        //[_listPath closePath];
        
        _toolbarGradient = [[NSGradient alloc] initWithStartingColor:_colorTheme.barBackgroundStartColor endingColor:_colorTheme.barBackgroundEndColor];
        
        _clipPath = [NSBezierPath bezierPathWithRect:[self bounds]];
        [_clipPath appendBezierPath:_toolbarPath];
        [_clipPath appendBezierPath:_listPath];
    }
    
    // Fill toolbar
    [_toolbarGradient drawInBezierPath:_toolbarPath angle:270];
    
    // Fill list
    [_colorTheme.listBackgroundColor setFill];
    [_listPath fill];
    
    [NSGraphicsContext saveGraphicsState];
    
    [_clipPath addClip];
    
    // Stroke list
    [_colorTheme.listPathColor setStroke];
    [_listPath setLineWidth:LINE_THICKNESS];
    [_listPath stroke];
    
    // Stroke toolbar
    [_colorTheme.barPathColor setStroke];
    [_toolbarPath setLineWidth:LINE_THICKNESS];
    [_toolbarPath stroke];
    
    [NSGraphicsContext restoreGraphicsState];
    
    // Draw inner shadow
    [NSGraphicsContext saveGraphicsState];
    
    [[NSColor colorWithCalibratedWhite:1.0 alpha:0.3] setStroke];
    [NSBezierPath clipRect:NSMakeRect(
                                      _toolbarPath.bounds.origin.x + CORNER_RADIUS * 0.3,
                                      _toolbarPath.bounds.origin.y - CORNER_RADIUS * 0.3,
                                      _toolbarPath.bounds.size.width - CORNER_RADIUS * 0.6,
                                      _toolbarPath.bounds.size.height + CORNER_RADIUS * 0.3)];
    
    [_toolbarPath addClip];
    [_toolbarPath setLineWidth:LINE_THICKNESS * 2.5];
    [_toolbarPath stroke];

    [NSGraphicsContext restoreGraphicsState];
}

@end
