//
//  PopoverWindow.m
//  HWMonitor
//
//  Created by Kozlek on 25/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "PopoverWindow.h"
#import <objc/runtime.h>
#import "HWMEngine.h"
#import "HWMConfiguration.h"
#import "HWMColorTheme.h"
#import "EXTKeyPathCoding.h"
#import "Localizer.h"
#import "NSWindow+BackgroundBlur.h"
//#import "SensorsTableView.h"
#import "PopoverWindowFrameView.h"

@interface NSView (AppKitDetails)
- (void)_addKnownSubview:(NSView *)subview;
@end

@interface PopoverWindow ()

@property (readonly) NSImage *noiseImage;
//@property (readonly) SensorsTableView *sensorsTableView;
@property (nonatomic, strong) NSView * customContentView;

@end

@implementation PopoverWindow

@synthesize toolbarView = _toolbarView;
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

-(void)setToolbarView:(NSView *)newToolbarView
{
    if (_toolbarView) {
        [_toolbarView removeFromSuperview];
    }

    _toolbarView = newToolbarView;

    [Localizer localizeView:_toolbarView];

    if (_toolbarView) {
        NSView * themeFrameView = [self.contentView superview];
        if ([themeFrameView respondsToSelector:@selector(_addKnownSubview:)]) {
            [themeFrameView _addKnownSubview:_toolbarView];
        }
        else {
            [themeFrameView addSubview:_toolbarView];
        }
        [self layoutContent];
    }
}

-(NSView *)toolbarView
{
    return _toolbarView;
}

-(CGFloat)toolbarHeight
{
    return _toolbarView.frame.size.height;
}

- (void)setContentView:(NSView *)aView
{
    if ([self.customContentView isEqualTo:aView])
    {
        return;
    }

    NSRect bounds = [self frame];
    bounds.origin = NSZeroPoint;

    PopoverWindowFrameView *frameView = [super contentView];
    if (!frameView)
    {
        frameView = [[PopoverWindowFrameView alloc] initWithFrame:bounds];
        [super setContentView:frameView];
    }

    if (self.customContentView)
    {
        [self.customContentView removeFromSuperview];
    }
    self.customContentView = aView;
    [self.customContentView setFrame:bounds];
    [frameView addSubview:self.customContentView];
}

- (NSView *)contentView
{
    return self.customContentView;
}

-(NSColor *)backgroundColor
{
    return [NSColor clearColor];
}

-(BOOL)isOpaque
{
    return NO;
}

-(BOOL)canBecomeKeyWindow
{
    return YES;
}

-(BOOL)canBecomeMainWindow
{
    return YES;
}

-(void)layoutContent
{
    NSButton *closeButton = [self standardWindowButton:NSWindowCloseButton];
    NSButton *minimiseButton = [self standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *zoomButton = [self standardWindowButton:NSWindowZoomButton];

    [closeButton setHidden:YES];
    [minimiseButton setHidden:YES];
    [zoomButton setHidden:YES];

    [[self.contentView superview] viewWillStartLiveResize];

    // Position the toolbar view
    NSRect toolbarRect = NSMakeRect(0, self.frame.size.height - self.toolbarHeight, self.frame.size.width, self.toolbarHeight);
    [self.toolbarView setFrame:toolbarRect];

    // Position the content view
    NSRect contentViewFrame = [self.contentView frame];
    CGFloat currentTopMargin = NSHeight(self.frame) - NSHeight(contentViewFrame);
    CGFloat delta = self.toolbarHeight - currentTopMargin;

    contentViewFrame.size.height -= delta;

    [self.contentView setFrame:contentViewFrame];
    [self.customContentView setFrame:[self.contentView bounds]];

    [[self.contentView superview] viewDidEndLiveResize];

    //Redraw the theme frame
    [[self.contentView superview] setNeedsDisplayInRect:toolbarRect];
    [self invalidateShadow];
}

- (void)windowDidResizeNotification:(NSNotification *)aNotification
{
    [self layoutContent];
}

-(instancetype)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
    self = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:bufferingType defer:flag];

    if (self) {

        //[self setBackgroundColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];
        
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{

            [self layoutContent];
            [self redraw];

            [self addObserver:self forKeyPath:@keypath(self, monitorEngine.configuration.colorTheme) options:0 context:nil];

            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(redraw) name:NSWindowDidResizeNotification object:self];
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(redraw) name:NSWindowDidBecomeKeyNotification object:self];
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(redraw) name:NSWindowDidResignKeyNotification object:self];

            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResizeNotification:) name:NSWindowDidResizeNotification object:self];
        }];
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
        //        [self.sensorsTableView setBackgroundColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];
        //[self setBackgroundColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];
        [[[self contentView] superview] setNeedsDisplay:YES];
    }
    else if ([keyPath isEqual:@keypath(self, monitorEngine.configuration.showSensorLegendsInPopup)] ||
             [keyPath isEqual:@keypath(self, monitorEngine.sensorsAndGroups)]) {
        [self layoutContent];
    }
}

