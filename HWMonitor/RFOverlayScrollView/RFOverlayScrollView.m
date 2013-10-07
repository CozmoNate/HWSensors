//
//  RFOverlayScrollView.m
//  RFOverlayScrollView
//
//  Created by Tim Brückmann on 31.12.12.
//  Copyright (c) 2012 Rheinfabrik. All rights reserved.
//

/*
 * Copyright (c) 2012 Rheinfabrik
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#import <objc/objc-class.h>
#import "RFOverlayScrollView.h"
#import "RFOverlayScroller.h"

@implementation RFOverlayScrollView

static NSComparisonResult scrollerAboveSiblingViewsComparator(NSView *view1, NSView *view2, void *context)
{
    if ([view1 isKindOfClass:[RFOverlayScroller class]]) {
        return NSOrderedDescending;
    } else if ([view2 isKindOfClass:[RFOverlayScroller class]]) {
        return NSOrderedAscending;
    }
    
    return NSOrderedSame;
}

//- (id)initWithFrame:(NSRect)frameRect
//{
//    self = [super initWithFrame:frameRect];
//    if (self) {
//        self.wantsLayer = YES;
//    }
//    return self;
//}

//- (void)awakeFromNib
//{
//    self.wantsLayer = YES;
//}

- (void)tile
{
    [super tile];
    
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

@end
