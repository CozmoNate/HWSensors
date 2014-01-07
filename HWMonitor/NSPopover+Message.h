//
//  NSPopover+Message.h
//  Requires NSString+Size, available at: https://github.com/faceleg/Cocoa-Categories
//
//  Created by Michael Robinson <mike@pagesofinterest.net> on 8/03/12.
//  Copyright (c) 2012 Code of Interest. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface COICOPopoverView : NSView {
    NSGradient *backgroundGradient;
    NSColor *backgroundColour;
}

@property (nonatomic, retain) NSColor *backgroundColour;

@end


@interface NSPopover (Message)

+ (NSPopover*) showRelativeToRect:(NSRect)rect
                     ofView:(NSView *)view
              preferredEdge:(NSRectEdge)edge
                     string:(NSString *)string
                   maxWidth:(float)width;

+ (NSPopover*) showRelativeToRect:(NSRect)rect
                     ofView:(NSView *)view
              preferredEdge:(NSRectEdge)edge
                     string:(NSString *)string
            backgroundColor:(NSColor *)backgroundColor
                   maxWidth:(float)width;

+ (NSPopover*) showRelativeToRect:(NSRect)rect
                     ofView:(NSView *)view
              preferredEdge:(NSRectEdge)edge
                     string:(NSString *)string
            backgroundColor:(NSColor *)backgroundColor
            foregroundColor:(NSColor *)foregroundColor
                       font:(NSFont *)font
                   maxWidth:(float)width;

+ (NSPopover*) showRelativeToRect:(NSRect)rect
                     ofView:(NSView *)view
              preferredEdge:(NSRectEdge)edge
           attributedString:(NSAttributedString *)attributedString
            backgroundColor:(NSColor *)backgroundColor
                   maxWidth:(float)width;
@end
