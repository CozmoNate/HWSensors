//
//  Localizer.m
//  HWMonitor
//
//  Created by kozlek on 20.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

//  The MIT License (MIT)
//
//  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE

#import "Localizer.h"

@implementation Localizer

+ (Localizer *)localizerWithBundle:(NSBundle *)bundle
{
    Localizer *me = [[Localizer alloc] initWithBundle:bundle];
    
    return me;
}

+(void)localizeView:(id)view
{
    Localizer *localizer = [Localizer localizerWithBundle:[NSBundle mainBundle]];
    [localizer localizeView:view];
}

+(void)localizeView:(id)view withBunde:(NSBundle *)bundle
{
    Localizer *localizer = [Localizer localizerWithBundle:bundle];
    [localizer localizeView:view];
}

-(id)init
{
    self = [super init];
    
    if (self) {
        _bundle = [NSBundle mainBundle];
    }
    
    return self;
}

-(Localizer *)initWithBundle:(NSBundle *)bundle
{
    self = [super init];
    
    if (self) {
        _bundle = bundle;
    }
    
    return self;
}

- (void)localizeView:(id)view
{
    if (!view) {
        return;
    }
    
    if ([view isKindOfClass:[NSWindow class]]) {
        NSWindow *window = (NSWindow *)view;
        [window setTitle:GetLocalizedString(window.title)];
        [self localizeView:window.contentView];
        [self localizeView:window.toolbar];
    }
    else if ([view isKindOfClass:[NSTextField class]]) {
        NSTextField *textField = (NSTextField*)view;
        
        NSString *title = [textField stringValue];
        
        [textField setStringValue:GetLocalizedString(title)];
    }
    else if ([view isKindOfClass:[NSButton class]]) {
        NSButton *button = (NSButton*)view;
        
        NSString *title = [button title];
        
        [button setTitle:GetLocalizedString(title)];
        [button setAlternateTitle:GetLocalizedString([button alternateTitle])];
        
        [self localizeView:button.menu];
    }
    else if ([view isKindOfClass:[NSMatrix class]]) {
        NSMatrix *matrix = (NSMatrix*)view;
        
        NSUInteger row, column;
        
        for (row = 0 ; row < [matrix numberOfRows]; row++) {
            for (column = 0; column < [matrix numberOfColumns] ; column++) {
                NSButtonCell* cell = [matrix cellAtRow:row column:column];
                
                NSString *title = [cell title];
                
                [cell setTitle:GetLocalizedString(title)];
            }
        }
    }
    else if ([view isKindOfClass:[NSMenu class]]) {
        NSMenu *menu = (NSMenu*)view;
        
        [menu setTitle:GetLocalizedString([menu title])];
        
        for (id subItem in [menu itemArray]) {
            if ([subItem isKindOfClass:[NSMenuItem class]]) {
                NSMenuItem* menuItem = subItem;
                
                if ([menuItem attributedTitle] && [[menuItem attributedTitle] string].length) {
                    NSMutableAttributedString *title = [[NSMutableAttributedString alloc] initWithAttributedString:[menuItem attributedTitle]];
                    
                    [title replaceCharactersInRange:NSMakeRange(0, title.length) withString:GetLocalizedString(title.string)];
                    
                    [menuItem setAttributedTitle:title];
                }
                else {
                    [menuItem setTitle:GetLocalizedString([menuItem title])];
                }
                
                
                if ([menuItem hasSubmenu])
                    [self localizeView:[menuItem submenu]];
            }
        }
    }
    else if ([view isKindOfClass:[NSTabView class]]) {
        for (NSTabViewItem *item in [(NSTabView*)view tabViewItems]) {
            [item setLabel:GetLocalizedString([item label])];
            [self localizeView:[item view]];
        }
    }
    else if ([view isKindOfClass:[NSToolbar class]]) {
        for (NSToolbarItem *item in [(NSToolbar*)view items]) {
            [item setLabel:GetLocalizedString([item label])];
            //[self localizeView:[item view]];
        }
    }
    else if ([view isKindOfClass:[NSBox class]]) {
        NSBox *box = (NSBox*)view;
        
        [box setTitle:GetLocalizedString(box.title)];
        
        NSArray *subviews = [[view subviews] copy];
        for (NSView *subView in subviews) {
            [self localizeView:subView];
        }
    }
    else if ([view isKindOfClass:[NSTableView class]]) {

        NSTableView *tableView = (NSTableView*)view;

        for (NSTableColumn *column in tableView.tableColumns) {
            if (column.headerCell) {
                NSTableHeaderCell *cell = column.headerCell;

                NSString * title = GetLocalizedString(cell.title);

                [cell setTitle:title];
            }
        }
    }
    else if ([view isKindOfClass:[NSSegmentedControl class]]) {

        NSSegmentedControl *control = (NSSegmentedControl*)view;

        for (int i = 0; i < control.segmentCount; i++) {

            NSString * title = GetLocalizedString([control labelForSegment:i]);

            [control setLabel:title forSegment:i];

        }
    }
    // Must be at the end to allow other checks to pass because almost all controls are derived from NSView
    else if ([view isKindOfClass:[NSView class]] && [view subviews]) {
        // Loop through children
        if ([view subviews].count) {
            NSArray *subviews = [[view subviews] copy];

            for (NSView *subView in subviews) {
                [self localizeView:subView];
            }
        }
    }
    else {
        if ([view respondsToSelector:@selector(setTitle:)]) {
            NSString *title = [(id)view title];
            [view setTitle:GetLocalizedString(title)];
        }
        else if ([view respondsToSelector:@selector(setStringValue:)]) {
            NSString *title = [(id)view stringValue];
            [view setStringValue:GetLocalizedString(title)];
        }

        if ([view respondsToSelector:@selector(setAlternateTitle:)]) {
            NSString *title = [(id)view alternateTitle];
            [view setAlternateTitle:GetLocalizedString(title)];
        }
    }

    if ([view respondsToSelector:@selector(setToolTip:)]) {
        NSString *tooltip = [view toolTip];
        [view setToolTip:GetLocalizedString(tooltip)];
    }
}


@end
