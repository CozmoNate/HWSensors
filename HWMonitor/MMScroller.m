//
//  MMScroller.m
//  MiniMail
//
//  Created by DINH Viêt Hoà on 21/02/10.
//  Copyright 2011 Sparrow SAS. All rights reserved.
//

#import "MMScroller.h"
#import "MMDrawingUtils.h"

@interface MMScroller ()

- (void) _showKnob;
- (void) _updateKnob;
- (void) _updateKnobAfterDelay;

@end

@implementation MMScroller

@synthesize shouldClearBackground = _shouldClearBackground;

#define MIN_ALPHA       0.3
#define MAX_ALPHA       0.6
#define FRAME_COUNT     10
#define SHOW_DELAY      0.3

- (id) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	_oldValue = -1;
	return self;
}

- (void)awakeFromNib
{
    _animationStep = 0;
}

- (void) dealloc
{
    for (NSTrackingArea *area in [self trackingAreas]) {
		[self removeTrackingArea:area];
    }
	[NSObject cancelPreviousPerformRequestsWithTarget:self];
}

- (void)drawKnob
{
	CGFloat alphaValue;
	
	alphaValue = MIN_ALPHA + (MAX_ALPHA - MIN_ALPHA) * (float) _animationStep / (float) FRAME_COUNT;
    if ([self bounds].size.width < [self bounds].size.height) {
        [[NSColor colorWithCalibratedWhite:0.0 alpha:alphaValue] setFill];
        NSRect rect = [self rectForPart:NSScrollerKnob];
        rect.size.width = 9;
        rect.origin.x += 3.0;
        MMFillRoundedRect(rect, 4, 4);
    }
    else {
        // horiz scrollbar
        [[NSColor colorWithCalibratedWhite:0.0 alpha:alphaValue] setFill];
        NSRect rect = [self rectForPart:NSScrollerKnob];
        rect.size.height = 6;
        rect.origin.y += 6.0;
        MMFillRoundedRect(rect, 4, 4);
    }
}

- (void) drawRect:(NSRect)rect
{
    if (_shouldClearBackground) {
        NSEraseRect([self bounds]);
    }
	[self drawKnob];
}

- (void) setFloatValue:(float)value
{
	[super setFloatValue:value];
	if (_oldValue != value) {
		//[self _showKnob];
		_oldValue = value;
	}
}

- (void) showScroller
{
    [self _showKnob];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	[super mouseMoved:theEvent];
	[self _showKnob];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
	[super mouseEntered:theEvent];
	_animationStep = FRAME_COUNT;
	_disableFade = YES;
	[self _updateKnob];
}

- (void)mouseExited:(NSEvent *)theEvent
{
	[super mouseExited:theEvent];
    [self _showKnob];
}

- (void) updateTrackingAreas
{
	NSTrackingArea * trackingArea;
	
    for (NSTrackingArea *area in [self trackingAreas]) {
		[self removeTrackingArea:area];
    }
	
	trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingMouseEnteredAndExited | NSTrackingActiveInActiveApp | NSTrackingMouseMoved owner:self userInfo:nil];
	[self addTrackingArea:trackingArea];
}

- (void) _showKnob
{
	_animationStep = FRAME_COUNT;
    _disableFade = YES;
	[self _updateKnob];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_showKnobAfterDelay) object:nil];
    [self performSelector:@selector(_showKnobAfterDelay) withObject:nil afterDelay:0.5];
}

- (void) _showKnobAfterDelay
{
    _disableFade = NO;
	_animationStep = FRAME_COUNT;
	if (!_scheduled) {
		[self _updateKnob];
	}
}

- (void) _updateKnob
{
	[self setNeedsDisplay:YES];
	
	if (_animationStep > 0) {
		if (!_disableFade) {
			if (!_scheduled) {
				_scheduled = YES;
				[self performSelector:@selector(_updateKnobAfterDelay) withObject:nil afterDelay:SHOW_DELAY / FRAME_COUNT];
				_animationStep --;
			}
		}
	}
}

- (void) _updateKnobAfterDelay
{
	_scheduled = NO;
	[self _updateKnob];
}

@end
