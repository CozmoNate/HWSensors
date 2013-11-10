//
//  CreditsScrollView.h
//
//
//  Created by Matteo Gaggiano on 21/09/13.
//
//

#import "JLNFadingScrollView.h"

@interface CreditsScrollView : JLNFadingScrollView
{
    NSTimer *_scrollTimer;
    CGFloat _currentPosition;
}

- (void)stopScroll;
- (void)startScroll;
- (void)startScrollWithStartTime:(NSTimeInterval)startTime position:(CGFloat)currentPosition;

@end
