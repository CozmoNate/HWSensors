//
//  PopoverWindowFrameView.m
//  HWMonitor
//
//  Created by Kozlek on 7/6/15.
//  Copyright (c) 2015 kozlek. All rights reserved.
//

//
//  RoundWindowFrameView.m
//  RoundWindow
//
//  Created by Matt Gallagher on 12/12/08.
//  Copyright 2008 Matt Gallagher. All rights reserved.
//
//  Permission is given to use this source code file without charge in any
//  project, commercial or otherwise, entirely at your risk, with the condition
//  that any redistribution (in part or whole) of source code must retain
//  this copyright and permission notice. Attribution in compiled projects is
//  appreciated but not required.
//

#import "PopoverWindowFrameView.h"
#import "PopoverWindow.h"
#import "HWMEngine.h"
#import "HWMColorTheme.h"
#import "HWMConfiguration.h"

@interface PopoverWindowFrameView ()

@property (readonly) NSImage *noiseImage;

@end

@implementation PopoverWindowFrameView

@synthesize noiseImage = _noiseImage;

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

- (void)drawRect:(NSRect)rect
{
    
    PopoverWindow * window = (PopoverWindow*)self.window;
    
    NSRect windowBounds = [window.contentView superview].bounds;
    
    // Erase the window content
    NSRectFillUsingOperation(windowBounds, NSCompositeClear);
    
    CGFloat originX = windowBounds.origin.x;
    CGFloat originY = windowBounds.origin.y;
    CGFloat width = windowBounds.size.width;
    CGFloat height = windowBounds.size.height;
    
    CGFloat toolbarOriginY = windowBounds.origin.y + windowBounds.size.height - window.toolbarHeight;
    NSRect toolbarBounds = NSMakeRect(originX, toolbarOriginY, width, window.toolbarHeight);
    
    CGFloat cornerRadius = 5;
    
    NSPoint topLeft = NSMakePoint(originX, originY + height);
    NSPoint topMiddle = NSMakePoint(originX + width / 2.0, originY + height);
    NSPoint topRight = NSMakePoint(originX + width, originY + height);
    NSPoint bottomLeft = NSMakePoint(originX, originY);
    NSPoint bottomMiddle = NSMakePoint(originX + width / 2.0, originY);
    NSPoint bottomRight = NSMakePoint(originX + width, originY);
    
    NSPoint toolbarBottomLeft = NSMakePoint(originX, toolbarOriginY);
    NSPoint toolbarBottomRight = NSMakePoint(originX + width, toolbarOriginY);
    
    NSBezierPath *toolbarPath = [NSBezierPath bezierPath];
    
    [toolbarPath moveToPoint:toolbarBottomLeft];
    
    [toolbarPath appendBezierPathWithArcFromPoint:topLeft
                                          toPoint:topMiddle
                                           radius:cornerRadius];
    [toolbarPath appendBezierPathWithArcFromPoint:topRight
                                          toPoint:toolbarBottomRight
                                           radius:cornerRadius];
    //    [toolbarPath lineToPoint:toolbarBottomLeft];
    //    [toolbarPath lineToPoint:bottomRight];
    [toolbarPath closePath];
    
    NSColor *bottomColor, *topColor, *topColorTransparent;
    
    if (window.isKeyWindow)
    {
        bottomColor = [HWMEngine sharedEngine].configuration.colorTheme.toolbarEndColor;
        topColor = [HWMEngine sharedEngine].configuration.colorTheme.toolbarStartColor;
        topColorTransparent = [NSColor colorWithCalibratedRed:topColor.redComponent green:topColor.greenComponent blue:topColor.blueComponent alpha:0.0];
    }
    else
    {
        bottomColor = [[HWMEngine sharedEngine].configuration.colorTheme.toolbarEndColor highlightWithLevel:0.2];
        topColor = [[HWMEngine sharedEngine].configuration.colorTheme.toolbarStartColor highlightWithLevel:0.2];
        topColorTransparent = [[NSColor colorWithCalibratedRed:topColor.redComponent green:topColor.greenComponent blue:topColor.blueComponent alpha:0.0] highlightWithLevel:0.15];
    }
    
    NSBezierPath *borderPath = [NSBezierPath bezierPath];
    
    [borderPath moveToPoint:topMiddle];
    [borderPath appendBezierPathWithArcFromPoint:topRight
                                         toPoint:toolbarBottomRight
                                          radius:cornerRadius];
    
    [borderPath appendBezierPathWithArcFromPoint:bottomRight
                                         toPoint:bottomMiddle
                                          radius:cornerRadius];
    
    [borderPath appendBezierPathWithArcFromPoint:bottomLeft
                                         toPoint:toolbarBottomLeft
                                          radius:cornerRadius];
    
    [borderPath appendBezierPathWithArcFromPoint:topLeft
                                         toPoint:topMiddle
                                          radius:cornerRadius];
    
    [borderPath closePath];
    
    [NSGraphicsContext saveGraphicsState];
    
    [borderPath addClip];
    
    [[HWMEngine sharedEngine].configuration.colorTheme.listBackgroundColor set];
    NSRectFillUsingOperation(windowBounds, NSCompositeDestinationOver);

    [bottomColor set];
    NSRectFillUsingOperation(toolbarBounds, NSCompositeSourceOver);
    
    NSGradient *headingGradient = [[NSGradient alloc] initWithStartingColor:topColorTransparent
                                                                endingColor:topColor];
    [headingGradient drawInRect:toolbarBounds angle:90.0];
    
    // Draw some subtle noise to the titlebar if the window is the key window
    if (window.isKeyWindow)
    {
        [[NSColor colorWithPatternImage:self.noiseImage] set];
        NSRectFillUsingOperation(toolbarBounds, NSCompositeSourceOver);
    }
    
    CGContextSetShouldSmoothFonts([NSGraphicsContext currentContext].CGContext, true);
    
    if (window.title) {
        // Draw title
        NSMutableDictionary *titleAttributes = [[NSMutableDictionary alloc] init];
        [titleAttributes setValue:[NSColor colorWithCalibratedWhite:1.0 alpha:0.85] forKey:NSForegroundColorAttributeName];
        [titleAttributes setValue:[NSFont fontWithName:@"Helvetica Light" size:15] forKey:NSFontAttributeName];
        //        NSShadow *stringShadow = [[NSShadow alloc] init];
        //        [stringShadow setShadowColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.5]];
        //        [stringShadow setShadowOffset:NSMakeSize(0, 0)];
        //        [stringShadow setShadowBlurRadius:6];
        //        [titleAttributes setValue:stringShadow forKey:NSShadowAttributeName];
        NSSize titleSize = [window.title sizeWithAttributes:titleAttributes];
        
        NSPoint centerPoint;
        
        centerPoint.x = /*isAttached ? 10 :*/ (width / 2) - (titleSize.width / 2);
        centerPoint.y = topLeft.y - (window.toolbarHeight / 2) /*- (window.attachedToMenuBar ? OBMenuBarWindowArrowHeight / 2 : 0)*/ - (titleSize.height / 2);
        
        [window.title drawAtPoint:centerPoint withAttributes:titleAttributes];
    }
    
    [NSGraphicsContext restoreGraphicsState];
}

@end
