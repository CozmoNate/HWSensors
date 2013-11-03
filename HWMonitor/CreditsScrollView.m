//
//  CreditsScrollView.m
//
//
//  Created by Matteo Gaggiano on 21/09/13.
//
//

#import "CreditsScrollView.h"

@implementation CreditsScrollView
@synthesize textView;

- (id)init
{
    if ((self = [super init])) {
        [self setCurrentPosition:0.f];
        [self setRestartAtTop:NO];
        [self setStartTime:[NSDate timeIntervalSinceReferenceDate] + 1.0];        
        [textView scrollPoint:NSMakePoint( 0, 0 )];
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
}

- (void)setCurrentPosition:(CGFloat)currentPosition
{
    __currentPosition = currentPosition;
}

- (CGFloat)currentPosition
{
    return __currentPosition;
}

- (void)setRestartAtTop:(BOOL)restartAtTop
{
    __restartAtTop = restartAtTop;
}

- (BOOL)restartAtTop
{
    return __restartAtTop;
}

- (void)setStartTime:(NSTimeInterval)startTime
{
    __startTime = startTime;
}

- (NSTimeInterval)startTime
{
    return __startTime;
}

- (void)setIsShown:(BOOL)isShown
{
    __isShown = isShown;
}

- (BOOL)isShown
{
    return __isShown;
}

- (void)setIsTimerValid:(BOOL)isTimerValid
{
    __isTimerValid = isTimerValid;
}

- (BOOL)isTimerValid
{
    return __isTimerValid;
}

- (void)startScroll
{
    [self stopScroll];
    NSTrackingAreaOptions opts = NSTrackingActiveInActiveApp | NSTrackingMouseEnteredAndExited | NSTrackingAssumeInside | NSTrackingInVisibleRect;
    
    NSTrackingArea*track=[[NSTrackingArea alloc] initWithRect:self.bounds
                                                      options:opts
                                                        owner:self
                                                     userInfo:nil];
    [self addTrackingArea:track];
    [self setNeedsDisplay:YES];
    
    __scrollTimer = [NSTimer scheduledTimerWithTimeInterval:1/4
                                                     target:self
                                                   selector:@selector(scrollCredits:)
                                                   userInfo:nil
                                                    repeats:YES];
    __isTimerValid = YES;
}

- (void)stopScroll
{
    if ([[self trackingAreas] count] > 0) {
        [self removeTrackingArea:[[self trackingAreas] objectAtIndex:0]];
    }
    
    if (__isTimerValid) {
        [__scrollTimer invalidate];
        __isTimerValid = NO;
    }
}

- (void)startScrollWithStartTime:(NSTimeInterval)startTime position:(CGFloat)currentPosition restartAtTop:(BOOL)restartAtTop
{
    __startTime = startTime;
    __currentPosition = currentPosition;
    __restartAtTop = restartAtTop;
    [self startScroll];
}

- (void)mouseEntered:(NSEvent *)event
{
    if (__isTimerValid) {
        [__scrollTimer invalidate];
    }
    __isTimerValid = NO;;
}

- (void)mouseExited:(NSEvent *)event
{
    if (__isTimerValid) {
        [__scrollTimer invalidate];
    }
    __scrollTimer = [NSTimer scheduledTimerWithTimeInterval:1/4
                                                     target:self
                                                   selector:@selector(scrollCredits:)
                                                   userInfo:nil
                                                    repeats:YES];
    __isTimerValid = YES;
}

- (void)scrollCredits:(NSTimer *)timer
{
    if ([NSDate timeIntervalSinceReferenceDate] >= __startTime)
    {
        if (__restartAtTop)
        {
            // Reset the startTime
            __startTime = [NSDate timeIntervalSinceReferenceDate] + WAITING_TIME;
            __restartAtTop = NO;
            
            // Set the position
            [textView scrollToBeginningOfDocument:self];
            
            return;
        }
        if (__currentPosition >= self.documentVisibleRect.origin.y + 5)
        {
            // Reset the startTime
            __startTime = [NSDate timeIntervalSinceReferenceDate] + WAITING_TIME;
            
            // Reset the position
            __currentPosition = 0;
            __restartAtTop = YES;
        }
        else
        {
            // Scroll to the position
            [textView scrollPoint:NSMakePoint( 0, __currentPosition )];
            // Increment the scroll position

            __currentPosition += 0.005f;
        }
    }
}

@end
