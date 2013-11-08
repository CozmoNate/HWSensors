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

@private
    NSTimer *_scrollTimer;
    BOOL _isTimerValid;
}

@property (nonatomic, assign) NSTimeInterval startTime;
@property (nonatomic, assign) CGFloat currentPosition;
@property (nonatomic, assign) BOOL restartAtTop;
@property (nonatomic, assign) BOOL isShown;

- (void)stopScroll;
- (void)startScroll;
- (void)startScrollWithStartTime:(NSTimeInterval)startTime position:(CGFloat)currentPosition restartAtTop:(BOOL)restartAtTop;

@end
