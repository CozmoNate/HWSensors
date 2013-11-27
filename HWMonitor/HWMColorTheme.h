//
//  HWMColorTheme.h
//  HWMonitor
//
//  Created by Kozlek on 21/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class HWMConfiguration;

@interface HWMColorTheme : NSManagedObject

@property (nonatomic, retain) NSString * name;
@property (nonatomic, retain) NSColor* groupEndColor;
@property (nonatomic, retain) NSColor* groupStartColor;
@property (nonatomic, retain) NSColor* groupTitleColor;
@property (nonatomic, retain) NSColor* itemSubTitleColor;
@property (nonatomic, retain) NSColor* itemTitleColor;
@property (nonatomic, retain) NSColor* itemValueTitleColor;
@property (nonatomic, retain) NSColor* listBackgroundColor;
@property (nonatomic, retain) NSColor* listStrokeColor;
@property (nonatomic, retain) NSColor* toolbarEndColor;
@property (nonatomic, retain) NSColor* toolbarShadowColor;
@property (nonatomic, retain) NSColor* toolbarStartColor;
@property (nonatomic, retain) NSColor* toolbarStrokeColor;
@property (nonatomic, retain) NSColor* toolbarTitleColor;
@property (nonatomic, retain) NSNumber * useDarkIcons;
@property (nonatomic, retain) HWMConfiguration * configuration;

@end
