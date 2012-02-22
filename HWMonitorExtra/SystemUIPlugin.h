//  SystemUIPlugin.h
// 64 bits version from S.Madrau May 2009

#import <Cocoa/Cocoa.h>

#if !__LP64__ 
typedef int NSInteger;
typedef float CGFloat;
#endif

#pragma mark -

@interface NSMenuExtra : NSStatusItem
{
    NSBundle *_bundle;	// 44 = 0x2c
    NSMenu *_menu;	// 48 = 0x30
    NSView *_view;	// 52 = 0x34
    CGFloat _length;	// 56 = 0x38
    struct {
		int customView:1;
		int menuDown:1;
		int reserved:30;
    } _flags;	// 60 = 0x3c
    id _controller;	// 64 = 0x40
}

- initWithBundle:fp12;	// IMP=0x74f20c70
- initWithBundle:fp12 data:fp16;	// IMP=0x74f20ccc
- _initInStatusBar:fp12 withLength:(CGFloat)fp36 withPriority:(int)fp20;	// IMP=0x74f21234
- (void)_adjustLength;	// IMP=0x74f2127c
- _button;	// IMP=0x74f21274
- _window;	// IMP=0x74f2126c
- (SEL)action;	// IMP=0x74f211e8
- alternateImage;	// IMP=0x74f20ebc
- attributedTitle;	// IMP=0x74f2120c
- bundle;	// IMP=0x74f20d5c
- (void)dealloc;	// IMP=0x74f20d04
- (void)drawMenuBackground:(BOOL)fp12;	// IMP=0x74f21170
- (BOOL)highlightMode;	// IMP=0x74f21228
- image;	// IMP=0x74f20e10
- (BOOL)isEnabled;	// IMP=0x74f21218
- (BOOL)isMenuDown;	// IMP=0x74f21164
- (CGFloat)length;	// IMP=0x74f20d64
- menu;	// IMP=0x74f20f68
- (void)popUpMenu:fp12;	// IMP=0x74f211a8
- (void)sendActionOn:(int)fp12;	// IMP=0x74f21230
- (void)setAction:(SEL)fp12;	// IMP=0x74f211f0
- (void)setAlternateImage:fp12;	// IMP=0x74f20f14
- (void)setAttributedTitle:fp12;	// IMP=0x74f21214
- (void)setEnabled:(BOOL)fp12;	// IMP=0x74f21220
- (void)setHighlightMode:(BOOL)fp12;	// IMP=0x74f21224
- (void)setImage:fp12;	// IMP=0x74f20e68
- (void)setLength:(CGFloat)fp36;	// IMP=0x74f20dac
- (void)setMenu:fp12;	// IMP=0x74f20f70
- (void)setTarget:fp12;	// IMP=0x74f211fc
- (void)setTitle:fp12;	// IMP=0x74f21208
- (void)setToolTip:fp12;	// IMP=0x74f20ffc
- (void)setView:fp12;	// IMP=0x74f210f0
- statusBar;	// IMP=0x74f211e0
- target;	// IMP=0x74f211f4
- title;	// IMP=0x74f21200
- toolTip;	// IMP=0x74f20fc4
- view;	// IMP=0x74f21034
- (void)willUnload;	// IMP=0x74f20d00

@end


@interface NSMenuExtra (NSMenuExtraPrivate)
+ (unsigned int)defaultLength;	// IMP=0x74f21298
- (CGFloat)defaultLength;	// IMP=0x74f212a0
- (void)setController:fp12;	// IMP=0x74f21280
- (void)setMenuDown:(BOOL)fp12;	// IMP=0x74f21288
@end

#pragma mark -

@interface NSMenuExtraView : NSView
{
    NSMenu *_menu;	// 80 = 0x50
    NSMenuExtra *_menuExtra;	// 84 = 0x54
    NSImage *_image;	// 88 = 0x58
    NSImage *_alternateImage;	// 92 = 0x5c
}

- initWithFrame:(NSRect)fp12 menuExtra:fp28;	// IMP=0x74f22b48
- alternateImage;	// IMP=0x74f22cb4
- (void)dealloc;	// IMP=0x74f22bc4
- (void)drawRect:(NSRect)fp12;	// IMP=0x74f22d18
- image;	// IMP=0x74f22c3c
- (void)mouseDown:fp12;	// IMP=0x74f22ed8
- (void)setAlternateImage:fp12;	// IMP=0x74f22cbc
- (void)setImage:fp12;	// IMP=0x74f22c44
- (void)setMenu:fp12;	// IMP=0x74f22c34

@end
