//
//  RFTransparentScroller.m
//  RFOverlayScrollView
//
//  Created by Tim Brückmann on 30.12.12.
//  Copyright (c) 2012 Rheinfabrik. All rights reserved.
//

/*
 * Copyright (c) 2012 Rheinfabrik
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#import "RFOverlayScroller.h"

#define HIGHLLIGHTED_OPACITY    0.85
#define NORMAL_OPACITY          0.35

@implementation RFOverlayScroller

-(instancetype)init
{
    self = [super init];

    if (self) {
        [self initialize];
    }

    return self;
}

-(instancetype)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if (self) {
        [self initialize];
    }

    return self;
}

-(instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];

    if (self) {
        [self initialize];
    }
    return self;
}

-(void)initialize
{
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self setAlphaValue:NORMAL_OPACITY];
    }];
}

- (void) dealloc
{
    for (NSTrackingArea *area in [self trackingAreas]) {
		[self removeTrackingArea:area];
    }
}

//-(void)resetCursorRects
//{
//    [self discardCursorRects];
//    [self addCursorRect:self.frame cursor:[NSCursor pointingHandCursor]];
//}

//- (void)drawRect:(NSRect)dirtyRect
//{
//    // Only draw the knob. drawRect: should only be invoked when overlay scrollers are not used
//    [self drawKnob];
//}

- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag
{
    // Don't draw the background. Should only be invoked when using overlay scrollers
//    [[NSColor colorWithCalibratedWhite:0.5 alpha:0.5] setFill];
//    NSRectFill(slotRect);
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

- (void)mouseExited:(NSEvent *)theEvent
{
    [super mouseExited:theEvent];

    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [[NSAnimationContext currentContext] setDuration:0.15];
        [self.animator setAlphaValue:NORMAL_OPACITY];
    } completionHandler:^{

    }];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    [super mouseEntered:theEvent];

    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        [[NSAnimationContext currentContext] setDuration:0.1];
        [self.animator setAlphaValue:HIGHLLIGHTED_OPACITY];
    } completionHandler:^{

    }];
}

- (void)updateTrackingAreas
{
    [super updateTrackingAreas];
	
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                                options:(
                                                                         NSTrackingMouseEnteredAndExited
                                                                         | NSTrackingActiveInActiveApp
                                                                         | NSTrackingMouseMoved
                                                                         )
                                                                  owner:self
                                                               userInfo:nil];
    [self addTrackingArea:trackingArea];
}

+ (BOOL)isCompatibleWithOverlayScrollers
{
    return self != [RFOverlayScroller class];
}

+ (CGFloat)zeroWidth
{
    return 0.0f;
}

@end
