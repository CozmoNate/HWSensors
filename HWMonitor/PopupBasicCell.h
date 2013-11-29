//
//  PopupBasicCell.h
//  HWMonitor
//
//  Created by Kozlek on 22/11/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "HWMColorTheme.h"

@interface PopupBasicCell : NSTableCellView

- (void)colorThemeHasChanged:(HWMColorTheme*)newColorTheme;
- (void)didInitialized;
- (void)willDeallocated;

@end
