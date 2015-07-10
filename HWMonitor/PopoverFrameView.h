//
//  ToolbarView.h
//  HWMonitor
//
//  Created by Kozlek on 23/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface PopoverFrameView : NSView

@property (nonatomic, assign) IBInspectable CGFloat toolbarHeight;
@property (nonatomic, strong) IBInspectable NSString * toolbarTitle;

@end
