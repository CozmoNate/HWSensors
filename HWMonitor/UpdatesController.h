//
//  UpdatesController.h
//  HWMonitor
//
//  Created by kozlek on 20.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

@interface UpdatesController : NSWindowController
{
    NSString *_currentVersion;
    NSString *_remoteVersion;
    NSString *_skippedVersion;
}

@property (nonatomic, unsafe_unretained) IBOutlet NSTextField *messageTextField;

- (IBAction)openDownloadsPage:(id)sender;
- (IBAction)skipVersion:(id)sender;
- (IBAction)cancelUpdate:(id)sender;

@end
