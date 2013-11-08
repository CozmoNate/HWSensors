//
//  CreditsScrollView.m
//
//
//  Created by Matteo Gaggiano on 21/09/13.
//
//

#import "CreditsScrollView.h"

#define WAITING_TIME 3.0f
#define SCROLL_STEP 0.1f

@implementation CreditsScrollView

-(id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];

	if (self) {
        [self setCurrentPosition:0.f];
        [self setRestartAtTop:NO];
        [self setStartTime:[NSDate timeIntervalSinceReferenceDate] + 1.0];
        [[self.contentView.subviews firstObject] scrollPoint:NSMakePoint( 0, 0 )];
	}

    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
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
    
    _scrollTimer = [NSTimer scheduledTimerWithTimeInterval:1/4
                                                     target:self
                                                   selector:@selector(scrollCredits:)
                                                   userInfo:nil
                                                    repeats:YES];
    _isTimerValid = YES;
}

- (void)stopScroll
{
    if ([[self trackingAreas] count] > 0) {
        [self removeTrackingArea:[[self trackingAreas] objectAtIndex:0]];
    }
    
    if (_isTimerValid) {
        [_scrollTimer invalidate];
        _isTimerValid = NO;
    }
}

- (void)startScrollWithStartTime:(NSTimeInterval)startTime position:(CGFloat)currentPosition restartAtTop:(BOOL)restartAtTop
{
    _startTime = startTime;
    _currentPosition = currentPosition;
    _restartAtTop = restartAtTop;
    [self startScroll];
}

- (void)mouseEntered:(NSEvent *)event
{
    if (_isTimerValid) {
        [_scrollTimer invalidate];
    }
    _isTimerValid = NO;
}

- (void)mouseExited:(NSEvent *)event
{
    if (_isTimerValid) {
        [_scrollTimer invalidate];
    }
    _scrollTimer = [NSTimer scheduledTimerWithTimeInterval:SCROLL_STEP
                                                    target:self
                                                  selector:@selector(scrollCredits:)
                                                  userInfo:nil
                                                   repeats:YES];
    _isTimerValid = YES;
}

- (void)scrollToBeginningAnimated
{
    NSScrollView *scrollView = self;
    NSClipView *clipView = [scrollView contentView];
    NSPoint constrainedPoint = [clipView constrainScrollPoint:NSMakePoint(0, 0)];
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:0.7];
    [[clipView animator] setBoundsOrigin:constrainedPoint];
    [NSAnimationContext endGrouping];
    [scrollView reflectScrolledClipView:clipView];
}

- (void)scrollToPositionAnimated:(NSPoint)position duration:(NSTimeInterval)duration
{
    NSScrollView *scrollView = self;
    NSClipView *clipView = [scrollView contentView];
    NSPoint constrainedPoint = [clipView constrainScrollPoint:position];
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:duration];
    [[clipView animator] setBoundsOrigin:constrainedPoint];
    [NSAnimationContext endGrouping];
    [scrollView reflectScrolledClipView:clipView];
}

- (void)scrollCredits:(NSTimer *)timer
{
    if ([NSDate timeIntervalSinceReferenceDate] >= _startTime)
    {
        if (_restartAtTop)
        {
            // Reset the startTime
            _startTime = [NSDate timeIntervalSinceReferenceDate] + WAITING_TIME;
            _restartAtTop = NO;
            
            // Set the position
            [self scrollToPositionAnimated:NSMakePoint(0, 0) duration:0.0];
            
            return;
        }

        if (self.documentVisibleRect.origin.y > 0 && _currentPosition >= self.documentVisibleRect.origin.y + 5)
        {
            // Reset the startTime
            _startTime = [NSDate timeIntervalSinceReferenceDate] + WAITING_TIME;
            
            // Reset the position
            _currentPosition = 0;
            _restartAtTop = YES;

            [self performSelector:@selector(scrollToBeginningAnimated) withObject:nil afterDelay:WAITING_TIME / 2.0];
        }
        else
        {
            [self scrollToPositionAnimated:NSMakePoint(0, _currentPosition) duration:SCROLL_STEP];
            // Increment the scroll position

            _currentPosition += 10 * SCROLL_STEP;
        }
    }
}

@end
