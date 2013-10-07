//
//  JLNFadingScrollView.m
//  JLNFadingScrollView
//
//  Created by Joshua Nozzi on 12/23/11.
//  Copyright (c) 2011 Joshua Nozzi. All rights reserved.
//
//	 This software is supplied to you by Joshua Nozzi in consideration 
//	 of your agreement to the following terms, and your use, installation, 
//	 modification or redistribution of this software constitutes 
//	 acceptance of these terms. If you do not agree with these terms, 
//	 please do not use, install, modify or redistribute this software.
//	 
//	 In consideration of your agreement to abide by the following terms, 
//	 and subject to these terms, Joshua Nozzi grants you a personal, 
//	 non-exclusive license, to use, reproduce, modify and redistribute 
//	 the software, with or without modifications, in source and/or binary 
//	 forms; provided that if you redistribute the software in its entirety 
//	 and without modifications, you must retain this notice and the 
//	 following text and disclaimers in all such redistributions of the 
//	 software, and that in all cases attribution of Joshua Nozzi as the 
//	 original author of the source code shall be included in all such 
//	 resulting software products or distributions. Neither the name, 
//	 trademarks, service marks or logos of Joshua Nozzi may be used to 
//	 endorse or promote products derived from the software without specific 
//	 prior written permission from Joshua Nozzi. Except as expressly stated 
//	 in this notice, no other rights or licenses, express or implied, are 
//	 granted by Joshua Nozzi herein, including but not limited to any patent 
//	 rights that may be infringed by your derivative works or by other works 
//	 in which the software may be incorporated.
//	 
//	 THIS SOFTWARE IS PROVIDED BY JOSHUA NOZZI ON AN "AS IS" BASIS. JOSHUA 
//	 NOZZI MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT 
//	 LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
//	 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE SOFTWARE OR ITS USE 
//	 AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//	 
//	 IN NO EVENT SHALL JOSHUA NOZZI BE LIABLE FOR ANY SPECIAL, INDIRECT, 
//	 INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
//	 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
//	 PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE 
//	 USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE SOFTWARE, 
//	 HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING 
//	 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF JOSHUA NOZZI HAS 
//	 BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//	

#import <objc/objc-class.h>
#import "JLNFadingScrollView.h"
#import "RFOverlayScroller.h"


#define DEFAULTFADEHEIGHT					6.0


@implementation JLNFadingScrollView


#pragma mark Constructors / Destructors

static NSComparisonResult scrollerAboveSiblingViewsComparator(NSView *view1, NSView *view2, void *context)
{
    if ([view1 isKindOfClass:[RFOverlayScroller class]]) {
        return NSOrderedDescending;
    } else if ([view2 isKindOfClass:[RFOverlayScroller class]]) {
        return NSOrderedAscending;
    }

    return NSOrderedSame;
}

- (id)initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	if (self)
	{
		
		// iVars
		_topFadeView = nil;
		_bottomFadeView = nil;
		_fadeHeight = DEFAULTFADEHEIGHT;
		_fadeColor = [NSColor blackColor];

		// Kill copy on scroll
		[[self contentView] setCopiesOnScroll:NO];
	}
	
	return self;
}

#pragma mark Nib Loading

- (void)awakeFromNib
{
	// Kill copy on scroll
	[[self contentView] setCopiesOnScroll:NO];
	
	// Initial state
	_fadeHeight = DEFAULTFADEHEIGHT;
	_fadeColor = [NSColor blackColor];
}


#pragma mark Accessors

- (CGFloat)fadeHeight
{
	return _fadeHeight;
}

- (void)setFadeHeight:(CGFloat)newHeight
{
	if (_fadeHeight != newHeight)
	{
		_fadeHeight = newHeight;
		[self tileFadeViews];
	}
}

- (NSColor *)fadeColor
{
	return _fadeColor;
}

- (void)setFadeColor:(NSColor *)newColor
{
	
	// Must be a different color and not be nil
	if (_fadeColor != newColor && newColor)
	{
		
		// Swap the colors
		_fadeColor = [newColor copy];
		
		// Update the fade views
		[_topFadeView setFadeColor:_fadeColor];
		[_bottomFadeView setFadeColor:_fadeColor];

	}
	
}


#pragma mark Tiling

