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
#import "PopupController.h"
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
        
        NSInsetRect(toolbarRect, 0.5, 0.5);
        
        NSBezierPath *toolbarPath = [NSBezierPath bezierPath];
        
        [toolbarPath setLineWidth:LINE_THICKNESS];
        [toolbarPath setLineJoinStyle:NSRoundLineJoinStyle];
        
        [toolbarPath moveToPoint:NSMakePoint(NSMinX(toolbarRect), NSMinY(toolbarRect))];
        [toolbarPath lineToPoint:NSMakePoint(NSMinX(toolbarRect), NSMaxY(toolbarRect) - ARROW_HEIGHT - CORNER_RADIUS)];
        
        NSPoint topLeftCorner = NSMakePoint(NSMinX(toolbarRect) + CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT - CORNER_RADIUS);
        [toolbarPath appendBezierPathWithArcWithCenter:topLeftCorner radius:CORNER_RADIUS startAngle:180 endAngle:90 clockwise:YES];
        
        [toolbarPath lineToPoint:NSMakePoint(_arrowPosition - ARROW_WIDTH / 2.0f, NSMaxY(toolbarRect) - ARROW_HEIGHT)];
        [toolbarPath lineToPoint:NSMakePoint(_arrowPosition, NSMaxY(toolbarRect))];
        [toolbarPath lineToPoint:NSMakePoint(_arrowPosition + ARROW_WIDTH / 2.0f, NSMaxY(toolbarRect) - ARROW_HEIGHT)];
        
        [toolbarPath lineToPoint:NSMakePoint(NSMaxX(toolbarRect) - CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT)];
        
        NSPoint topRightCorner = NSMakePoint(NSMaxX(toolbarRect) - CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT - CORNER_RADIUS);
        [toolbarPath appendBezierPathWithArcWithCenter:topRightCorner radius:CORNER_RADIUS startAngle:90 endAngle:0 clockwise:YES];
        
        [toolbarPath lineToPoint:NSMakePoint(NSMaxX(toolbarRect), NSMinY(toolbarRect))];
        //    [toolbarPath lineToPoint:NSMakePoint(NSMinX(shapeBounds), NSMinY(shapeBounds))];
        //    
        //    [toolbarPath closePath];
        
        // List
        
        NSBezierPath *listPath = [NSBezierPath bezierPath];
        
        [listPath moveToPoint:NSMakePoint(NSMaxX(toolbarRect), NSMinY(toolbarRect))];
        
        [listPath lineToPoint:NSMakePoint(NSMaxX(popupBounds), NSMinY(popupBounds) + CORNER_RADIUS)];
        
        NSPoint bottomRightCorner = NSMakePoint(NSMaxX(popupBounds) - CORNER_RADIUS, NSMinY(popupBounds) + CORNER_RADIUS);
        [listPath appendBezierPathWithArcWithCenter:bottomRightCorner radius:CORNER_RADIUS startAngle:0 endAngle:270 clockwise:YES];
        
        [listPath lineToPoint:NSMakePoint(NSMinX(popupBounds) + CORNER_RADIUS, NSMinY(popupBounds))];
        
        NSPoint bottomLeftCorner = NSMakePoint(NSMinX(popupBounds) + CORNER_RADIUS, NSMinY(popupBounds) + CORNER_RADIUS);
        [listPath appendBezierPathWithArcWithCenter:bottomLeftCorner radius:CORNER_RADIUS startAngle:270 endAngle:180 clockwise:YES];
        
        [listPath lineToPoint:NSMakePoint(NSMinX(toolbarRect), NSMinY(toolbarRect))];
       
        // Fill list
        [_colorTheme.listBackgroundColor setFill];
        [listPath fill];
        
        // Draw toolbar
        [[[NSGradient alloc] initWithStartingColor:_colorTheme.toolbarStartColor endingColor:_colorTheme.toolbarEndColor] drawInBezierPath:toolbarPath angle:270];
        
        [NSGraphicsContext saveGraphicsState];
            
        NSBezierPath *clipPath = [NSBezierPath bezierPathWithRect:self.bounds];
        [clipPath setLineWidth:LINE_THICKNESS];
        [clipPath appendBezierPath:toolbarPath];
        [clipPath appendBezierPath:listPath];
        [clipPath addClip];

        // Stroke toolbar
        [_colorTheme.toolbarStrokeColor setStroke];
        [toolbarPath setLineWidth:LINE_THICKNESS + 0.5];
        [toolbarPath stroke];
        
        // Stroke list
        [_colorTheme.listStrokeColor setStroke];
        [listPath setLineWidth:LINE_THICKNESS + 0.5];
        [listPath stroke];
        
        [NSGraphicsContext restoreGraphicsState];
        
        // Draw inner shadow on toolbar
        [NSGraphicsContext saveGraphicsState];
        [_colorTheme.toolbarShadowColor setStroke];
        NSRect clipRect = NSOffsetRect(NSInsetRect(toolbarPath.bounds, CORNER_RADIUS * 0.3, 0), 0, LINE_THICKNESS);
        [NSBezierPath clipRect:clipRect];
        [toolbarPath addClip];
        [toolbarPath setLineWidth:toolbarPath.lineWidth + 0.25];
        [toolbarPath stroke];
        [NSGraphicsContext restoreGraphicsState];
        
        [_cachedImage unlockFocus];
    }
    
    [_cachedImage drawInRect:rect fromRect:NSOffsetRect(rect,-self.bounds.origin.x,-self.bounds.origin.y) operation:NSCompositeSourceOver fraction:1.0];
}

@end
