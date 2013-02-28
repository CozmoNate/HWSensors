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
    if (!gradient) {
        gradient = [[NSGradient alloc]
                    initWithStartingColor:[NSColor colorWithCalibratedWhite:0.4 alpha:0.5]
                    endingColor:[NSColor colorWithCalibratedWhite:0.2 alpha:0.5]];
    }
    
    [gradient drawInRect:[self bounds] angle:270];
}


@end