- (void)tile
{
	// Super first, then fade views
	[super tile];
	[self tileFadeViews];

    // Fake zero scroller width so the contentView gets drawn to the edge
    method_exchangeImplementations(class_getClassMethod([RFOverlayScroller class], @selector(scrollerWidthForControlSize:scrollerStyle:)),
                                   class_getClassMethod([RFOverlayScroller class], @selector(zeroWidth)));
	[super tile];
    // Restore original scroller width
    method_exchangeImplementations(class_getClassMethod([RFOverlayScroller class], @selector(scrollerWidthForControlSize:scrollerStyle:)),
                                   class_getClassMethod([RFOverlayScroller class], @selector(zeroWidth)));

    // Resize vertical scroller
    CGFloat width = [RFOverlayScroller scrollerWidthForControlSize:self.verticalScroller.controlSize
                                                     scrollerStyle:self.verticalScroller.scrollerStyle];
	[self.verticalScroller setFrame:(NSRect){
        self.bounds.size.width - width,
        0.0f,
        width,
        self.bounds.size.height
    }];

    // Move scroller to front
    [self sortSubviewsUsingFunction:scrollerAboveSiblingViewsComparator
                            context:NULL];
}

- (void)tileFadeViews
{
	
	// Make sure we have our top and bottom fade views
	if (!_topFadeView)
	{
		_topFadeView = [[JLNFadeView alloc] initWithFrame:NSMakeRect(0.0, 0.0, NSWidth([[self contentView] frame]), _fadeHeight)];
		[_topFadeView setTopDown:YES];
		[_topFadeView setFadeColor:[self backgroundColor]];
	}
	if (!_bottomFadeView)
	{
		_bottomFadeView = [[JLNFadeView alloc] initWithFrame:NSMakeRect(0.0, 0.0, NSWidth([[self contentView] frame]), _fadeHeight)];
		[_bottomFadeView setFadeColor:[self backgroundColor]];
	}
	
	// Determine inset for border type
	CGFloat borderInset;
	switch ([self borderType])
	{
		
		case NSLineBorder:
		case NSBezelBorder:
			borderInset = 1.0;
			break;
			
		case NSGrooveBorder:
			borderInset = 2.0;
			break;
			
		case NSNoBorder:
		default:
			borderInset = 0.0;
			break;
			
	}
	
	// Determine and set the fade views' frames (accounting for border)
	NSRect topFrame = NSMakeRect(borderInset, NSMaxY([self bounds]) - (_fadeHeight + borderInset), NSWidth([self bounds]) - (2 * borderInset), _fadeHeight);
	NSRect bottomFrame = NSMakeRect(borderInset, borderInset, NSWidth([self bounds]) - (2 * borderInset), _fadeHeight);

    // Remove fade views from superview and add back ordered above all else
	// (this is the best way I know how to make *damn* sure they stay on top)

    if (!NSEqualRects(_topFadeView.frame, topFrame)) {
        [_topFadeView removeFromSuperviewWithoutNeedingDisplay];
        [self addSubview:_topFadeView positioned:NSWindowAbove relativeTo:[self contentView]];
        [_topFadeView setFrame:topFrame];
    }

    if (!NSEqualRects(_bottomFadeView.frame, bottomFrame)) {
        [_bottomFadeView removeFromSuperviewWithoutNeedingDisplay];
        [_bottomFadeView setFrame:bottomFrame];
        [self addSubview:_bottomFadeView positioned:NSWindowAbove relativeTo:[self contentView]];
    }

	// Flag the fade views for display
	//[_topFadeView setNeedsDisplay:YES];
	//[_bottomFadeView setNeedsDisplay:YES];
	
}


@end



#pragma mark -

@implementation JLNFadeView

#pragma mark Accessors

@synthesize topDown = _topDown;

- (NSColor *)fadeColor
{
	return _fadeColor;
}

- (void)setFadeColor:(NSColor *)newColor
{
	
	// Must be different and must not be nil
	if (_fadeColor != newColor && newColor != nil)
	{
		
		// Swap the color
		_fadeColor = [newColor copy];
		_fadient = nil;
        
		// Flag for display
		[self setNeedsDisplay:YES];
		
	}
	
}


#pragma mark Drawing

- (void)viewWillDraw
{
	
	// Make sure we have a sane fade color and gradient
	if (!_fadeColor)
		_fadeColor = [NSColor blackColor];
	if (!_fadient)
		_fadient = [[NSGradient alloc] initWithColors:[NSArray arrayWithObjects:
													   _fadeColor, 
													   [_fadeColor colorWithAlphaComponent:0.25], 
													   [_fadeColor colorWithAlphaComponent:0.00], 
													   nil]];
	
}

- (void)drawRect:(NSRect)dirtyRect
{
	
	// Angle?
	float angle;
	if ([self isFlipped])
		angle = ([self topDown]) ? -90 : 90;
	else
		angle = ([self topDown]) ? 90 : -90;
	
	// Draw the gradient
	[_fadient drawInRect:[self bounds] angle:angle];
	
}


@end

