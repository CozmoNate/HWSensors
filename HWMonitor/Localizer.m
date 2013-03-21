//
//  Localizer.m
//  HWMonitor
//
//  Created by kozlek on 20.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "Localizer.h"

@implementation Localizer

+ (void)localizeView:(id)view
{
    if ([view isKindOfClass:[NSMatrix class]]) {
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
    else if ([view isKindOfClass:[NSButton class]]) {
        NSButton *button = (NSButton*)view;
        
        NSString *title = [button title];
        
        [button setTitle:GetLocalizedString(title)];
        [button setAlternateTitle:GetLocalizedString([button alternateTitle])];
    }
    else if ([view isKindOfClass:[NSTextField class]]) {
        NSTextField *textField = (NSTextField*)view;
        
        NSString *title = [[textField cell] title];
        
        [[textField cell] setTitle:GetLocalizedString(title)];
    }
    else if ([view isKindOfClass:[NSTabView class]]) {
        for (NSTabViewItem *item in [(NSTabView*)view tabViewItems]) {
            [item setLabel:GetLocalizedString([item label])];
            [Localizer localizeView:[item view]];
        }
    }
    else if ([view isKindOfClass:[NSToolbar class]]) {
        for (NSToolbarItem *item in [(NSToolbar*)view items]) {
            [item setLabel:GetLocalizedString([item label])];
            [Localizer localizeView:[item view]];
        }
        
        return;
    }
    else if ([view isKindOfClass:[NSMenu class]]) {
        NSMenu *menu = (NSMenu*)view;
        
        [menu setTitle:GetLocalizedString([menu title])];
        
        for (id subItem in [menu itemArray]) {
            if ([subItem isKindOfClass:[NSMenuItem class]]) {
                NSMenuItem* menuItem = subItem;
                
                [menuItem setTitle:GetLocalizedString([menuItem title])];
                
                if ([menuItem hasSubmenu])
                    [Localizer localizeView:[menuItem submenu]];
            }
        }
        
        return;
    }
    else {
        if ([view respondsToSelector:@selector(setAlternateTitle:)]) {
            NSString *title = [(id)view alternateTitle];
            [(id)view setAlternateTitle:GetLocalizedString(title)];
        }
        if ([view respondsToSelector:@selector(setTitle:)]) {
            NSString *title = [(id)view title];
            [(id)view setTitle:GetLocalizedString(title)];
        }
    }
    
    if ([view isKindOfClass:[NSWindow class]]) {
        [Localizer localizeView:[view contentView]];
    }
    else if ([view isKindOfClass:[NSView class]] ) {
        for(NSView *subView in [view subviews])
            [Localizer localizeView:subView];
    }
}


@end
