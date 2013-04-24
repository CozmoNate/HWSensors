//
//  iOSScroller.m
//  HWMonitor
//
//  Created by kozlek on 08.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "ArrowlessScroller.h"

@implementation ArrowlessScroller

-(id)init
{
    self = [super init];
    
    if (self) {
        [self setAlphaValue:0.5];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    
    if (self) {
        [self setAlphaValue:0.5];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    
    if (self) {
        [self setAlphaValue:0.5];
    }
    return self;
}

- (void) dealloc
{
    for (NSTrackingArea *area in [self trackingAreas]) {
		[self removeTrackingArea:area];
    }
}

- (void)mouseEntered:(NSEvent *)theEvent
{
	[super mouseEntered:theEvent];
    [[self animator] setAlphaValue:0.75];
	[self setNeedsDisplay];
}

- (void)mouseExited:(NSEvent *)theEvent
{
	[super mouseExited:theEvent];
    [[self animator] setAlphaValue:0.5];
    [self setNeedsDisplay];
}

-(void)updateTrackingAreas
{
    NSTrackingArea * trackingArea;
	
    for (NSTrackingArea *area in [self trackingAreas]) {
		[self removeTrackingArea:area];
    }
	
	trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingMouseMoved owner:self userInfo:nil];
    
	[self addTrackingArea:trackingArea];
}

- (void)drawKnob
{
    NSRect rect = [self rectForPart:NSScrollerKnob];
	
    if (self.bounds.size.width < self.bounds.size.height) {
        // vertical scrollbar
        rect = NSOffsetRect(NSInsetRect(rect, self.bounds.size.width * 0.25, 1), 1, 0);
    }
    else {
        // horizontal scrollbar
        rect = NSOffsetRect(NSInsetRect(rect, 1, self.bounds.size.height * 0.25), 0, 1);
    }
    
    NSBezierPath* thePath = [NSBezierPath bezierPath];
    
    [thePath appendBezierPathWithRoundedRect:rect xRadius:3 yRadius:3];
    
    [[NSColor colorWithCalibratedWhite:0.5 alpha:1.0] setFill];
    [thePath fill];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self drawKnob];
}

@end
