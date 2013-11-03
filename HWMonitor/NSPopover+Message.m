//
//  NSPopover+Instance.m
//  Requires NSString+Size, available at: https://github.com/faceleg/Cocoa-Categories
//
//  Created by Michael Robinson <mike@pagesofinterest.net> on 8/03/12.
//  Copyright (c) 2012 Code of Interest. All rights reserved.
//

#import "NSPopover+Message.h"
#import "NSString+Size.h"

@interface COICOPopoverView : NSView {
    NSColor *backgroundColour;
}

@property (nonatomic, retain) NSColor *backgroundColour;

@end


@implementation COICOPopoverView

@synthesize backgroundColour;

- (void)drawRect:(NSRect)aRect {
    if (self.backgroundColour == nil) {
        [self setBackgroundColour:[NSColor controlBackgroundColor]];
    }
    
    NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:backgroundColour
                                                         endingColor:[backgroundColour blendedColorWithFraction:0.2 ofColor:[NSColor controlBackgroundColor] ]];
    
    NSRect drawingRect = [self frame];
    drawingRect.origin.x = 0;
    drawingRect.origin.y = 0;
    
    NSBezierPath *border = [NSBezierPath bezierPathWithRoundedRect:drawingRect
                                                           xRadius:5.0
                                                           yRadius:5.0];
    [gradient drawInBezierPath:border angle:270.0];
    
    [super drawRect:aRect];
}

@end

@implementation NSPopover (Message)

+ (NSPopover*) showRelativeToRect:(NSRect)rect
                     ofView:(NSView *)view
              preferredEdge:(NSRectEdge)edge
                     string:(NSString *)string
                   maxWidth:(float)width {
    
    return [NSPopover showRelativeToRect:rect
                           ofView:view
                    preferredEdge:edge
                           string:string
                  backgroundColor:[NSColor controlBackgroundColor]
                         maxWidth:width];
}

+ (NSPopover*) showRelativeToRect:(NSRect)rect
                     ofView:(NSView *)view
              preferredEdge:(NSRectEdge)edge
                     string:(NSString *)string
            backgroundColor:(NSColor *)backgroundColor
                   maxWidth:(float)width {
    
    return [NSPopover showRelativeToRect:rect
                           ofView:view
                    preferredEdge:edge
                           string:string
                  backgroundColor:backgroundColor
                  foregroundColor:[NSColor controlTextColor]
                             font:[NSFont systemFontOfSize:[NSFont systemFontSize]]
                         maxWidth:width];
}

+ (NSPopover*) showRelativeToRect:(NSRect)rect
                     ofView:(NSView *)view
              preferredEdge:(NSRectEdge)edge
                     string:(NSString *)string
            backgroundColor:(NSColor *)backgroundColor
            foregroundColor:(NSColor *)foregroundColor
                       font:(NSFont *)font
                   maxWidth:(float)width {
    
    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString:string
                                                                                         attributes:@{
                                                                                NSFontAttributeName: font,
                                                                     NSForegroundColorAttributeName: foregroundColor }];
    
    return [NSPopover showRelativeToRect:rect
                           ofView:view
                    preferredEdge:edge
                 attributedString:attributedString
                  backgroundColor:backgroundColor
                         maxWidth:width];
}

+ (NSPopover*) showRelativeToRect:(NSRect)rect
                     ofView:(NSView *)view
              preferredEdge:(NSRectEdge)edge
           attributedString:(NSAttributedString *)attributedString
            backgroundColor:(NSColor *)backgroundColor
                   maxWidth:(float)width {
    
    float padding = 15;
    
    NSRect containerRect = [attributedString boundingRectWithSize:NSMakeSize(width, 0)
                                                          options:NSStringDrawingUsesLineFragmentOrigin];
    containerRect.size.width = containerRect.size.width *= (20/(containerRect.size.width+2)+1);
    
    NSSize size = containerRect.size;
    NSSize popoverSize = NSMakeSize(containerRect.size.width + (padding * 2), containerRect.size.height + (padding * 2));
    
    containerRect = NSMakeRect(0, 0, popoverSize.width, popoverSize.height);
    
#if __has_feature(objc_arc)
    NSTextField *label = [[NSTextField alloc] initWithFrame:NSMakeRect(padding, padding, size.width, size.height)];
#else
    NSTextField *label = [[[NSTextField alloc] initWithFrame:NSMakeRect(padding, padding, size.width, size.height)] retain];
#endif

    
    [label setBezeled:NO];
    [label setDrawsBackground:NO];
    [label setEditable:NO];
    [label setSelectable:NO];
    [label setAttributedStringValue:attributedString];
    [[label cell] setLineBreakMode:NSLineBreakByWordWrapping];

#if __has_feature(objc_arc)
    COICOPopoverView *container = [[COICOPopoverView alloc] initWithFrame:containerRect];
#else
    COICOPopoverView *container = [[[COICOPopoverView alloc] initWithFrame:containerRect] retain];
#endif
    
    [container setBackgroundColour:backgroundColor];
    [container addSubview:label];
    [label setBounds:NSMakeRect(padding, padding, size.width, size.height - 30)];
    [container awakeFromNib];
    
#if __has_feature(objc_arc)
    NSViewController *controller = [[NSViewController alloc] init];
#else
    NSViewController *controller = [[[NSViewController alloc] init] retain];
#endif
    [controller setView:container];
    
#if __has_feature(objc_arc)
    NSPopover *popover = [[NSPopover alloc] init];
#else
    NSPopover *popover = [[[NSPopover alloc] init] retain];
#endif
    [popover setContentSize:popoverSize];
    [popover setContentViewController:controller];
    [popover setAnimates:YES];
    [popover setBehavior:NSPopoverBehaviorTransient];
    [popover showRelativeToRect:rect
                         ofView:view
                  preferredEdge:edge];

#if __has_feature(objc_arc)
    return popover;
#else
    //[popover release];
    [controller release];
    [container release];
    [label release];
    
    return [popover autorelease];
#endif
}


@end
