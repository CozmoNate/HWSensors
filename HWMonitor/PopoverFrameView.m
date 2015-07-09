//
//  ToolbarView.m
//  HWMonitor
//
//  Created by Kozlek on 23/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopoverFrameView.h"
#import "EXTKeyPathCoding.h"
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"

@interface PopoverFrameView ()

@property (readonly) HWMEngine *monitorEngine;
@property (readonly) NSImage *noiseImage;

@end

@implementation PopoverFrameView

@synthesize noiseImage = _noiseImage;

-(HWMEngine *)monitorEngine
{
    return [HWMEngine sharedEngine];
}

-(NSImage*)noiseImage
{
    if (_noiseImage == nil)
    {
        size_t dimension = 100;
        size_t bytes = dimension * dimension * 4;

        //CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

        // Fix rainbow noise when selected device-independent monitor profile
        // Values got from: http://stackoverflow.com/questions/501199/disabling-color-correction-in-quartz-2d
        const CGFloat whitePoint[] = {0.95047, 1.0, 1.08883};
        const CGFloat blackPoint[] = {0, 0, 0};
        const CGFloat gamma[] = {1, 1, 1};
        const CGFloat matrix[] = {0.449695, 0.244634, 0.0251829, 0.316251, 0.672034, 0.141184, 0.18452, 0.0833318, 0.922602 };
        CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateCalibratedRGB(whitePoint, blackPoint, gamma, matrix);

        unsigned char *data = malloc(bytes);
        unsigned char grey;
        for (NSUInteger i = 0; i < bytes; i += 4)
        {
            grey = rand() % 256;
            data[i] = grey;
            data[i + 1] = grey;
            data[i + 2] = grey;
            data[i + 3] = 6;
        }
        CGContextRef contextRef = CGBitmapContextCreate(data, dimension, dimension, 8, dimension * 4, colorSpaceRef,(CGBitmapInfo)kCGImageAlphaPremultipliedLast);
        CGImageRef imageRef = CGBitmapContextCreateImage(contextRef);
        _noiseImage = [[NSImage alloc] initWithCGImage:imageRef size:NSMakeSize(dimension, dimension)];
        CGImageRelease(imageRef);
        CGContextRelease(contextRef);
        free(data);
        CGColorSpaceRelease(colorSpaceRef);
    }
    return _noiseImage;
}

-(void)initialize
{
    _toolbarHeight = 34;

    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        [self addObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.colorTheme) options:0 context:nil];
    }];

}

- (instancetype)init
{
    self = [super init];
    if (self) {
        [self initialize];
    }
    return self;
}

-(instancetype)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) {
        [self initialize];
    }
    return self;
}

