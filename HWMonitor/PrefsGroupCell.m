//
//  PrefsGroup.m
//  HWMonitor
//
//  Created by kozlek on 27.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "PrefsGroupCell.h"

@implementation PrefsGroupCell

- (void)drawRect:(NSRect)dirtyRect
{
    if (!_gradient) {
        _gradient = [[NSGradient alloc]
                     initWithStartingColor:[NSColor colorWithCalibratedWhite:0.99 alpha:1.0]
                     endingColor:[NSColor colorWithCalibratedWhite:0.9 alpha:1.0]];
    }
    
    [_gradient drawInRect:[self bounds] angle:270];
}

@end
