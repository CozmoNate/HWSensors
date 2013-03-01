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

+ (NSArray*)createColorThemes
{
    NSMutableArray *themes = [[NSMutableArray alloc] init];
    
    ColorTheme *theme = [[ColorTheme alloc] init];
    theme.name = @"HWMonitor";
    
    theme.barBackgroundEndColor = [NSColor colorWithCalibratedRed:0.0 green:0.2 blue:1.0 alpha:FILL_OPACITY];
    theme.barBackgroundStartColor = [theme.barBackgroundEndColor highlightWithLevel:0.9];
    theme.barTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    theme.barPathColor = [NSColor colorWithCalibratedWhite:0.0 alpha:0.2];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:FILL_OPACITY];
    theme.listPathColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    
    theme.groupBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.96 alpha:FILL_OPACITY];
    theme.groupBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.90 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.56 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    [theme setUseAlternateImages:NO];
    
    [themes addObject:theme];
    
    theme = [[ColorTheme alloc] init];
    theme.name = @"iTunes";
    
    theme.barBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.5 alpha:FILL_OPACITY];
    theme.barBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.1 alpha:FILL_OPACITY];
    theme.barTitleColor = [NSColor colorWithCalibratedWhite:0.7 alpha:1.0];
    theme.barPathColor = [NSColor colorWithCalibratedWhite:0.3 alpha:1.0];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:FILL_OPACITY];
    theme.listPathColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    
    theme.groupBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.96 alpha:FILL_OPACITY];
    theme.groupBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.90 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.56 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    [theme setUseAlternateImages:NO];
    
    [themes addObject:theme];
    
    theme = [[ColorTheme alloc] init];
    theme.name = @"Darkness";
    
    theme.barBackgroundEndColor = [NSColor colorWithCalibratedRed:0.0 green:0.2 blue:1.0 alpha:FILL_OPACITY];
    theme.barBackgroundStartColor = [theme.barBackgroundEndColor highlightWithLevel:0.9];
    theme.barTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    theme.barPathColor = [NSColor colorWithCalibratedWhite:0.0 alpha:0.2];
    
    theme.listBackgroundColor = [NSColor colorWithCalibratedWhite:0.1 alpha:FILL_OPACITY];
    theme.listPathColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    theme.groupBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.26 alpha:FILL_OPACITY];
    theme.groupBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.20 alpha:FILL_OPACITY];
    theme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.75 alpha:1.0];
    
    theme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.7 alpha:1.0];
    theme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.5 alpha:1.0];
    theme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.95 alpha:1.0];
    
    [theme setUseAlternateImages:YES];
    
    [themes addObject:theme];
    
    return themes;
}

@end
