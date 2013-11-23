//
//  CreditsScrollView.m
//
//
//  Created by Matteo Gaggiano on 21/09/13.
//
//

#import "CreditsScrollView.h"

#define WAITING_TIME 3.0f
#define SCROLLING_TIME (1.0 / 50) // 50 fps
#define SCROLLING_STEP 0.5f

@implementation CreditsScrollView

-(id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];

	if (self) {
        _currentPosition = 0;
        [self scrollToBeginningAnimated];
	}

    return self;
}

- (void)startScrollWithStartTime:(NSTimeInterval)startTime position:(CGFloat)currentPosition
{
    [self stopScroll];
    [self scrollToPosition:NSMakePoint(0, currentPosition)];
    
    _currentPosition = currentPosition;

    [self performSelector:@selector(startScroll) withObject:nil afterDelay:startTime];
}

- (void)startScroll
{
    [self stopScroll];

    _scrollTimer = [NSTimer scheduledTimerWithTimeInterval:SCROLLING_TIME
                                                    target:self
                                                  selector:@selector(scrollCredits)
                                                  userInfo:nil
                                                   repeats:YES];
}

- (void)stopScroll
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    if (_scrollTimer) {
        [_scrollTimer invalidate];
        _scrollTimer = 0;
    }
}

//- (void)mouseEntered:(NSEvent *)event
//{
//    if (_isTimerValid) {
//        [_scrollTimer invalidate];
//    }
//    _isTimerValid = NO;
//}
//
//- (void)mouseExited:(NSEvent *)event
//{
//    if (_isTimerValid) {
//        [_scrollTimer invalidate];
//    }
//    _scrollTimer = [NSTimer scheduledTimerWithTimeInterval:SCROLL_STEP
//                                                    target:self
//                                                  selector:@selector(scrollCredits:)
//                                                  userInfo:nil
//                                                   repeats:YES];
//    _isTimerValid = YES;
//}

- (void)scrollToBeginningAnimated
{
    NSScrollView *scrollView = self;
    NSClipView *clipView = [scrollView contentView];
    NSPoint constrainedPoint = [clipView constrainScrollPoint:NSMakePoint(0, 0)];
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:1.0f];
    [[clipView animator] setBoundsOrigin:constrainedPoint];
    [NSAnimationContext endGrouping];
    [scrollView reflectScrolledClipView:clipView];
}

- (void)scrollToPosition:(NSPoint)position
{
    NSScrollView *scrollView = self;
    NSClipView *clipView = [scrollView contentView];
    NSPoint constrainedPoint = [clipView constrainScrollPoint:position];
    [clipView setBoundsOrigin:constrainedPoint];
    [scrollView reflectScrolledClipView:clipView];
}

- (void)scrollCredits
{
    if (self.documentVisibleRect.origin.y > 0 && _currentPosition >= self.documentVisibleRect.origin.y + 5) {
        [self stopScroll];
        _currentPosition = 0;
        [self performSelector:@selector(scrollToBeginningAnimated) withObject:nil afterDelay:WAITING_TIME];
        [self performSelector:@selector(startScroll) withObject:nil afterDelay:WAITING_TIME * 2];
    }
    else {
        [self scrollToPosition:NSMakePoint(0, _currentPosition)];

        //[[self.contentView.subviews objectAtIndex:0] scrollPoint:NSMakePoint( 0, _currentPosition )];
        _currentPosition += SCROLLING_STEP;
    }
}

@end
