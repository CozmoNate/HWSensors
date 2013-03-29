//
//  OverlapingScrollView.m
//  HWMonitor
//
//  Created by kozlek on 29.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "OverlapingScrollView.h"

@implementation OverlapingScrollView

- (void)tile
{
    [super tile];
    
    NSRect frame = [self bounds];
    
    if ([self borderType] != NSNoBorder) {
        frame = NSInsetRect(frame, 1, 1);
    }
    
	[[self contentView] setFrame:frame];
}

@end
