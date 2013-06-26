//
//  AboutController.h
//  HWMonitor
//
//  Created by Kozlek on 30/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AboutController : NSWindowController

@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *versionTextField;
@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *copyrightTextField;

- (IBAction)showCredits:(id)sender;
- (IBAction)openGitHubLink:(id)sender;

@end
