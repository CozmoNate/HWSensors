//
//  UpdatesController.m
//  HWMonitor
//
//  Created by kozlek on 20.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "UpdatesController.h"

#import "Localizer.h"
#import "HWMonitorDefinitions.h"

#define DOWNLOADS_URL @"https://bitbucket.org/kozlek/hwsensors/downloads"

@interface UpdatesController ()

@end

@implementation UpdatesController

- (id)init
{
    self = [super initWithWindowNibName:@"UpdatesController" owner:self];
    
    bool checkForUpdates = [[[NSUserDefaultsController sharedUserDefaultsController] defaults] boolForKey:kHWMonitorCheckUpdates];
    
    if (self && checkForUpdates) {
        [self performSelector:@selector(localizeWindow) withObject:nil afterDelay:0.0];
        [self performSelector:@selector(checkForUpdates) withObject:nil afterDelay:60.0];
    }
    
    return self;
}

-(void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [super showWindow:sender];
}

- (void)localizeWindow
{
    [Localizer localizeView:self.window];
}

- (void)checkForUpdates
{
    NSURL *url = [NSURL URLWithString:@"https://github.com/kozlek/HWSensors/raw/master/Shared/version.plist"];
    NSMutableDictionary *list = [[NSMutableDictionary alloc] initWithContentsOfURL:url];

    if (list) {
        _currentVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
        _remoteVersion = [list objectForKey:@"AppVersion"];
        _skippedVersion = [[[NSUserDefaultsController sharedUserDefaultsController] defaults] objectForKey:kHWMonitorSkippedAppVersion];
        
        if (_currentVersion && _remoteVersion && [_remoteVersion isGreaterThan:_currentVersion] && (!_skippedVersion || [_skippedVersion isLessThan:_remoteVersion])) {
            [_messageTextField setStringValue:[NSString stringWithFormat:GetLocalizedString([_messageTextField stringValue]), _remoteVersion, _currentVersion]];
            [NSApp activateIgnoringOtherApps:YES];
            [self.window setLevel:NSFloatingWindowLevel];
            [self.window makeKeyAndOrderFront:nil];
            
            return; // stop checking for updates in this session
        }
    }

    // continue check for updates every hour???
    [self performSelector:@selector(checkForUpdates) withObject:nil afterDelay:60.0 * 60];
}

- (IBAction)openDownloadsPage:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:DOWNLOADS_URL]];
    [self.window close];
}

-(void)cancelUpdate:(id)sender
{
    [self.window close];
}

- (IBAction)skipVersion:(id)sender
{
    [self.window close];
    
    [[[NSUserDefaultsController sharedUserDefaultsController] defaults] setObject:_remoteVersion forKey:kHWMonitorSkippedAppVersion];
}

@end