-(void)dealloc
{
    [self removeObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.colorTheme)];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@keypath(self, monitorEngine.configuration.colorTheme)]) {
        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    //[super drawRect:dirtyRect];

    CGFloat originX = self.bounds.origin.x;
    CGFloat originY = self.bounds.origin.y;
    CGFloat toolbarOriginY = self.bounds.origin.y + self.bounds.size.height - _toolbarHeight;
    CGFloat width = self.bounds.size.width;
    CGFloat height = self.bounds.size.height;
    NSRect toolbarBounds = NSMakeRect(originX, toolbarOriginY, width, _toolbarHeight);
//    NSRect bounds = NSMakeRect(originX, originY, width, height);

//    CGFloat cornerRadius = 5;
//
//
    NSPoint topLeft = NSMakePoint(originX, originY + height);
//    NSPoint topMiddle = NSMakePoint(originX + width / 2.0, originY + height);
//    NSPoint topRight = NSMakePoint(originX + width, originY + height);
//    NSPoint bottomLeft = NSMakePoint(originX, originY);
//    NSPoint bottomRight = NSMakePoint(originX + width, originY);
//    NSPoint toolbarBottomLeft = NSMakePoint(originX, toolbarOriginY);
//    NSPoint toolbarBottomRight = NSMakePoint(originX + width, toolbarOriginY);
//
//    NSBezierPath *toolbarPath = [NSBezierPath bezierPath];
//
//    [toolbarPath moveToPoint:topMiddle];
//    [toolbarPath appendBezierPathWithArcFromPoint:topRight
//                                         toPoint:toolbarBottomRight
//                                          radius:cornerRadius];
//    [toolbarPath lineToPoint:bottomRight];
//
//    [toolbarPath moveToPoint:topMiddle];
//    [toolbarPath appendBezierPathWithArcFromPoint:topLeft
//                                         toPoint:toolbarBottomLeft
//                                          radius:cornerRadius];
//    [toolbarPath lineToPoint:bottomLeft];
//    [toolbarPath lineToPoint:bottomRight];
//    [toolbarPath closePath];

    NSColor *bottomColor, *topColor, *topColorTransparent;

    /*if (self.window && self.window.isVisible)
    {*/
        bottomColor = self.monitorEngine.configuration.colorTheme.toolbarEndColor;
        topColor = self.monitorEngine.configuration.colorTheme.toolbarStartColor;
        topColorTransparent = [NSColor colorWithCalibratedRed:topColor.redComponent green:topColor.greenComponent blue:topColor.blueComponent alpha:0.0];
    /*}
    else
    {
        bottomColor = [self.monitorEngine.configuration.colorTheme.toolbarEndColor highlightWithLevel:0.2];
        topColor = [self.monitorEngine.configuration.colorTheme.toolbarStartColor highlightWithLevel:0.2];
        topColorTransparent = [[NSColor colorWithCalibratedRed:topColor.redComponent green:topColor.greenComponent blue:topColor.blueComponent alpha:0.0] highlightWithLevel:0.15];
    }*/

//    NSBezierPath *borderPath = [NSBezierPath bezierPath];
//
//    [borderPath moveToPoint:topMiddle];
//    [borderPath appendBezierPathWithArcFromPoint:topRight
//                                          toPoint:bottomRight
//                                           radius:cornerRadius];
//    [borderPath lineToPoint:bottomRight];
//
//    [borderPath moveToPoint:topMiddle];
//    [borderPath appendBezierPathWithArcFromPoint:topLeft
//                                          toPoint:bottomLeft
//                                           radius:cornerRadius];
//    [borderPath lineToPoint:bottomLeft];
//    [borderPath lineToPoint:bottomRight];
//    [borderPath closePath];
//
//    [NSGraphicsContext saveGraphicsState];
//
//    [borderPath addClip];
//
//    [[self.monitorEngine.configuration.colorTheme.listBackgroundColor colorWithAlphaComponent:0.5] set];
//    NSRectFill(bounds);

//    [NSGraphicsContext restoreGraphicsState];

//    [NSGraphicsContext saveGraphicsState];

//    [toolbarPath addClip];

    [bottomColor set];
//    NSRectFill(toolbarBounds);
    NSRectFillUsingOperation(toolbarBounds, NSCompositeSourceOver);

    NSGradient *headingGradient = [[NSGradient alloc] initWithStartingColor:topColorTransparent
                                                                endingColor:topColor];
    [headingGradient drawInRect:toolbarBounds angle:90.0];

    // Draw some subtle noise to the titlebar if the window is the key window
    /*if (self.window.isKeyWindow)
    {*/
        [[NSColor colorWithPatternImage:self.noiseImage] set];
        NSRectFillUsingOperation(toolbarBounds, NSCompositeSourceOver);
    /*}*/

//    [NSGraphicsContext restoreGraphicsState];

    [NSGraphicsContext saveGraphicsState];

    CGContextSetShouldSmoothFonts([NSGraphicsContext currentContext].CGContext, true);
    
    if (_toolbarTitle) {
        // Draw title
        NSMutableDictionary *titleAttributes = [[NSMutableDictionary alloc] init];
        [titleAttributes setValue:[NSColor colorWithCalibratedWhite:1.0 alpha:0.85] forKey:NSForegroundColorAttributeName];
        [titleAttributes setValue:[NSFont fontWithName:@"Helvetica Light" size:15] forKey:NSFontAttributeName];
//        NSShadow *stringShadow = [[NSShadow alloc] init];
//        [stringShadow setShadowColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.5]];
//        [stringShadow setShadowOffset:NSMakeSize(0, 0)];
//        [stringShadow setShadowBlurRadius:6];
//        [titleAttributes setValue:stringShadow forKey:NSShadowAttributeName];
        NSSize titleSize = [_toolbarTitle sizeWithAttributes:titleAttributes];

        NSPoint centerPoint;

        centerPoint.x = 9;///*isAttached ? 10 :*/ (width / 2) - (titleSize.width / 2);
        centerPoint.y = topLeft.y - (_toolbarHeight / 2) /*- (window.attachedToMenuBar ? OBMenuBarWindowArrowHeight / 2 : 0)*/ - (titleSize.height / 2);

        [_toolbarTitle drawAtPoint:centerPoint withAttributes:titleAttributes];
    }
    
    [NSGraphicsContext restoreGraphicsState];
}

@end
