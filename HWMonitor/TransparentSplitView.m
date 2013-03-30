//
//  TransparentSplitView.m
//  HWMonitor
//
//  Created by kozlek on 30.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "TransparentSplitView.h"

@implementation TransparentSplitView

-(void) drawDividerInRect:(NSRect)aRect {
    [[NSColor colorWithCalibratedWhite:0.2 alpha:1.0] set];
    NSRectFill(aRect);
}

@end
