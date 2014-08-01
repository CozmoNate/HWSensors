//
//  PopoverWindowController.h
//  HWMonitor
//
//  Created by Kozlek on 23/07/14.
//  Copyright (c) 2014 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "SensorsViewController.h"

@class PopoverController;

@interface PopoverWindowController : NSWindowController <SensorsViewControllerDelegate>

@property (nonatomic, weak) IBOutlet PopoverController *popoverController;
@property (assign) IBOutlet NSWindowController *appController;
@property (assign) IBOutlet NSWindowController *graphsController;
@property (assign) IBOutlet NSWindowController *aboutController;

@end
