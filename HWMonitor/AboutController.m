//
//  AboutController.m
//  HWMonitor
//
//  Created by Kozlek on 30/04/13.
//  Copyright (c) 2013 kozlek. All rights reserved.
//

#import "AboutController.h"

@interface AboutController ()

@end

@implementation AboutController

-(id)init
{
    self = [super initWithWindowNibName:@"AboutController" owner:self];
    
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    [_appVersionTextField setStringValue:[NSString stringWithFormat:@"Version %@ (%@)", [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"], [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"]]];
    
    NSDictionary *projectVersion = [NSDictionary dictionaryWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"version" ofType:@"plist"]];

    [_projectVersionTextField setStringValue:projectVersion ? [NSString stringWithFormat:@"Project Version %@", [projectVersion objectForKey:@"ProjectVersion"]] : @""];
    
    //[_creditsTextField readRTFDFromFile:[[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"]];
    [_copyrightTextField setStringValue:[[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSHumanReadableCopyright"]];
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

- (void)openGitHubLink:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/kozlek/HWSensors"]];
}

@end
