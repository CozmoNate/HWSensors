//
//  CreditsScrollView.h
//
//
//  Created by Matteo Gaggiano on 21/09/13.
//
//

#import "JLNFadingScrollView.h"

#define WAITING_TIME 3.0f

@interface CreditsScrollView : JLNFadingScrollView
{

@private
    NSTimer *__scrollTimer;
    NSTimeInterval __startTime;
    CGFloat __currentPosition;
    BOOL __restartAtTop;
    BOOL __isShown;
    BOOL __isTimerValid;
}

- (void)setStartTime:(NSTimeInterval)startTime;
- (NSTimeInterval)startTime;
- (void)setCurrentPosition:(CGFloat)currentPosition;
- (CGFloat)currentPosition;
- (void)setRestartAtTop:(BOOL)restartAtTop;
- (BOOL)restartAtTop;
- (void)setIsShown:(BOOL)isShown;
- (BOOL)isShown;
- (void)setIsTimerValid:(BOOL)isTimerValid;
- (BOOL)isTimerValid;

- (void)stopScroll;
- (void)startScroll;
- (void)startScrollWithStartTime:(NSTimeInterval)startTime position:(CGFloat)currentPosition restartAtTop:(BOOL)restartAtTop;

@end
