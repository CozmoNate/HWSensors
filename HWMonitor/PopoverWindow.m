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

@property (nonatomic, strong) NSView * customContentView;

@end

@implementation PopoverWindow

@synthesize toolbarView = _toolbarView;

-(HWMEngine *)monitorEngine
{
    return [HWMEngine sharedEngine];
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

-(void)colorThemeChanged
{
//    [self.sensorsTableView setBackgroundColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];
//    [self setBackgroundColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];
    if ([NSAppearance class]) {
        [self setAppearance:[NSAppearance appearanceNamed:self.monitorEngine.configuration.colorTheme.useBrightIcons.boolValue ? NSAppearanceNameVibrantDark : NSAppearanceNameVibrantLight]];
    }
    [[[self contentView] superview] setNeedsDisplay:YES];
}

- (void)windowDidResizeNotification:(NSNotification *)aNotification
{
    [self layoutContent];
}

-(instancetype)initWithContentRect:(NSRect)contentRect styleMask:(NSWindowStyleMask)style backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
    self = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:bufferingType defer:flag];

    if (self) {

        //[self setBackgroundColor:self.monitorEngine.configuration.colorTheme.listBackgroundColor];
        
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{

            if ([NSAppearance class]) {
                [self setAppearance:[NSAppearance appearanceNamed:self.monitorEngine.configuration.colorTheme.useBrightIcons ? NSAppearanceNameVibrantDark : NSAppearanceNameVibrantLight]];
            }

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
        [self colorThemeChanged];
    }
    else if ([keyPath isEqual:@keypath(self, monitorEngine.configuration.showSensorLegendsInPopup)] ||
             [keyPath isEqual:@keypath(self, monitorEngine.sensorsAndGroups)]) {
        [self layoutContent];
    }
}

- (NSTimeInterval)animationResizeTime:(NSRect)newFrame
{
    return 0.25;
}

-(void)orderFront:(id)sender
{
    [super orderFront:sender];
    [self setHeavyBackgroundBlur];
    [self colorThemeChanged];
}

- (void)mouseDown:(NSEvent *)event
{
    PopoverWindow * window = self;

    NSPoint originalMouseLocation = [NSEvent mouseLocation];
    NSRect originalFrame = [window frame];

    while (YES)
    {
        //
        // Lock focus and take all the dragged and mouse up events until we
        // receive a mouse up.
        //
        NSEvent *newEvent = [window nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)];

        // Double click on window
        if ([newEvent type] == NSLeftMouseUp)
        {
            if (newEvent.clickCount == 2) {

                id<PopoverWindowDelegate> delegate = self.popoverWindowDelegate;

                if (delegate && [delegate respondsToSelector:@selector(popoverWindowDidDoubleClick:)]) {
                    [delegate popoverWindowDidDoubleClick:self];
                }
            }
            break;
        }

        //
        // Work out how much the mouse has moved
        //
        NSPoint newMouseLocation = [NSEvent mouseLocation];
        NSPoint delta = NSMakePoint(
                                    newMouseLocation.x - originalMouseLocation.x,
                                    newMouseLocation.y - originalMouseLocation.y);

        NSRect newFrame = originalFrame;

        //
        // Alter the frame for a drag
        //
        newFrame.origin.x += delta.x;
        newFrame.origin.y += delta.y;


        [window setFrame:newFrame display:YES animate:NO];
    }
}

-(void)redraw
{
    [[self.contentView superview] setNeedsDisplay:YES];
}

@end
