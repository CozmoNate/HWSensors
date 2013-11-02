//
//  InnerShadow.m
//  Pandora
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
    NSColor* trasparente = [NSColor colorWithCalibratedRed: 1 green: 1 blue: 1 alpha: 0];
    
    //// Gradient Declarations
    NSGradient* gradiente = [[NSGradient alloc] initWithStartingColor: [NSColor whiteColor] endingColor: trasparente];
    
    //// Rectangle 2 Drawing
    NSBezierPath* alto = [NSBezierPath bezierPathWithRect: NSMakeRect(0, h - hg, w, hg)];
    [gradiente drawInBezierPath: alto angle: -90];
    
    //// Rectangle Drawing
    NSBezierPath* basso = [NSBezierPath bezierPathWithRect: NSMakeRect(0, 0, w, hg)];
    [gradiente drawInBezierPath: basso angle: 90];
}

@end
