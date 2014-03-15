//
//  GraphsGroupCell.m
//  HWMonitor
//
//  Created by kozlek on 27.02.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "GraphsGroupCell.h"

@implementation GraphsGroupCell

- (void)drawRect:(NSRect)dirtyRect
{
    if (!_gradient) {
        _gradient = [[NSGradient alloc]
                    initWithStartingColor:[NSColor colorWithDeviceWhite:0.4 alpha:0.5]
                    endingColor:[NSColor colorWithDeviceWhite:0.2 alpha:0.5]];
    }
    
    [_gradient drawInRect:[self bounds] angle:270];
}


@end
