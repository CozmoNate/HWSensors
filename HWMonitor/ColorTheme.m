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
    
    ColorTheme *lightTheme = [[ColorTheme alloc] init];
    
    lightTheme.name = @"iTunes";
    
    lightTheme.barBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.5 alpha:FILL_OPACITY];
    lightTheme.barBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.1 alpha:FILL_OPACITY];
    lightTheme.barTitleColor = [NSColor colorWithCalibratedWhite:0.7 alpha:1.0];
    lightTheme.barPathColor = [NSColor colorWithCalibratedWhite:0.3 alpha:1.0];
    
    lightTheme.listBackgroundColor = [NSColor colorWithCalibratedWhite:1.0 alpha:FILL_OPACITY];
    lightTheme.listPathColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    
    lightTheme.groupBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.96 alpha:FILL_OPACITY];
    lightTheme.groupBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.90 alpha:FILL_OPACITY];
    lightTheme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    
    lightTheme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.15 alpha:1.0];
    lightTheme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.56 alpha:1.0];
    lightTheme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    [lightTheme setUseAlternateImages:NO];
    
    ColorTheme *darkTheme = [[ColorTheme alloc] init];
    
    darkTheme.name = @"Darkness";
    
    darkTheme.barBackgroundEndColor = [NSColor colorWithCalibratedRed:0.0 green:0.2 blue:1.0 alpha:FILL_OPACITY];
    darkTheme.barBackgroundStartColor = [darkTheme.barBackgroundEndColor highlightWithLevel:0.7];
    darkTheme.barTitleColor = [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
    darkTheme.barPathColor = [NSColor colorWithCalibratedRed:0.0 green:0.1 blue:1.0 alpha:0.2];
    
    darkTheme.listBackgroundColor = [NSColor colorWithCalibratedWhite:0.1 alpha:FILL_OPACITY];
    darkTheme.listPathColor = [NSColor colorWithCalibratedWhite:0.0 alpha:1.0];
    
    darkTheme.groupBackgroundStartColor = [NSColor colorWithCalibratedWhite:0.4 alpha:FILL_OPACITY];
    darkTheme.groupBackgroundEndColor = [NSColor colorWithCalibratedWhite:0.1 alpha:FILL_OPACITY];
    darkTheme.groupTitleColor = [NSColor colorWithCalibratedWhite:0.75 alpha:1.0];
    
    darkTheme.itemTitleColor = [NSColor colorWithCalibratedWhite:0.6 alpha:1.0];
    darkTheme.itemSubTitleColor = [NSColor colorWithCalibratedWhite:0.4 alpha:1.0];
    darkTheme.itemValueTitleColor = [NSColor colorWithCalibratedWhite:0.95 alpha:1.0];
    
    [darkTheme setUseAlternateImages:YES];
    
    [themes addObject:lightTheme];
    [themes addObject:darkTheme];
    
    return themes;
}

@end
