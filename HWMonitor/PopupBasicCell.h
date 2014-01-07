//
//  PopupBasicCell.h
//  HWMonitor
//
//  Created by Kozlek on 22/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class HWMColorTheme;

@interface PopupBasicCell : NSTableCellView

@property (readonly) HWMColorTheme *colorTheme;

- (void)colorThemeHasChanged:(HWMColorTheme*)newColorTheme;
- (void)initialize;
- (void)deallocate;

@end
