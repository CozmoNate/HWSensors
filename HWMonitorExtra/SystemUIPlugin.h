//  SystemUIPlugin.h
// 64 bits version from S.Madrau May 2009

#import <Cocoa/Cocoa.h>

#if !__LP64__ 
typedef int NSInteger;
typedef float CGFloat;
#endif

@interface NSDockExtra : NSObject
{
    NSBundle *_bundle;	// 4 = 0x4
    unsigned int _id;	// 8 = 0x8
    void *_nativeWindow;	// 12 = 0xc
    id _controller;	// 16 = 0x10
    struct {
		int nativeWindow:1;
		int reserved:31;
    } _flags;	// 20 = 0x14
    long _fReserved1;	// 24 = 0x18
    long _fReserved2;	// 28 = 0x1c
    long _fReserved3;	// 32 = 0x20
    long _fReserved4;	// 36 = 0x24
}

- initWithBundle:fp12;	// IMP=0x74f213b8
- initWithBundle:fp12 data:fp16;	// IMP=0x74f21424
- (void)dealloc;	// IMP=0x74f21458
- bundle;	// IMP=0x74f214cc
- (void *)carbonCGrafPtr;	// IMP=0x74f21610
- cocoaWindow;	// IMP=0x74f21558
- (void)handleMenuCommand:(unsigned int)fp12 tag:(unsigned int)fp16;	// IMP=0x74f214e0
- menu;	// IMP=0x74f214d4
- (void)setDockLabel:fp12;	// IMP=0x74f2151c
- (void)toggle;	// IMP=0x74f214dc
- (void)willUnload;	// IMP=0x74f214c8

@end


@interface NSDockExtra (NSDockExtraPrivate)
- initWithBundle:fp12 identifier:(unsigned int)fp16 controller:fp20;	// IMP=0x74f216bc
- initWithBundle:fp12 identifier:(unsigned int)fp16 data:fp20 controller:fp24;	// IMP=0x74f216f8
- (void *)cgWindow;	// IMP=0x74f21738
@end


@interface NSDockExtra (NSDockExtraReallyPrivate)
- (void)_releaseWindow;	// IMP=0x74f21740
@end

#pragma mark -

@interface NSApplicationDockExtra : NSDockExtra
{
}

- (void)applicationDied;	// IMP=0x74f22b40
- (void)applicationLaunched;	// IMP=0x74f22b3c
- (void)setDockLabel:fp12;	// IMP=0x74f22b44

@end

#pragma mark -

@interface NSMutableArray (NSDockExtraMenu)
- itemWithTitle:fp12;	// IMP=0x74f21a5c
- insertItemWithTitle:fp12 action:(SEL)fp16 atIndex:(int)fp20;	// IMP=0x74f2180c
- itemWithAction:(SEL)fp12;	// IMP=0x74f21aec
- (void)addItem:fp12;	// IMP=0x74f217d8
- addItemWithTitle:fp12 action:(SEL)fp16;	// IMP=0x74f21874
- (int)indexOfItemWithAction:(SEL)fp12;	// IMP=0x74f219d8
- (int)indexOfItemWithTitle:fp12;	// IMP=0x74f21940
- (void)insertItem:fp12 atIndex:(int)fp16;	// IMP=0x74f217a4
- (void)removeItemAtIndex:(int)fp12;	// IMP=0x74f218d4
- (void)setSubmenu:fp12 forItem:fp16;	// IMP=0x74f21908
@end

#pragma mark -

@interface NSMutableDictionary(NSDockExtraMenuItem)
+ itemWithTitle:fp12 action:(SEL)fp16;	// IMP=0x74f21bd0
+ separatorItem;	// IMP=0x74f21b68
- (SEL)action;	// IMP=0x74f22098
- (BOOL)dynamicBreak;	// IMP=0x74f22890
- (int)dynamicMask;	// IMP=0x74f22790
- (BOOL)hasSubmenu;	// IMP=0x74f21cdc
- (unsigned short)indent;	// IMP=0x74f22690
- (BOOL)isEnabled;	// IMP=0x74f21fb8
- (BOOL)isHeader;	// IMP=0x74f22498
- (BOOL)isIconEnabled;	// IMP=0x74f22ae4
- (BOOL)isSeparatorItem;	// IMP=0x74f21f08
- (BOOL)isSubmenuSelectable;	// IMP=0x74f21e04
- (int)mark;	// IMP=0x74f2229c
- markChar;	// IMP=0x74f223b0
- (void)removeDynamicMask;	// IMP=0x74f227ec
- resourceIcon;	// IMP=0x74f22a40
- (void)setAction:(SEL)fp12;	// IMP=0x74f22010
- (void)setDynamicBreak:(BOOL)fp12;	// IMP=0x74f22828
- (void)setDynamicMask:(int)fp12;	// IMP=0x74f226ec
- (void)setEnabled:(BOOL)fp12;	// IMP=0x74f21f50
- (void)setHeader:(BOOL)fp12;	// IMP=0x74f22430
- (void)setIconEnabled:(BOOL)fp12;	// IMP=0x74f22a7c
- (void)setIndent:(unsigned short)fp12;	// IMP=0x74f225ec
- (void)setMark:(int)fp12;	// IMP=0x74f221f4
- (void)setMarkChar:fp12;	// IMP=0x74f2232c
- (void)setResourceIcon:fp12;	// IMP=0x74f229e4
- (void)setStyle:(int)fp12;	// IMP=0x74f224ec
- (void)setSubmenu:fp12;	// IMP=0x74f21d24
- (void)setSubmenuSelectable:(BOOL)fp12;	// IMP=0x74f21d9c
- (void)setSystemIcon:(unsigned int)fp12;	// IMP=0x74f228e4
- (void)setTag:(int)fp12;	// IMP=0x74f220f4
- (void)setTitle:fp12;	// IMP=0x74f21e58
- (int)style;	// IMP=0x74f22590
- submenu;	// IMP=0x74f21d60
- (unsigned int)systemIcon;	// IMP=0x74f22988
- (NSInteger)tag;	// IMP=0x74f22198
- title;	// IMP=0x74f21ecc
@end

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
