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

@property (nonatomic, strong) NSColor *toolbarStartColor;
@property (nonatomic, strong) NSColor *toolbarEndColor;
@property (nonatomic, strong) NSColor *toolbarTitleColor;
@property (nonatomic, strong) NSColor *toolbarStrokeColor;

@property (nonatomic, strong) NSColor *listBackgroundColor;

@property (nonatomic, strong) NSColor *groupStartColor;
@property (nonatomic, strong) NSColor *groupEndColor;
@property (nonatomic, strong) NSColor *groupTitleColor;

@property (nonatomic, strong) NSColor *itemTitleColor;
@property (nonatomic, strong) NSColor *itemSubTitleColor;
@property (nonatomic, strong) NSColor *itemValueTitleColor;

@property (nonatomic, assign) BOOL useDarkIcons;

+ (NSArray*)createColorThemes;

@end
