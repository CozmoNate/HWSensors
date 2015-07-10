//
//  PopoverViewController.h
//  HWMonitor
//
//  Created by Kozlek on 19/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import "SensorsViewController.h"
#import "StatusItemView.h"

@interface PopoverController : NSViewController <SensorsViewControllerDelegate, NSPopoverDelegate>

@property (assign) IBOutlet id <NSPopoverDelegate> popoverDelegate;

@property (assign) IBOutlet NSWindowController *appController;
@property (assign) IBOutlet NSWindowController *graphsController;
@property (assign) IBOutlet NSWindowController *aboutController;

@property (readonly) NSStatusItem *statusItem;
@property (readonly) StatusItemView *statusItemView;

@property (readonly) BOOL isShown;
@property (readonly) BOOL isDetached;

-(IBAction)open:(id)sender;
-(IBAction)close:(id)sender;
-(IBAction)toggle:(id)sender;
-(IBAction)detach:(id)sender;

@end
