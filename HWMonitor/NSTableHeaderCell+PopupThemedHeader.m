//
//  NSTableHeaderCell+PopupThemedHeader.m
//  HWMonitor
//
//  Created by Kozlek on 08.02.14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "NSTableHeaderCell+PopupThemedHeader.h"
#import "HWMColorTheme.h"

static HWMColorTheme* gPopupThemedHeaderTheme = nil;


@implementation NSTableHeaderCell (PopupThemedHeader)


+(void)setGlobalColorTheme:(HWMColorTheme*)theme
{
    gPopupThemedHeaderTheme = theme;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSMutableDictionary *attibutes = [[NSMutableDictionary dictionaryWithDictionary:[[self attributedStringValue]attributesAtIndex:0 effectiveRange:NULL]] mutableCopy];

    [attibutes setObject:gPopupThemedHeaderTheme.itemValueTitleColor forKey:NSForegroundColorAttributeName];

    NSMutableParagraphStyle * paragraphStyle = [[NSMutableParagraphStyle alloc] init];

    [paragraphStyle setLineBreakMode:NSLineBreakByClipping];
    [paragraphStyle setAlignment:NSCenterTextAlignment];

    [attibutes setObject:paragraphStyle forKey:NSParagraphStyleAttributeName];

    NSGradient *gradient = nil;

    if (gPopupThemedHeaderTheme.useDarkIcons) {
        gradient = [[NSGradient alloc] initWithColorsAndLocations:
                    [gPopupThemedHeaderTheme.groupStartColor shadowWithLevel:0.05], 0.1,
                    gPopupThemedHeaderTheme.groupEndColor, 0.9, nil];
    }
    else {
        gradient = [[NSGradient alloc] initWithColorsAndLocations:
                    [gPopupThemedHeaderTheme.groupStartColor highlightWithLevel:0.1], 0.1,
                    gPopupThemedHeaderTheme.groupEndColor, 0.9, nil];
    }

    [gradient drawInRect:cellFrame angle:90];
    
    [[self stringValue] drawInRect:cellFrame withAttributes:attibutes];
}

@end
