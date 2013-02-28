//
//  HWMonitorGroupCell.m
//  HWSensors
//
//  Created by kozlek on 22.02.13.
//
//

#import "GroupCell.h"
#import "PopupView.h"

@implementation GroupCell

- (void)drawRect:(NSRect)dirtyRect
{
    if (!gradient) {
        gradient = [[NSGradient alloc]
                    initWithStartingColor:[NSColor colorWithCalibratedWhite:0.96 alpha:FILL_OPACITY]
                              endingColor:[NSColor colorWithCalibratedWhite:0.90 alpha:FILL_OPACITY]];
//        gradient = [[NSGradient alloc]
//                    initWithColorsAndLocations:
//                    [NSColor colorWithCalibratedWhite:0.97 alpha:0.95], 0.0,
//                    [NSColor colorWithCalibratedWhite:0.82 alpha:0.95], 0.6,
//                    [NSColor colorWithCalibratedWhite:0.90 alpha:0.95], 1.0,
//                    nil];
    }
    
    [gradient drawInRect:[self bounds] angle:270];
}

@end
