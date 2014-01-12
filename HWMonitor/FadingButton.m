//
//  FadingButton.m
//  HWMonitor
//
//  Created by kozlek on 28.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "FadingButton.h"

#define NORMAL_OPACITY  0.8
#define HOVER_OPACITY   0.95
#define DOWN_OPACITY    0.6

@implementation FadingButton

- (void)fadeIn:(id)sender
{
    if (self.isEnabled) {
        [NSAnimationContext beginGrouping];
        [[NSAnimationContext currentContext] setDuration:0.01];
        [[self animator] setAlphaValue:_hoverOpacity];
        [NSAnimationContext endGrouping];
    }
}

- (void)fadeOut:(id)sender
{
    if (self.isEnabled) {
        [NSAnimationContext beginGrouping];
        [[NSAnimationContext currentContext] setDuration:0.1];
        [[self animator] setAlphaValue:_normalOpacity];
        [NSAnimationContext endGrouping];
    }
}

- (void)initialize
{
    _normalOpacity = 0.8;
    _hoverOpacity = 0.95;
    _downOpacity = 0.6;

    [self setAlphaValue:_normalOpacity];
}

- (id)init
{
    self = [super init];
    
    if (self) [self initialize];

    return self;
}

-(id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];

    if (self) [self initialize];

    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];

    if (self) [self initialize];

    return self;
}

- (void)dealloc
{
    for (NSTrackingArea *area in [self trackingAreas]) {
		[self removeTrackingArea:area];
    }
}

- (void)awakeFromNib
{
    [self setAlphaValue:_normalOpacity];
}

- (void)updateTrackingAreas
{
    [super updateTrackingAreas];
    
    for (NSTrackingArea *area in [self trackingAreas]) {
		[self removeTrackingArea:area];
    }
    
    NSTrackingAreaOptions options = NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways;
    [self addTrackingArea:[[NSTrackingArea alloc] initWithRect:NSZeroRect options:options owner:self userInfo:nil]];
}

-(void)mouseEntered:(NSEvent *)theEvent
{
    [self fadeIn:self];
    [super mouseEntered:theEvent];
}

-(void)mouseExited:(NSEvent *)theEvent
{
    [self fadeOut:self];
    [super mouseExited:theEvent];
}

-(void)mouseDown:(NSEvent *)theEvent
{
    [self setAlphaValue:_downOpacity];

    [super mouseDown:theEvent];
    
    [self setAlphaValue:_normalOpacity];
    
    if (self.menu) {
        NSRect frame = [self convertRect:self.bounds toView:nil];
        
        NSEvent *event = [NSEvent
                          mouseEventWithType:NSRightMouseDown
                          location: NSMakePoint(frame.origin.x - self.bounds.size.width / 2, frame.origin.y - self.bounds.size.height / 2)
                          modifierFlags: theEvent.modifierFlags
                          timestamp: theEvent.timestamp
                          windowNumber:theEvent.windowNumber
                          context:theEvent.context
                          eventNumber:theEvent.eventNumber
                          clickCount:theEvent.clickCount
                          pressure:theEvent.pressure];
        [NSMenu
         popUpContextMenu:self.menu
         withEvent:event
         forView:self];
    }
}

@end
