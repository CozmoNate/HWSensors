//
//  PopupView.m
//  HWMonitor
//
//  Created by kozlek on 23.02.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
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

#import "PopupView.h"
#import "HWMonitorDefinitions.h"

@implementation PopupView

-(void)setColorTheme:(ColorTheme*)colorTheme
{
    _colorTheme = colorTheme;
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [NSGraphicsContext saveGraphicsState];
    
    NSRect contentRect = NSInsetRect([self bounds], LINE_THICKNESS, LINE_THICKNESS);
    
    // Header
    
    NSRect headerRect = contentRect;
    headerRect.size.height = kHWMonitorButtonsHeight + ARROW_HEIGHT - LINE_THICKNESS * 2; // Buttons row height
    headerRect.origin.y = contentRect.size.height - headerRect.size.height;
    
    NSBezierPath *headerPath = [NSBezierPath bezierPath];
    
    [headerPath moveToPoint:NSMakePoint(_arrowPosition, NSMaxY(headerRect))];
    [headerPath lineToPoint:NSMakePoint(_arrowPosition + ARROW_WIDTH / 2.0, NSMaxY(headerRect) - ARROW_HEIGHT)];
    [headerPath lineToPoint:NSMakePoint(NSMaxX(headerRect) - CORNER_RADIUS, NSMaxY(headerRect) - ARROW_HEIGHT)];
    
    NSPoint topRightCorner = NSMakePoint(NSMaxX(headerRect) - CORNER_RADIUS, NSMaxY(headerRect) - ARROW_HEIGHT - CORNER_RADIUS);
    [headerPath appendBezierPathWithArcWithCenter:topRightCorner radius:CORNER_RADIUS startAngle:90 endAngle:0 clockwise:YES];
    
    [headerPath lineToPoint:NSMakePoint(NSMaxX(headerRect), NSMinY(headerRect) + CORNER_RADIUS)];
    
    [headerPath lineToPoint:NSMakePoint(NSMaxX(headerRect), NSMinY(headerRect))];
    [headerPath lineToPoint:NSMakePoint(NSMinX(headerRect), NSMinY(headerRect))];
    
    [headerPath lineToPoint:NSMakePoint(NSMinX(headerRect), NSMaxY(headerRect) - ARROW_HEIGHT - CORNER_RADIUS)];
    
    NSPoint topLeftCorner = NSMakePoint(NSMinX(headerRect) + CORNER_RADIUS, NSMaxY(headerRect) - ARROW_HEIGHT - CORNER_RADIUS);
    [headerPath appendBezierPathWithArcWithCenter:topLeftCorner radius:CORNER_RADIUS startAngle:180 endAngle:90 clockwise:YES];
    
    [headerPath lineToPoint:NSMakePoint(_arrowPosition - ARROW_WIDTH / 2.0, NSMaxY(headerRect) - ARROW_HEIGHT)];
    [headerPath closePath];
    
    // Content
    
    NSBezierPath *contentPath = [NSBezierPath bezierPath];

    [contentPath moveToPoint:NSMakePoint(NSMaxX(headerRect), NSMinY(headerRect))];

    [contentPath lineToPoint:NSMakePoint(NSMaxX(contentRect), NSMinY(contentRect) + CORNER_RADIUS)];
     
    NSPoint bottomRightCorner = NSMakePoint(NSMaxX(contentRect) - CORNER_RADIUS, NSMinY(contentRect) + CORNER_RADIUS);
    [contentPath appendBezierPathWithArcWithCenter:bottomRightCorner radius:CORNER_RADIUS startAngle:0 endAngle:270 clockwise:YES];
    
    [contentPath lineToPoint:NSMakePoint(NSMinX(contentRect) + CORNER_RADIUS, NSMinY(contentRect))];
    
    NSPoint bottomLeftCorner = NSMakePoint(NSMinX(contentRect) + CORNER_RADIUS, NSMinY(contentRect) + CORNER_RADIUS);
    [contentPath appendBezierPathWithArcWithCenter:bottomLeftCorner radius:CORNER_RADIUS startAngle:270 endAngle:180 clockwise:YES];
    
    [contentPath lineToPoint:NSMakePoint(NSMinX(headerRect), NSMinY(headerRect))];
     
    [contentPath closePath];
    
    // Draw panel
    
    [[[NSGradient alloc]
      initWithStartingColor:_colorTheme.barBackgroundStartColor
      endingColor:_colorTheme.barBackgroundEndColor]
     drawInBezierPath:headerPath angle:270];
    
    [_colorTheme.listBackgroundColor setFill];
    [contentPath fill];
    
    NSBezierPath *clip = [NSBezierPath bezierPathWithRect:[self bounds]];
    [clip appendBezierPath:headerPath];
    [clip appendBezierPath:contentPath];
    [clip addClip];
    
    [headerPath setLineWidth:LINE_THICKNESS];
    [_colorTheme.barPathColor setStroke];
    [headerPath stroke];
    
    [contentPath setLineWidth:LINE_THICKNESS];
    [_colorTheme.listPathColor setStroke];
    [contentPath stroke];
    
    [NSGraphicsContext restoreGraphicsState];
}

- (void)setArrowPosition:(NSInteger)value
{
    _arrowPosition = value;
    [self setNeedsDisplay:YES];
}


@end
