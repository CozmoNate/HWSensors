//
//  AboutController.m
//  HWMonitor
//
//  Created by Kozlek on 30/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "AboutController.h"

#import "FadingButton.h"

@interface AboutController ()

@end

@implementation AboutController

-(id)init
{
    self = [super initWithWindowNibName:@"AboutController"];
    
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    [self.versionField setStringValue:[NSString stringWithFormat:@"Version %@", [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"]]];
    
    //[_creditsTextField readRTFDFromFile:[[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"]];
    [self.copyrightField setStringValue:[[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSHumanReadableCopyright"]];
}

-(void)showWindow:(id)sender
{
    [NSApp activateIgnoringOtherApps:YES];
    [super showWindow:sender];
}

- (void)showCredits:(id)sender
{
    [[NSWorkspace sharedWorkspace] openFile:[[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"]];
}

- (void)openLink:(id)sender
{
    if ([sender isKindOfClass:[FadingButton class]]) {
        FadingButton *button = sender;
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[button actionURL]]];
    }
}

- (void)openGitHubLink:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/kozlek/HWSensors"]];
}

- (void)openBitbucketLink:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://bitbucket.org/kozlek/hwsensors"]];
}

@end
