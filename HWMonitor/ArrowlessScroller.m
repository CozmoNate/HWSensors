//
//  iOSScroller.m
//  HWMonitor
//
//  Created by kozlek on 08.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "ArrowlessScroller.h"

@implementation ArrowlessScroller

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
    
    [thePath appendBezierPathWithRoundedRect:rect xRadius:4 yRadius:4];
    
    [[NSColor colorWithCalibratedWhite:0.5 alpha:0.5] setFill];    
    [thePath fill];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self drawKnob];
}

@end