-(void)orderFront:(id)sender
{
    [super orderFront:sender];
    [self setHeavyBackgroundBlur];
}

-(void)redraw
{
    [[self.contentView superview] setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect bounds = [self.contentView superview].bounds;

    // Erase the window content
    NSRectFillUsingOperation(bounds, NSCompositeClear);

    CGFloat originX = bounds.origin.x;
    CGFloat originY = bounds.origin.y;
    CGFloat width = bounds.size.width;
    CGFloat height = bounds.size.height;

    CGFloat toolbarOriginY = bounds.origin.y + bounds.size.height - self.toolbarHeight;
    NSRect toolbarBounds = NSMakeRect(originX, toolbarOriginY, width, self.toolbarHeight);

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

    if (self && self.isKeyWindow)
    {
        bottomColor = self.monitorEngine.configuration.colorTheme.toolbarEndColor;
        topColor = self.monitorEngine.configuration.colorTheme.toolbarStartColor;
        topColorTransparent = [NSColor colorWithCalibratedRed:topColor.redComponent green:topColor.greenComponent blue:topColor.blueComponent alpha:0.0];
    }
    else
    {
        bottomColor = [self.monitorEngine.configuration.colorTheme.toolbarEndColor highlightWithLevel:0.2];
        topColor = [self.monitorEngine.configuration.colorTheme.toolbarStartColor highlightWithLevel:0.2];
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

    [self.monitorEngine.configuration.colorTheme.listBackgroundColor set];
    NSRectFill(bounds);

    //[NSGraphicsContext restoreGraphicsState];

    //[NSGraphicsContext saveGraphicsState];

    //[toolbarPath addClip];

    [bottomColor set];

    NSRectFill(toolbarBounds);

    NSGradient *headingGradient = [[NSGradient alloc] initWithStartingColor:topColorTransparent
                                                                endingColor:topColor];
    [headingGradient drawInRect:toolbarBounds angle:90.0];

    // Draw some subtle noise to the titlebar if the window is the key window
    if (self.isKeyWindow)
    {
        [[NSColor colorWithPatternImage:self.noiseImage] set];
        NSRectFillUsingOperation(toolbarBounds, NSCompositeSourceOver);
    }

    [NSGraphicsContext restoreGraphicsState];

    if (self.title) {
        // Draw title
        NSMutableDictionary *titleAttributes = [[NSMutableDictionary alloc] init];
        [titleAttributes setValue:[NSColor colorWithCalibratedWhite:1.0 alpha:0.85] forKey:NSForegroundColorAttributeName];
        [titleAttributes setValue:[NSFont fontWithName:@"Helvetica Light" size:15] forKey:NSFontAttributeName];
        NSShadow *stringShadow = [[NSShadow alloc] init];
        [stringShadow setShadowColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.5]];
        [stringShadow setShadowOffset:NSMakeSize(0, 0)];
        [stringShadow setShadowBlurRadius:6];
        [titleAttributes setValue:stringShadow forKey:NSShadowAttributeName];
        NSSize titleSize = [self.title sizeWithAttributes:titleAttributes];

        NSPoint centerPoint;

        centerPoint.x = /*isAttached ? 10 :*/ (width / 2) - (titleSize.width / 2);
        centerPoint.y = topLeft.y - (self.toolbarHeight / 2) /*- (window.attachedToMenuBar ? OBMenuBarWindowArrowHeight / 2 : 0)*/ - (titleSize.height / 2);

        [self.title drawAtPoint:centerPoint withAttributes:titleAttributes];
    }
}

@end
