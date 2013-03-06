//
//  ColorTheme.m
//  HWMonitor
//
//  Created by kozlek on 01.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "ColorTheme.h"

@implementation ColorTheme

#define FILL_OPACITY 0.95
#define STROKE_OPACITY 1.00

+ (NSArray*)createColorThemes
{
    NSMutableArray *themes = [[NSMutableArray alloc] init];
    
    ColorTheme *theme = [[ColorTheme alloc] init];
    theme.name = @"Default";
    
    theme.barBackgroundEndColor = [NSColor colorWithCalibratedRed:0.0 green:0.2 blue:1.0 alpha:FILL_OPACITY];
    theme.barBackgroundStartColor = [theme.barBackgroundEndColor highlightWithLevel:0.8];
    theme.barTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    theme.barPathColor = [NSColor colorWithCalibratedRed:0.3 green:0.3 blue:0.4 alpha:STROKE_OPACITY];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:FILL_OPACITY];
    theme.listPathColor = [NSColor colorWithCalibratedWhite:0.2 alpha:STROKE_OPACITY];
    
    theme.groupBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.96 alpha:FILL_OPACITY];
    theme.groupBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.90 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.56 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    [theme setUseAlternateImages:NO];
    
    [themes addObject:theme];
    
    theme = [[ColorTheme alloc] init];
    theme.name = @"Gray";
    
    theme.barBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.5 alpha:FILL_OPACITY];
    theme.barBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.1 alpha:FILL_OPACITY];
    theme.barTitleColor = [NSColor colorWithCalibratedWhite:0.7 alpha:1.0];
    theme.barPathColor = [NSColor colorWithCalibratedWhite:0.1 alpha:FILL_OPACITY];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:FILL_OPACITY];
    theme.listPathColor = [NSColor colorWithCalibratedWhite:0.2 alpha:FILL_OPACITY];
    
    theme.groupBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.96 alpha:FILL_OPACITY];
    theme.groupBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.90 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.56 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    [theme setUseAlternateImages:NO];
    
    [themes addObject:theme];
    
    theme = [[ColorTheme alloc] init];
    theme.name = @"Dark";
    
    theme.barBackgroundEndColor = [NSColor colorWithCalibratedRed:0.0 green:0.15 blue:0.85 alpha:FILL_OPACITY];
    theme.barBackgroundStartColor = [theme.barBackgroundEndColor highlightWithLevel:0.75];
    theme.barTitleColor = [NSColor colorWithCalibratedWhite:0.85 alpha:1.0];
    theme.barPathColor = [NSColor colorWithCalibratedRed:0.25 green:0.25 blue:0.35 alpha:STROKE_OPACITY];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:0.1 alpha:FILL_OPACITY];
    theme.listPathColor = [NSColor colorWithCalibratedWhite:0.1 alpha:STROKE_OPACITY];
    
    theme.groupBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.2 alpha:FILL_OPACITY];
    theme.groupBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.14 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.45 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.7 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.5 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.95 alpha:1.0];
    
    [theme setUseAlternateImages:YES];
    
    [themes addObject:theme];
    
    return themes;
}

@end
