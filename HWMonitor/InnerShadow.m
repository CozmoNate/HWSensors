//
//  InnerShadow.m
//
//
//  Created by Matteo Gaggiano on 20/09/13.
//
//

#import "InnerShadow.h"

@implementation InnerShadow

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
	[super drawRect:dirtyRect];
    CGFloat h = self.frame.size.height;
    CGFloat w = self.frame.size.width;
    CGFloat hg = 16.0f;
    
    //// Color Declarations
    NSColor* transparent = [NSColor colorWithCalibratedRed: 1 green: 1 blue: 1 alpha: 0];
    
    //// Gradient Declarations
    NSGradient* gradient = [[NSGradient alloc] initWithStartingColor: [NSColor whiteColor] endingColor: transparent];
    
    //// Rectangle 2 Drawing
    NSBezierPath* top = [NSBezierPath bezierPathWithRect: NSMakeRect(0, h - hg, w, hg)];
    [gradient drawInBezierPath: top angle: -90];
    
    //// Rectangle Drawing
    NSBezierPath* bottom = [NSBezierPath bezierPathWithRect: NSMakeRect(0, 0, w, hg)];
    [gradient drawInBezierPath: bottom angle: 90];
}

@end
