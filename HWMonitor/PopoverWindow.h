//
//  PopoverWindow.h
//  HWMonitor
//
//  Created by Kozlek on 25/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

@class HWMEngine;

@class PopoverWindow;

@protocol PopoverWindowDelegate <NSObject>

-(void)popoverWindowDidDoubleClick:(PopoverWindow*)window;

@end

@interface PopoverWindow : NSPanel

@property (nonatomic, weak) IBOutlet id<PopoverWindowDelegate> popoverWindowDelegate;

@property (nonatomic, strong) IBOutlet NSView * toolbarView;
@property (readonly) CGFloat toolbarHeight;

@property (readonly) HWMEngine *monitorEngine;

-(void)layoutContent;

@end
