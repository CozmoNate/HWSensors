//
//  AboutController.h
//  HWMonitor
//
//  Created by Kozlek on 30/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AboutController : NSWindowController

@property (assign) IBOutlet NSTextField *versionField;
@property (assign) IBOutlet NSTextField *copyrightField;

- (IBAction)showCredits:(id)sender;
- (IBAction)openLink:(id)sender;

@end
