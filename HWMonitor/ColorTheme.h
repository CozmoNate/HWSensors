//
//  ColorTheme.h
//  HWMonitor
//
//  Created by kozlek on 01.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ColorTheme : NSObject

@property (nonatomic, strong) NSString *name;

@property (nonatomic, strong) NSColor *barBackgroundStartColor;
@property (nonatomic, strong) NSColor *barBackgroundEndColor;
@property (nonatomic, strong) NSColor *barTitleColor;
@property (nonatomic, strong) NSColor *barPathColor;

@property (nonatomic, strong) NSColor *listBackgroundColor;
@property (nonatomic, strong) NSColor *listPathColor;

@property (nonatomic, strong) NSColor *groupBackgroundStartColor;
@property (nonatomic, strong) NSColor *groupBackgroundEndColor;
@property (nonatomic, strong) NSColor *groupTitleColor;

@property (nonatomic, strong) NSColor *itemTitleColor;
@property (nonatomic, strong) NSColor *itemSubTitleColor;
@property (nonatomic, strong) NSColor *itemValueTitleColor;

@property (nonatomic, assign) BOOL useAlternateImages;

+ (NSArray*)createColorThemes;

@end
