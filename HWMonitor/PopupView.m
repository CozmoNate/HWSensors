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
        _cachedImage = nil;
        [self setNeedsDisplay:YES];
    }
}

- (void)setArrowPosition:(CGFloat)arrowPosition
{
    if (_arrowPosition != arrowPosition) {
        _arrowPosition = arrowPosition + LINE_THICKNESS / 2.0;
        _cachedImage = nil;
        [self setNeedsDisplay:YES];
    }
}

- (NSBezierPath*)toolbarShapeWithBounds:(NSRect)shapeBounds
{
    NSBezierPath *toolbarPath = [NSBezierPath bezierPath];
    
    [toolbarPath setLineWidth:LINE_THICKNESS];
    [toolbarPath setLineJoinStyle:NSRoundLineJoinStyle];
    
    [toolbarPath moveToPoint:NSMakePoint(NSMinX(shapeBounds), NSMinY(shapeBounds))];
    [toolbarPath lineToPoint:NSMakePoint(NSMinX(shapeBounds), NSMaxY(shapeBounds) - ARROW_HEIGHT - CORNER_RADIUS)];
    
    NSPoint topLeftCorner = NSMakePoint(NSMinX(shapeBounds) + CORNER_RADIUS, NSMaxY(shapeBounds) - ARROW_HEIGHT - CORNER_RADIUS);
    [toolbarPath appendBezierPathWithArcWithCenter:topLeftCorner radius:CORNER_RADIUS startAngle:180 endAngle:90 clockwise:YES];
    
    [toolbarPath lineToPoint:NSMakePoint(_arrowPosition - ARROW_WIDTH / 2.0f, NSMaxY(shapeBounds) - ARROW_HEIGHT)];
    [toolbarPath lineToPoint:NSMakePoint(_arrowPosition, NSMaxY(shapeBounds))];
    [toolbarPath lineToPoint:NSMakePoint(_arrowPosition + ARROW_WIDTH / 2.0f, NSMaxY(shapeBounds) - ARROW_HEIGHT)];
    
    [toolbarPath lineToPoint:NSMakePoint(NSMaxX(shapeBounds) - CORNER_RADIUS, NSMaxY(shapeBounds) - ARROW_HEIGHT)];
    
    NSPoint topRightCorner = NSMakePoint(NSMaxX(shapeBounds) - CORNER_RADIUS, NSMaxY(shapeBounds) - ARROW_HEIGHT - CORNER_RADIUS);
    [toolbarPath appendBezierPathWithArcWithCenter:topRightCorner radius:CORNER_RADIUS startAngle:90 endAngle:0 clockwise:YES];
    
    [toolbarPath lineToPoint:NSMakePoint(NSMaxX(shapeBounds), NSMinY(shapeBounds))];
    
    return toolbarPath;
}

- (void)drawRect:(NSRect)rect
{
    NSRect popupBounds = NSInsetRect([self bounds], LINE_THICKNESS, LINE_THICKNESS);
    
    if (!_cachedImage || !NSEqualRects(popupBounds, _popupBounds)) {
        _popupBounds = popupBounds;
        
        _cachedImage = [[NSImage alloc] initWithSize:[self bounds].size];
        
        [_cachedImage lockFocus];
        
        // Toolbar and arrow
        NSRect toolbarRect = popupBounds;
        toolbarRect.size.height = kHWMonitorToolbarHeight + ARROW_HEIGHT - LINE_THICKNESS * 3;
        toolbarRect.origin.y = popupBounds.size.height - toolbarRect.size.height;
        
        NSBezierPath *toolbarPath = [self toolbarShapeWithBounds:toolbarRect];
        NSBezierPath *toolbarStroke = [self toolbarShapeWithBounds:NSInsetRect(toolbarRect, -LINE_THICKNESS, -LINE_THICKNESS)];
        
        // List
        toolbarRect.origin.y -= LINE_THICKNESS;
        
        NSBezierPath *listPath = [NSBezierPath bezierPath];
        
        [listPath moveToPoint:NSMakePoint(NSMaxX(toolbarRect), NSMinY(toolbarRect))];
        
        [listPath lineToPoint:NSMakePoint(NSMaxX(popupBounds), NSMinY(popupBounds) + CORNER_RADIUS)];
        
        NSPoint bottomRightCorner = NSMakePoint(NSMaxX(popupBounds) - CORNER_RADIUS, NSMinY(popupBounds) + CORNER_RADIUS);
        [listPath appendBezierPathWithArcWithCenter:bottomRightCorner radius:CORNER_RADIUS startAngle:0 endAngle:270 clockwise:YES];
        
        [listPath lineToPoint:NSMakePoint(NSMinX(popupBounds) + CORNER_RADIUS, NSMinY(popupBounds))];
        
        NSPoint bottomLeftCorner = NSMakePoint(NSMinX(popupBounds) + CORNER_RADIUS, NSMinY(popupBounds) + CORNER_RADIUS);
        [listPath appendBezierPathWithArcWithCenter:bottomLeftCorner radius:CORNER_RADIUS startAngle:270 endAngle:180 clockwise:YES];
        
        [listPath lineToPoint:NSMakePoint(NSMinX(toolbarRect), NSMinY(toolbarRect))];
        
        // Draw toolbar
        [[[NSGradient alloc] initWithStartingColor:[_colorTheme.toolbarStartColor shadowWithLevel:0.33] endingColor:[_colorTheme.toolbarEndColor shadowWithLevel:0.4]] drawInBezierPath:toolbarStroke angle:270];
        [[[NSGradient alloc] initWithStartingColor:_colorTheme.toolbarStartColor endingColor:_colorTheme.toolbarEndColor] drawInBezierPath:toolbarPath angle:270];
        
        // Fill list
        [_colorTheme.listBackgroundColor setFill];
        [listPath fill];
        
        [NSGraphicsContext saveGraphicsState];
            
        NSBezierPath *clipPath = [NSBezierPath bezierPathWithRect:self.bounds];
        [clipPath appendBezierPath:toolbarPath];
        [clipPath appendBezierPath:listPath];
        [clipPath setLineWidth:LINE_THICKNESS];
        [clipPath addClip];
        
        // Stroke list
        [[_colorTheme.listBackgroundColor shadowWithLevel:0.8] set];
        //[listPath closePath];
        [listPath setLineWidth:LINE_THICKNESS * 1.5];
        [listPath stroke];
        
        [NSGraphicsContext restoreGraphicsState];
        
        // Draw inner shadow on toolbar
        [NSGraphicsContext saveGraphicsState];
        [_colorTheme.toolbarShadowColor setStroke];
        [NSBezierPath clipRect:NSInsetRect(toolbarPath.bounds, CORNER_RADIUS * 0.3, 0)];
        [toolbarPath addClip];
        [toolbarPath setLineWidth:toolbarPath.lineWidth + 0.2];
        [toolbarPath stroke];
        [NSGraphicsContext restoreGraphicsState];
                
        [_cachedImage unlockFocus];
    }
    
    [_cachedImage drawInRect:rect fromRect:NSOffsetRect(rect,-self.bounds.origin.x,-self.bounds.origin.y) operation:NSCompositeSourceOver fraction:1.0];
}

@end
