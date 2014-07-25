//
//  PopoverViewController.h
//  HWMonitor
//
//  Created by Kozlek on 19/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "SensorsViewController.h"

@interface PopoverController : NSViewController <SensorsViewControllerDelegate, NSPopoverDelegate>

@property (assign) IBOutlet id <NSPopoverDelegate> popoverDelegate;

@property (assign) IBOutlet NSWindowController *appController;
@property (assign) IBOutlet NSWindowController *graphsController;
@property (assign) IBOutlet NSWindowController *aboutController;

@property (readonly) BOOL isShown;

@end
