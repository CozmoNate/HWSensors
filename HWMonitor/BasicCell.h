//
//  BasicCell.h
//  HWMonitor
//
//  Created by Kozlek on 20/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class ColorTheme;

@interface BasicCell : NSTableCellView

@property (nonatomic, strong) NSManagedObject *managedObject;
@property (nonatomic, strong) NSString *colorThemeName;
@property (nonatomic, strong) ColorTheme *colorTheme;

@end
