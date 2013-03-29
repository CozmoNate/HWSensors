//
//  iOSScroller.m
//  HWMonitor
//
//  Created by kozlek on 08.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "ArrowlessScroller.h"

@implementation ArrowlessScroller

- (void)drawRect:(NSRect)dirtyRect
{
    [self drawKnob];
}

@end
