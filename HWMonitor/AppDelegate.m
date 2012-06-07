//
//  AppDelegate.m
//  HWMonitor
//
//  Created by kozlek on 20.10.11.
//  Copyright (c) 2011 Natan Zalkin <natan.zalkin@me.com>. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)awakeFromNib
{
    _statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    _monitor = [[HWMonitor alloc] initWithStatusItem:_statusItem bundle:[NSBundle mainBundle]];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
}

@end
