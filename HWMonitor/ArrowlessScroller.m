//
//  iOSScroller.m
//  HWMonitor
//
//  Created by kozlek on 08.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "ArrowlessScroller.h"

@implementation ArrowlessScroller

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self drawKnob];
}

@end
