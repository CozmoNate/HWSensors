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
                     initWithStartingColor:[NSColor colorWithCalibratedWhite:0.96 alpha:0.95]
                     endingColor:[NSColor colorWithCalibratedWhite:0.9 alpha:0.95]];
    }
    
    [_gradient drawInRect:[self bounds] angle:270];
}

@end
