//
//  UpdatesController.h
//  HWMonitor
//
//  Created by kozlek on 20.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

@interface UpdatesController : NSWindowController <NSURLConnectionDelegate, NSURLDownloadDelegate>
{
    NSString *_currentVersion;
    NSString *_remoteVersion;
    NSString *_skippedVersion;
    NSString *_installerPath;
    BOOL _forced;
    NSURLConnection *_connection;
}

@property (assign) IBOutlet NSTextField *messageTextField;

@property (assign) IBOutlet NSWindow *noUpdatesWindow;

@property (assign) IBOutlet NSWindow *progressionWindow;
@property (assign) IBOutlet NSTextField *progressionMessageTextField;
@property (assign) IBOutlet NSTextField *progressionValueTextField;
@property (assign) IBOutlet NSLevelIndicator *levelIndicator;
@property (assign) IBOutlet NSProgressIndicator *progressionIndicator;

- (void)checkForUpdates;
- (void)checkForUpdatesForced;

- (IBAction)performUpdate:(id)sender;
- (IBAction)skipVersion:(id)sender;
- (IBAction)cancelUpdate:(id)sender;

@end
