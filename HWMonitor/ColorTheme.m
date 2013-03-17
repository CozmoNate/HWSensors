//
//  ColorTheme.m
//  HWMonitor
//
//  Created by kozlek on 01.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "ColorTheme.h"

@implementation ColorTheme

#define FILL_OPACITY        0.97
#define TOOLBAR_OPACITY     0.99
#define STROKE_OPACITY      0.75

+ (NSArray*)createColorThemes
{
    NSMutableArray *themes = [[NSMutableArray alloc] init];
    
    ColorTheme *theme = [[ColorTheme alloc] init];
    theme.name = @"Default";
    
    theme.toolbarEndColor = [NSColor colorWithCalibratedRed:0.0 green:0.2 blue:1.0 alpha:TOOLBAR_OPACITY];
    theme.toolbarStartColor = [theme.toolbarEndColor highlightWithLevel:0.8];
    theme.toolbarTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    theme.toolbarStrokeColor = [NSColor colorWithCalibratedWhite:0.15 alpha:STROKE_OPACITY];
    theme.toolbarShadowColor = [NSColor colorWithCalibratedWhite:1.0 alpha:0.3];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:FILL_OPACITY];
    theme.listStrokeColor = [NSColor colorWithCalibratedWhite:0.15 alpha:STROKE_OPACITY];
    theme.listShadowColor = [NSColor clearColor];
    
    theme.groupStartColor = [NSColor colorWithCalibratedWhite:0.96 alpha:FILL_OPACITY];
    theme.groupEndColor = [NSColor colorWithCalibratedWhite:0.90 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.56 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    [theme setHasDarkToolbar:NO];
    [theme setUseDarkIcons:YES];
    [themes addObject:theme];
    
    theme = [[ColorTheme alloc] init];
    theme.name = @"Gray";
    
    theme.toolbarStartColor = [NSColor colorWithCalibratedWhite:0.5 alpha:TOOLBAR_OPACITY];
    theme.toolbarEndColor = [NSColor colorWithCalibratedWhite:0.15 alpha:TOOLBAR_OPACITY];
    theme.toolbarTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    theme.toolbarStrokeColor = [NSColor colorWithCalibratedWhite:0.0 alpha:STROKE_OPACITY];
    theme.toolbarShadowColor = [NSColor colorWithCalibratedWhite:1.0 alpha:0.15];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:FILL_OPACITY];
    theme.listStrokeColor = [NSColor colorWithCalibratedWhite:0.15 alpha:STROKE_OPACITY];
    theme.listShadowColor = [NSColor clearColor];
    
    theme.groupStartColor = [NSColor colorWithCalibratedWhite:0.96 alpha:FILL_OPACITY];
    theme.groupEndColor = [NSColor colorWithCalibratedWhite:0.90 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.56 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    [theme setHasDarkToolbar:YES];
    [theme setUseDarkIcons:YES];
    [themes addObject:theme];
    
    theme = [[ColorTheme alloc] init];
    theme.name = @"Dark";
    
    theme.toolbarEndColor = [NSColor colorWithCalibratedRed:0.0 green:0.2 blue:0.85 alpha:TOOLBAR_OPACITY];
    theme.toolbarStartColor = [theme.toolbarEndColor highlightWithLevel:0.6];
    theme.toolbarTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    theme.toolbarStrokeColor = [NSColor colorWithCalibratedWhite:0.15 alpha:STROKE_OPACITY];
    theme.toolbarShadowColor = [NSColor colorWithCalibratedWhite:1.0 alpha:0.4];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:0.15 alpha:FILL_OPACITY];
    theme.listStrokeColor = [NSColor colorWithCalibratedWhite:0.0 alpha:STROKE_OPACITY];
    theme.listShadowColor = [NSColor colorWithCalibratedWhite:0.1 alpha:0.5];
    
    theme.groupStartColor = [NSColor colorWithCalibratedWhite:0.2 alpha:FILL_OPACITY];
    theme.groupEndColor = [NSColor colorWithCalibratedWhite:0.14 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.45 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.7 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.5 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.95 alpha:1.0];
    
    [theme setHasDarkToolbar:NO];
    [theme setUseDarkIcons:NO];
    [themes addObject:theme];
    
    return themes;
}

@end
