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
        
        NSInsetRect(popupBounds, 0.5, 0.5);
        
        // Toolbar and arrow path
        NSRect toolbarRect = popupBounds;
        toolbarRect.size.height = kHWMonitorToolbarHeight + ARROW_HEIGHT - LINE_THICKNESS * 2;
        toolbarRect.origin.y = popupBounds.size.height - toolbarRect.size.height;
        
        NSBezierPath *toolbarPath = [NSBezierPath bezierPath];
        
        [toolbarPath setFlatness:0.0];
        [toolbarPath setLineJoinStyle:NSMiterLineJoinStyle];
        
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
        
        // List path
        NSBezierPath *listPath = [NSBezierPath bezierPath];
        
        [listPath setFlatness:0.0];
        [listPath setLineJoinStyle:NSMiterLineJoinStyle];
        
        [listPath moveToPoint:NSMakePoint(NSMaxX(toolbarRect), NSMinY(toolbarRect))];
        
        [listPath lineToPoint:NSMakePoint(NSMaxX(popupBounds), NSMinY(popupBounds) + CORNER_RADIUS)];
        
        NSPoint bottomRightCorner = NSMakePoint(NSMaxX(popupBounds) - CORNER_RADIUS, NSMinY(popupBounds) + CORNER_RADIUS);
        [listPath appendBezierPathWithArcWithCenter:bottomRightCorner radius:CORNER_RADIUS startAngle:0 endAngle:270 clockwise:YES];
        
        [listPath lineToPoint:NSMakePoint(NSMinX(popupBounds) + CORNER_RADIUS, NSMinY(popupBounds))];
        
        NSPoint bottomLeftCorner = NSMakePoint(NSMinX(popupBounds) + CORNER_RADIUS, NSMinY(popupBounds) + CORNER_RADIUS);
        [listPath appendBezierPathWithArcWithCenter:bottomLeftCorner radius:CORNER_RADIUS startAngle:270 endAngle:180 clockwise:YES];
        
        [listPath lineToPoint:NSMakePoint(NSMinX(toolbarRect), NSMinY(toolbarRect))];
       
        NSBezierPath *clipPath = [NSBezierPath bezierPath];
        [clipPath appendBezierPath:toolbarPath];
        [clipPath appendBezierPath:listPath];
        [clipPath setFlatness:0.0];
        [clipPath setLineJoinStyle:NSMiterLineJoinStyle];
        [clipPath setLineWidth:LINE_THICKNESS];
        [clipPath addClip];

        // Fill list
        [_colorTheme.listBackgroundColor setFill];
        [listPath fill];
        
        // Fill toolbar
        [[[NSGradient alloc] initWithStartingColor:_colorTheme.toolbarStartColor endingColor:_colorTheme.toolbarEndColor] drawInBezierPath:toolbarPath angle:270];

        // Draw inner shadow
        NSBezierPath *shadowPath = [NSBezierPath bezierPath];
        topLeftCorner = NSMakePoint(NSMinX(toolbarRect) + CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT - CORNER_RADIUS - LINE_THICKNESS);
        [shadowPath appendBezierPathWithArcWithCenter:topLeftCorner radius:CORNER_RADIUS startAngle:135 endAngle:90 clockwise:YES];
        [shadowPath lineToPoint:NSMakePoint(_arrowPosition - ARROW_WIDTH / 2.0f, NSMaxY(toolbarRect) - ARROW_HEIGHT - LINE_THICKNESS)];
        [shadowPath lineToPoint:NSMakePoint(_arrowPosition, NSMaxY(toolbarRect) - LINE_THICKNESS)];
        [shadowPath lineToPoint:NSMakePoint(_arrowPosition + ARROW_WIDTH / 2.0f, NSMaxY(toolbarRect) - ARROW_HEIGHT - LINE_THICKNESS)];
        [shadowPath lineToPoint:NSMakePoint(NSMaxX(toolbarRect) - CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT - LINE_THICKNESS)];
        topRightCorner = NSMakePoint(NSMaxX(toolbarRect) - CORNER_RADIUS, NSMaxY(toolbarRect) - ARROW_HEIGHT - CORNER_RADIUS - LINE_THICKNESS);
        [shadowPath appendBezierPathWithArcWithCenter:topRightCorner radius:CORNER_RADIUS startAngle:45 endAngle:0 clockwise:YES];
        [shadowPath setFlatness:0.0];
        [shadowPath setLineWidth:LINE_THICKNESS * 2];
        [[NSColor colorWithCalibratedWhite:1.0 alpha:0.2] setStroke];
        [shadowPath stroke];
        
        // Stroke toolbar and list
        [_colorTheme.toolbarStrokeColor setStroke];
        [clipPath setLineWidth:LINE_THICKNESS];
        [clipPath stroke];
        
        [_cachedImage unlockFocus];
    }
    
    [_cachedImage drawInRect:rect fromRect:NSOffsetRect(rect,-self.bounds.origin.x,-self.bounds.origin.y) operation:NSCompositeSourceOver fraction:1.0];
}

@end
