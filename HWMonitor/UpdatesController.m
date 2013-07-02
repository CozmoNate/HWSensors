//
//  UpdatesController.m
//  HWMonitor
//
//  Created by kozlek on 20.03.13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

/*
 *  Copyright (c) 2013 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 */

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
    
    bool checkForUpdates = ![[[NSUserDefaultsController sharedUserDefaultsController] defaults] boolForKey:kHWMonitorDontCheckUpdates];
    
    if (self) {
        
        _currentVersion = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"HWSensorsVersion"];
        
        if (_currentVersion) {

            [self performSelector:@selector(localizeWindow) withObject:nil afterDelay:0.0];
            
            if (checkForUpdates)
                [self performSelector:@selector(checkForUpdates) withObject:nil afterDelay:60.0 inModes:[NSArray arrayWithObject:NSRunLoopCommonModes]];
        }
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
    [Localizer localizeView:_noUpdatesWindow];
}

- (void)checkForUpdates
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    if (!_connection && !_download) {
        NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:kHWMonitorLatestInstallerUrl] cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:30.0];
        
        _connection = [[NSURLConnection alloc] initWithRequest:request delegate:self startImmediately:NO];
        
        [_connection scheduleInRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
        [_connection start];
    }
}

- (void)checkForUpdatesForced
{
    _forced = YES;
    
    [self checkForUpdates];
}

- (IBAction)performUpdate:(id)sender
{
//    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:DOWNLOADS_URL]];
//    [self.window close];
    NSSavePanel *panel = [NSSavePanel savePanel];
    
    [panel setNameFieldStringValue:[_installerPath lastPathComponent]];
    [panel setTitle:GetLocalizedString(@"Set Clover installer location")];
    
    [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
        
        if (result == NSFileHandlingPanelOKButton) {
            
            _installerPath = panel.URL.path;
            
            NSURLRequest *request = [NSURLRequest requestWithURL: [NSURL URLWithString:kHWMonitorLatestInstallerUrl] cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:20.0];
            
            _download = [[NSURLDownload alloc] initWithRequest:request delegate:self];
            
            if (_download) {
                [self.window orderOut:self];
                
                [_progressionWindow setLevel:NSModalPanelWindowLevel];
                [_progressionWindow makeKeyAndOrderFront:self];
                
                [_progressionMessageTextField setStringValue:[NSString stringWithFormat:GetLocalizedString(@"Downloading %@"), [_installerPath lastPathComponent]]];
                [_progressionValueTextField setStringValue:@""];
            }
        }
    }];
}

-(void)cancelUpdate:(id)sender
{
    if (_download) {
        [_download cancel];
        _download = nil;
        [_progressionWindow orderOut:self];
    }
}

- (IBAction)skipVersion:(id)sender
{
    [self.window close];
    
    [[[NSUserDefaultsController sharedUserDefaultsController] defaults] setObject:_remoteVersion forKey:kHWMonitorSkippedAppVersion];
}

-(void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    _forced = NO;
    _connection = nil;
}

-(void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    // Stop downloading installer
    [connection cancel];
    
    _installerPath = response.suggestedFilename;
    
    NSArray *_rawRevision = [_installerPath componentsSeparatedByString:@"."];
    
    _remoteVersion = [NSString stringWithFormat:@"%@.%@.%@", [_rawRevision objectAtIndex:_rawRevision.count - 4], [_rawRevision objectAtIndex:_rawRevision.count - 3], [_rawRevision objectAtIndex:_rawRevision.count - 2]];
    
    _skippedVersion = [[[NSUserDefaultsController sharedUserDefaultsController] defaults] objectForKey:kHWMonitorSkippedAppVersion];
    
    if (_currentVersion && _remoteVersion && [_remoteVersion isGreaterThan:_currentVersion] && (!_skippedVersion || [_skippedVersion isLessThan:_remoteVersion])) {        
        [_messageTextField setStringValue:[NSString stringWithFormat:GetLocalizedString(@"New HWSensors version %@ is available to download. You have %@. Download newer version?"), _remoteVersion, _currentVersion]];
        [NSApp activateIgnoringOtherApps:YES];
        [self.window setLevel:NSModalPanelWindowLevel];
        [self.window makeKeyAndOrderFront:nil];
    }
    else if (_forced) {
        [NSApp activateIgnoringOtherApps:YES];
        [_noUpdatesWindow setLevel:NSModalPanelWindowLevel];
        [_noUpdatesWindow makeKeyAndOrderFront:self];
    }
    else {
        // continue check for updates every hour
        if (![[NSUserDefaults standardUserDefaults] boolForKey:kHWMonitorDontCheckUpdates]) {
            [self performSelector:@selector(checkForUpdates) withObject:nil afterDelay:60.0 * 60 inModes:[NSArray arrayWithObject:NSRunLoopCommonModes]];
        }
    }
    
    _forced = NO;
    _connection = nil;
}

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
    NSLog(@"Downloading to: %@", _installerPath);
    [download setDestination:_installerPath allowOverwrite:YES];
}

- (void)download:(NSURLDownload *)download didReceiveResponse:(NSURLResponse *)response;
{
    if ([response expectedContentLength]) {
        [_levelIndicator setHidden:NO];
        [_progressionIndicator setHidden:YES];
        [_levelIndicator setMinValue:0];
        [_levelIndicator setMaxValue:[response expectedContentLength]];
        [_levelIndicator setDoubleValue:0];
    }
    else {
        [_levelIndicator setHidden:YES];
        [_progressionIndicator setHidden:NO];
    }
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(NSUInteger)length
{
    if (![_levelIndicator isHidden]) {
        [_levelIndicator setDoubleValue:_levelIndicator.doubleValue + length];
        [_progressionValueTextField setStringValue:[NSString stringWithFormat:GetLocalizedString(@"%1.1f Mbytes"), _levelIndicator.doubleValue / (1024 * 1024)]];
    }
}

- (void)downloadDidFinish:(NSURLDownload *)download
{
    _download = nil;
    [[NSWorkspace sharedWorkspace] openFile:_installerPath];
    [NSApp terminate:self];
}

- (void)download:(NSURLDownload *)aDownload didFailWithError:(NSError *)error
{
    _download = nil;
    
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert setIcon:[NSImage imageNamed:NSImageNameCaution]];
    [alert setMessageText:GetLocalizedString(@"An error occured while trying to download Clover installer!")];
    [alert setInformativeText:error.localizedDescription];
    [alert addButtonWithTitle:GetLocalizedString(@"Ok")];
    
    [alert beginSheetModalForWindow:_progressionWindow modalDelegate:nil didEndSelector:nil contextInfo:NULL];
    
    //    [self changeProgressionTitle:@"Download..." isInProgress:NO];
}


@end
