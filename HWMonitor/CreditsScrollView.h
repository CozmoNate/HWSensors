//
//  CreditsScrollView.h
//  Pandora
//
//  Created by Matteo Gaggiano on 21/09/13.
//
//

#import <Cocoa/Cocoa.h>

#define WAITING_TIME 1.0f


@interface CreditsScrollView : NSScrollView 
{
#if !__has_feature(objc_arc)
    NSTextView*_contentTextView;
#endif
    
@private
    NSTimer *__scrollTimer;
    NSTimeInterval __startTime;
    CGFloat __currentPosition;
    BOOL __restartAtTop;
    BOOL __isShown;
    BOOL __isTimerValid;
}

#if !__has_feature(objc_arc)
@property (nonatomic, retain) IBOutlet NSTextView* contentTextView;
#else
@property (assign) IBOutlet NSTextView* contentTextView;
#endif
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
