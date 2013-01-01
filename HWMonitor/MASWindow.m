//
//  MASWindow.m
//  A Mac App Store-like NSWindow subclass.
//
//  Created by Zachary Waldowski on 1/8/11.
//  Secured under the "do-whatever-the-hell-you-want-with-it" license.  Erm, copyright 2011.
//

#import "MASWindow.h"

@implementation MASWindow

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag {
    if ((self = [super initWithContentRect:contentRect styleMask:aStyle backing:bufferingType defer:flag])) {    
        [self setHasShadow:YES];
        [self setOpaque:NO];
		[self setShowsToolbarButton:NO];
		[self _addResponders];
    }
    return self;
}

- (void)dealloc {
	[self _removeResponders];
	[super dealloc];
}

- (void)awakeFromNib {
	[super awakeFromNib];
	[self _moveToolbar];
}

-(void)_addResponders {
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_repositionTrafficLights) name:NSWindowDidResizeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_resizeContentView) name:NSWindowDidResizeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_repositionTrafficLights) name:NSWindowDidMoveNotification object:nil];	
}

-(void)_removeResponders {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}


-(void)_repositionTrafficLights {
	CGFloat windowHeight = self.frame.size.height;
    NSArray *trafficLights = [[[self contentView] superview] subviews];
    NSView *closeButton = [trafficLights objectAtIndex:0];
	NSView *zoomButton = [trafficLights objectAtIndex:1];
    NSView *minimizeButton = [trafficLights objectAtIndex:2];
	NSRect cf = closeButton.frame;
    NSRect zf = zoomButton.frame;
    NSRect mf = minimizeButton.frame;
	cf.origin.y = zf.origin.y = mf.origin.y = windowHeight - 12 - (self.toolbarHeight/2);
    [closeButton setFrame:cf];
    [zoomButton setFrame:zf];
    [minimizeButton setFrame:mf];	
}

-(void)_resizeContentView {
	NSView *contentView = [[[[self contentView] superview] subviews] objectAtIndex:3];
	CGFloat offset = (self.toolbarHeight/2) - 3;
	NSLog(@"%@ %f %f %f %f", contentView, contentView.frame.origin.x, contentView.frame.origin.y, contentView.frame.size.width, contentView.frame.size.height);
	NSRect contentFrame = [contentView frame];
	contentFrame.size.height += offset;
	[contentView setFrame:contentFrame];
}

-(void)_moveToolbar {
	NSArray *subViews = [[[self contentView] superview] subviews];
	if ([subViews count] > 4) {
		NSView *toolbar = [subViews objectAtIndex:4];
		NSRect toolbarFrame = toolbar.frame;
		toolbarFrame.origin.y = self.frame.size.height - 6 - toolbarFrame.size.height;
		[toolbar setFrame:toolbarFrame];
	} else {
		[self _removeResponders];
	}
}

- (void)setTitle:(NSString *)aString {
	return;
}

- (id)title {
	return @"";
}

- (BOOL)canBecomeKeyWindow {
    return YES;
}

- (void)setToolbar:(NSToolbar *)aToolbar {
	BOOL firstToolbar = (!self.toolbar);
	if (!firstToolbar) {
		[self _removeResponders];
	}
	[super setToolbar:aToolbar];
	if (!firstToolbar) {
		[self _addResponders];
		[self _moveToolbar];
	}
}

- (CGFloat)toolbarHeight {
	NSArray *subViews = [[[self contentView] superview] subviews];
	NSView *theView = [subViews objectAtIndex:4];
	return [theView frame].size.height;
}

@end
